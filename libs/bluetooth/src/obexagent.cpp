/*
    SPDX-FileCopyrightText: 2026 Nitrux Latinoamericana S.C.

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "obexagent.h"

#include <BluezQt/InitObexManagerJob>
#include <BluezQt/ObexAgent>
#include <BluezQt/ObexManager>
#include <BluezQt/ObexSession>
#include <BluezQt/ObexTransfer>
#include <BluezQt/PendingCall>
#include <BluezQt/Request>

#include <QDBusConnection>
#include <QDBusConnectionInterface>
#include <QDBusObjectPath>
#include <QDateTime>
#include <QDir>
#include <QFileInfo>
#include <QLoggingCategory>
#include <QPointer>
#include <QStandardPaths>

Q_LOGGING_CATEGORY(MAUI_OBEX_AGENT, "org.mauikit.system.bluetooth.obexagent")

namespace
{
constexpr auto AGENT_PATH = "/org/mauikit/system/bluetooth/obexagent";
constexpr auto OBEX_SERVICE = "org.bluez.obex";
constexpr auto OBEXD_FALLBACK_BIN = "obexd";
constexpr auto OBEXD_PRIMARY_BIN = "/usr/libexec/bluetooth/obexd";
constexpr auto REGISTER_RETRY_INTERVAL_MS = 1500;
}

class ObexReceiveAgent final : public BluezQt::ObexAgent
{
public:
    explicit ObexReceiveAgent(::ObexAgent *controller)
        : BluezQt::ObexAgent(controller)
        , m_controller(controller)
    {
    }

    QDBusObjectPath objectPath() const override
    {
        return QDBusObjectPath(QString::fromLatin1(AGENT_PATH));
    }

    void authorizePush(BluezQt::ObexTransferPtr transfer, BluezQt::ObexSessionPtr session, const BluezQt::Request<QString> &request) override
    {
        Q_UNUSED(session)

        QString fileName;
        if (transfer) {
            fileName = QFileInfo(transfer->name()).fileName();
        }
        if (fileName.isEmpty()) {
            fileName = QStringLiteral("incoming-file");
        }

        if (!m_controller) {
            request.cancel();
            return;
        }

        const QString destination = m_controller->uniqueDestinationFor(fileName);
        qCInfo(MAUI_OBEX_AGENT) << "Authorizing incoming OBEX transfer to" << destination;
        request.accept(destination);
    }

    void cancel() override
    {
        qCInfo(MAUI_OBEX_AGENT) << "Incoming OBEX transfer authorization canceled";
    }

    void release() override
    {
        qCInfo(MAUI_OBEX_AGENT) << "OBEX agent released by obexd";
        if (m_controller) {
            m_controller->handleAgentReleased();
        }
    }

private:
    QPointer<::ObexAgent> m_controller;
};

ObexAgent::ObexAgent(QObject *parent)
    : QObject(parent)
{
    m_retryTimer.setInterval(REGISTER_RETRY_INTERVAL_MS);
    m_retryTimer.setSingleShot(true);
    connect(&m_retryTimer, &QTimer::timeout, this, &ObexAgent::retryInitialization);

    connect(&m_obexdProcess, &QProcess::errorOccurred, this, &ObexAgent::handleObexdError);
    connect(&m_obexdProcess, qOverload<int, QProcess::ExitStatus>(&QProcess::finished), this, &ObexAgent::handleObexdFinished);
}

ObexAgent::~ObexAgent()
{
    if (m_startedObexd && m_obexdProcess.state() != QProcess::NotRunning) {
        m_obexdProcess.terminate();
        if (!m_obexdProcess.waitForFinished(3000)) {
            m_obexdProcess.kill();
            m_obexdProcess.waitForFinished(1000);
        }
    }
}

bool ObexAgent::initialize()
{
    if (!QDBusConnection::sessionBus().isConnected()) {
        qCWarning(MAUI_OBEX_AGENT) << "Session D-Bus is not available";
        return false;
    }

    retryInitialization();
    return true;
}

QString ObexAgent::uniqueDestinationFor(const QString &fileName) const
{
    const QDir downloadDir(obexRootPath());
    const QFileInfo fileInfo(fileName);
    const QString resolvedName = fileInfo.fileName().isEmpty() ? QStringLiteral("incoming-file") : fileInfo.fileName();
    const QString baseName = QFileInfo(resolvedName).completeBaseName().isEmpty() ? resolvedName : QFileInfo(resolvedName).completeBaseName();
    const QString suffix = QFileInfo(resolvedName).suffix();

    QString candidate = downloadDir.filePath(resolvedName);
    if (!QFileInfo::exists(candidate)) {
        return candidate;
    }

    for (int counter = 1; counter < 10000; ++counter) {
        const QString numberedName = suffix.isEmpty()
            ? QStringLiteral("%1-%2").arg(baseName).arg(counter)
            : QStringLiteral("%1-%2.%3").arg(baseName).arg(counter).arg(suffix);
        candidate = downloadDir.filePath(numberedName);
        if (!QFileInfo::exists(candidate)) {
            return candidate;
        }
    }

    return downloadDir.filePath(QStringLiteral("%1-%2").arg(resolvedName, QString::number(QDateTime::currentMSecsSinceEpoch())));
}

void ObexAgent::handleAgentReleased()
{
    m_agentRegistered = false;
    m_registerPending = false;
    retryInitialization();
}

void ObexAgent::handleObexdError(QProcess::ProcessError error)
{
    qCWarning(MAUI_OBEX_AGENT) << "obexd process error:" << error << m_obexdProcess.errorString();
    m_startedObexd = false;
    retryInitialization();
}

void ObexAgent::handleObexdFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
    qCWarning(MAUI_OBEX_AGENT) << "obexd exited" << exitCode << exitStatus;
    m_startedObexd = false;
    retryInitialization();
}

void ObexAgent::handleManagerInitResult(BluezQt::InitObexManagerJob *job)
{
    m_managerInitStarted = false;

    if (job->error() != 0) {
        qCWarning(MAUI_OBEX_AGENT) << "Failed to initialize BluezQt OBEX manager:" << job->errorText();
        if (!m_retryTimer.isActive()) {
            m_retryTimer.start();
        }
        return;
    }

    if (m_obexManager && m_obexManager->isOperational()) {
        registerAgent();
    } else if (!m_retryTimer.isActive()) {
        m_retryTimer.start();
    }
}

void ObexAgent::handleOperationalChanged(bool operational)
{
    if (operational) {
        registerAgent();
        return;
    }

    m_agentRegistered = false;
    m_registerPending = false;
    if (!m_retryTimer.isActive()) {
        m_retryTimer.start();
    }
}

void ObexAgent::handleRegisterAgentFinished(BluezQt::PendingCall *call)
{
    m_registerPending = false;

    if (!call) {
        if (!m_retryTimer.isActive()) {
            m_retryTimer.start();
        }
        return;
    }

    if (call->error() == BluezQt::PendingCall::NoError || call->error() == BluezQt::PendingCall::AlreadyExists) {
        m_agentRegistered = true;
        qCInfo(MAUI_OBEX_AGENT) << "Registered OBEX receive agent";
        return;
    }

    qCWarning(MAUI_OBEX_AGENT) << "Failed to register OBEX agent:" << call->errorText();
    if (!m_retryTimer.isActive()) {
        m_retryTimer.start();
    }
}

void ObexAgent::retryInitialization()
{
    if (m_agentRegistered || m_registerPending) {
        return;
    }

    if (!isObexServiceRegistered() && !ensureObexdRunning()) {
        if (!m_retryTimer.isActive()) {
            m_retryTimer.start();
        }
        return;
    }

    if (!m_obexManager) {
        initializeManager();
        return;
    }

    if (m_obexManager->isOperational()) {
        registerAgent();
        return;
    }

    if (!m_obexManager->isInitialized() && !m_managerInitStarted) {
        initializeManager();
        return;
    }

    if (!m_retryTimer.isActive()) {
        m_retryTimer.start();
    }
}

QString ObexAgent::obexRootPath()
{
    QString downloadDir = QStandardPaths::writableLocation(QStandardPaths::DownloadLocation);
    const QString homeDir = QDir::homePath();

    if (downloadDir.isEmpty() || QDir::cleanPath(downloadDir) == QDir::cleanPath(homeDir)) {
        downloadDir = homeDir + QStringLiteral("/Downloads");
    }

    QDir().mkpath(downloadDir);
    return QFileInfo(downloadDir).absoluteFilePath();
}

QString ObexAgent::obexdBinary()
{
    if (QFileInfo::exists(QString::fromLatin1(OBEXD_PRIMARY_BIN))) {
        return QString::fromLatin1(OBEXD_PRIMARY_BIN);
    }

    return QStandardPaths::findExecutable(QString::fromLatin1(OBEXD_FALLBACK_BIN));
}

bool ObexAgent::ensureObexdRunning()
{
    if (m_obexdProcess.state() != QProcess::NotRunning) {
        return true;
    }

    const QString binary = obexdBinary();
    if (binary.isEmpty()) {
        qCWarning(MAUI_OBEX_AGENT) << "Could not find obexd binary";
        return false;
    }

    const QString rootPath = obexRootPath();
    const QStringList arguments = {
        QStringLiteral("-n"),
        QStringLiteral("-a"),
        QStringLiteral("--symlinks"),
        QStringLiteral("--noplugin=pbap,mas"),
        QStringLiteral("--root=%1").arg(rootPath),
    };

    m_obexdProcess.setProgram(binary);
    m_obexdProcess.setArguments(arguments);
    m_obexdProcess.setProcessChannelMode(QProcess::ForwardedChannels);
    m_obexdProcess.start();

    if (!m_obexdProcess.waitForStarted(3000)) {
        qCWarning(MAUI_OBEX_AGENT) << "Failed to start obexd:" << m_obexdProcess.errorString();
        return false;
    }

    m_startedObexd = true;
    qCInfo(MAUI_OBEX_AGENT) << "Started obexd with root path" << rootPath;
    return true;
}

bool ObexAgent::isObexServiceRegistered() const
{
    auto *iface = QDBusConnection::sessionBus().interface();
    return iface && iface->isServiceRegistered(QString::fromLatin1(OBEX_SERVICE));
}

void ObexAgent::initializeManager()
{
    if (!m_obexManager) {
        m_obexManager = new BluezQt::ObexManager(this);
        connect(m_obexManager, &BluezQt::ObexManager::operationalChanged, this, &ObexAgent::handleOperationalChanged);
    }

    if (m_managerInitStarted) {
        return;
    }

    m_managerInitStarted = true;
    BluezQt::InitObexManagerJob *job = m_obexManager->init();
    connect(job, &BluezQt::InitObexManagerJob::result, this, &ObexAgent::handleManagerInitResult);
    job->start();
}

void ObexAgent::registerAgent()
{
    if (!m_obexManager || !m_obexManager->isOperational() || m_agentRegistered || m_registerPending) {
        return;
    }

    if (!m_receiveAgent) {
        m_receiveAgent = new ObexReceiveAgent(this);
    }

    m_registerPending = true;
    BluezQt::PendingCall *call = m_obexManager->registerAgent(m_receiveAgent);
    connect(call, &BluezQt::PendingCall::finished, this, &ObexAgent::handleRegisterAgentFinished);
}

#include "moc_obexagent.cpp"
