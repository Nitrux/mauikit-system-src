/*
    SPDX-FileCopyrightText: 2026 Nitrux Latinoamericana S.C.

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "obexagent.h"

#include <KLocalizedString>

#include <QDBusConnection>
#include <QDBusConnectionInterface>
#include <QDBusError>
#include <QDBusInterface>
#include <QDBusReply>
#include <QDBusServiceWatcher>
#include <QDBusVariant>
#include <QDateTime>
#include <QDir>
#include <QFileInfo>
#include <QLoggingCategory>
#include <QStandardPaths>

Q_LOGGING_CATEGORY(MAUI_OBEX_AGENT, "org.mauikit.system.bluetooth.obexagent")

namespace
{
constexpr auto AGENT_PATH = "/org/mauikit/system/bluetooth/obexagent";
constexpr auto AGENT_MANAGER_IFACE = "org.bluez.obex.AgentManager1";
constexpr auto DBUS_PROPERTIES_IFACE = "org.freedesktop.DBus.Properties";
constexpr auto OBEX_TRANSFER_IFACE = "org.bluez.obex.Transfer1";
constexpr auto OBEXD_FALLBACK_BIN = "obexd";
constexpr auto OBEXD_PRIMARY_BIN = "/usr/libexec/bluetooth/obexd";
}

ObexAgent::ObexAgent(QObject *parent)
    : QObject(parent)
{
}

ObexAgent::~ObexAgent()
{
    unregisterAgent();

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
    auto sessionBus = QDBusConnection::sessionBus();
    if (!sessionBus.isConnected()) {
        qCWarning(MAUI_OBEX_AGENT) << "Session D-Bus is not available";
        return false;
    }

    if (!sessionBus.registerObject(QString::fromLatin1(AGENT_PATH), this, QDBusConnection::ExportAllSlots)) {
        qCWarning(MAUI_OBEX_AGENT) << "Failed to register OBEX agent object:" << sessionBus.lastError().message();
        return false;
    }

    m_serviceWatcher = new QDBusServiceWatcher(obexServiceName(),
                                               sessionBus,
                                               QDBusServiceWatcher::WatchForRegistration | QDBusServiceWatcher::WatchForUnregistration,
                                               this);
    connect(m_serviceWatcher, &QDBusServiceWatcher::serviceRegistered, this, &ObexAgent::handleObexServiceRegistered);
    connect(m_serviceWatcher, &QDBusServiceWatcher::serviceUnregistered, this, &ObexAgent::handleObexServiceUnregistered);

    connect(&m_obexdProcess, &QProcess::errorOccurred, this, &ObexAgent::handleObexdError);
    connect(&m_obexdProcess, qOverload<int, QProcess::ExitStatus>(&QProcess::finished), this, &ObexAgent::handleObexdFinished);

    if (!isObexServiceRegistered() && !ensureObexdRunning()) {
        return false;
    }

    if (isObexServiceRegistered()) {
        return registerAgent();
    }

    qCInfo(MAUI_OBEX_AGENT) << "Waiting for org.bluez.obex to appear on the session bus";
    return true;
}

QString ObexAgent::AuthorizePush(const QDBusObjectPath &transfer)
{
    const QString name = transferName(transfer);
    const QString destination = uniqueDestinationFor(name);
    qCInfo(MAUI_OBEX_AGENT) << "Authorizing incoming OBEX transfer" << transfer.path() << "to" << destination;
    return destination;
}

void ObexAgent::Cancel()
{
    qCInfo(MAUI_OBEX_AGENT) << "Incoming OBEX transfer authorization canceled";
}

void ObexAgent::Release()
{
    qCInfo(MAUI_OBEX_AGENT) << "OBEX agent released by obexd";
    m_agentRegistered = false;
}

void ObexAgent::handleObexServiceRegistered(const QString &serviceName)
{
    Q_UNUSED(serviceName)
    registerAgent();
}

void ObexAgent::handleObexServiceUnregistered(const QString &serviceName)
{
    Q_UNUSED(serviceName)
    m_agentRegistered = false;

    if (m_startedObexd) {
        qCWarning(MAUI_OBEX_AGENT) << "org.bluez.obex disappeared; attempting to restart obexd";
        ensureObexdRunning();
    }
}

void ObexAgent::handleObexdError(QProcess::ProcessError error)
{
    qCWarning(MAUI_OBEX_AGENT) << "obexd process error:" << error << m_obexdProcess.errorString();
}

void ObexAgent::handleObexdFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
    qCWarning(MAUI_OBEX_AGENT) << "obexd exited" << exitCode << exitStatus;
    m_startedObexd = false;
}

QString ObexAgent::obexServiceName()
{
    return QStringLiteral("org.bluez.obex");
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
    return iface && iface->isServiceRegistered(obexServiceName());
}

bool ObexAgent::registerAgent()
{
    if (m_agentRegistered) {
        return true;
    }

    QDBusInterface managerIface(obexServiceName(),
                                QStringLiteral("/org/bluez/obex"),
                                QString::fromLatin1(AGENT_MANAGER_IFACE),
                                QDBusConnection::sessionBus(),
                                this);
    if (!managerIface.isValid()) {
        qCWarning(MAUI_OBEX_AGENT) << "OBEX AgentManager1 interface is not available";
        return false;
    }

    const QDBusReply<void> reply = managerIface.call(QStringLiteral("RegisterAgent"),
                                                     QVariant::fromValue(QDBusObjectPath(QString::fromLatin1(AGENT_PATH))));
    if (!reply.isValid()) {
        const QString errorName = reply.error().name();
        if (errorName == QStringLiteral("org.bluez.obex.Error.AlreadyExists")) {
            m_agentRegistered = true;
            return true;
        }

        qCWarning(MAUI_OBEX_AGENT) << "Failed to register OBEX agent:" << errorName << reply.error().message();
        return false;
    }

    m_agentRegistered = true;
    qCInfo(MAUI_OBEX_AGENT) << "Registered OBEX receive agent";
    return true;
}

void ObexAgent::unregisterAgent()
{
    if (!m_agentRegistered || !isObexServiceRegistered()) {
        return;
    }

    QDBusInterface managerIface(obexServiceName(),
                                QStringLiteral("/org/bluez/obex"),
                                QString::fromLatin1(AGENT_MANAGER_IFACE),
                                QDBusConnection::sessionBus(),
                                this);
    if (!managerIface.isValid()) {
        return;
    }

    managerIface.call(QStringLiteral("UnregisterAgent"),
                      QVariant::fromValue(QDBusObjectPath(QString::fromLatin1(AGENT_PATH))));
    m_agentRegistered = false;
}

QString ObexAgent::transferName(const QDBusObjectPath &transfer) const
{
    QDBusInterface propertiesIface(obexServiceName(),
                                   transfer.path(),
                                   QString::fromLatin1(DBUS_PROPERTIES_IFACE),
                                   QDBusConnection::sessionBus(),
                                   nullptr);
    if (propertiesIface.isValid()) {
        const QDBusReply<QVariantMap> reply = propertiesIface.call(QStringLiteral("GetAll"),
                                                                   QString::fromLatin1(OBEX_TRANSFER_IFACE));
        if (reply.isValid()) {
            const QString transferName = QFileInfo(reply.value().value(QStringLiteral("Name")).toString()).fileName();
            if (!transferName.isEmpty()) {
                return transferName;
            }
        }
    }

    const QString fallback = QFileInfo(transfer.path()).fileName();
    return fallback.isEmpty() ? QStringLiteral("incoming-file") : fallback;
}

QString ObexAgent::uniqueDestinationFor(const QString &fileName) const
{
    const QDir downloadDir(obexRootPath());
    const QFileInfo fileInfo(fileName);
    const QString baseName = fileInfo.completeBaseName().isEmpty() ? fileName : fileInfo.completeBaseName();
    const QString suffix = fileInfo.suffix();

    QString candidate = downloadDir.filePath(fileName);
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

    return downloadDir.filePath(QStringLiteral("%1-%2").arg(fileName, QString::number(QDateTime::currentMSecsSinceEpoch())));
}

#include "moc_obexagent.cpp"
