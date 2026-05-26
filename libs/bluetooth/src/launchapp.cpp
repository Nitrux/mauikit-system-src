/*
    SPDX-FileCopyrightText: 2014-2015 David Rosca <nowrep@gmail.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "launchapp.h"

#include <KLocalizedString>

#include <QDBusConnection>
#include <QDBusError>
#include <QDBusInterface>
#include <QDBusMessage>
#include <QDBusObjectPath>
#include <QDBusReply>
#include <QDBusVariant>
#include <QFileInfo>

namespace
{
constexpr auto BLUEZ_SERVICE = "org.bluez";
constexpr auto BLUEZ_OBEX_SERVICE = "org.bluez.obex";
constexpr auto ADAPTER_IFACE = "org.bluez.Adapter1";
constexpr auto DEVICE_IFACE = "org.bluez.Device1";
constexpr auto OBEX_CLIENT_IFACE = "org.bluez.obex.Client1";
constexpr auto OBEX_PUSH_IFACE = "org.bluez.obex.ObjectPush1";
constexpr auto DBUS_PROPERTIES_IFACE = "org.freedesktop.DBus.Properties";
}

LaunchApp::LaunchApp(QObject *parent)
    : QObject(parent)
{
}

void LaunchApp::launchWizard()
{
    startDiscovery();
}

void LaunchApp::launchSendFile(const QString &ubi)
{
    Q_EMIT fileTransferFinished(QString(), false, i18n("Use sendFile(deviceUbi, filePath) to send files without bluedevil."));
    Q_UNUSED(ubi)
}

void LaunchApp::startDiscovery(const QString &adapterUbi)
{
    QString adapterPath = adapterUbi.isEmpty() ? QStringLiteral("/org/bluez/hci0") : adapterPathFromUbi(adapterUbi);
    if (adapterPath.isEmpty()) {
        Q_EMIT discoveryStarted(false, i18n("No Bluetooth adapter was specified."));
        return;
    }

    QDBusInterface propertiesIface(QString::fromLatin1(BLUEZ_SERVICE),
                                   adapterPath,
                                   QString::fromLatin1(DBUS_PROPERTIES_IFACE),
                                   QDBusConnection::systemBus(),
                                   this);
    if (propertiesIface.isValid()) {
        propertiesIface.call(QStringLiteral("Set"),
                             QString::fromLatin1(ADAPTER_IFACE),
                             QStringLiteral("Pairable"),
                             QVariant::fromValue(QDBusVariant(QVariant(true))));
    }

    QDBusInterface adapterIface(QString::fromLatin1(BLUEZ_SERVICE),
                                adapterPath,
                                QString::fromLatin1(ADAPTER_IFACE),
                                QDBusConnection::systemBus(),
                                this);
    if (!adapterIface.isValid()) {
        Q_EMIT discoveryStarted(false, i18n("Bluetooth adapter interface is not available."));
        return;
    }

    const QDBusReply<void> reply = adapterIface.call(QStringLiteral("StartDiscovery"));
    Q_EMIT discoveryStarted(reply.isValid(), reply.isValid() ? QString() : reply.error().message());
}

void LaunchApp::pairDevice(const QString &ubi)
{
    if (ubi.isEmpty()) {
        Q_EMIT pairingFinished(ubi, false, i18n("Missing Bluetooth device path."));
        return;
    }

    QDBusInterface deviceIface(QString::fromLatin1(BLUEZ_SERVICE),
                               ubi,
                               QString::fromLatin1(DEVICE_IFACE),
                               QDBusConnection::systemBus(),
                               this);
    if (!deviceIface.isValid()) {
        Q_EMIT pairingFinished(ubi, false, i18n("Bluetooth device interface is not available."));
        return;
    }

    const QDBusReply<void> reply = deviceIface.call(QStringLiteral("Pair"));
    Q_EMIT pairingFinished(ubi, reply.isValid(), reply.isValid() ? QString() : reply.error().message());
}

void LaunchApp::sendFile(const QString &ubi, const QString &filePath)
{
    const QString targetAddress = deviceAddressFromUbi(ubi);
    if (targetAddress.isEmpty()) {
        Q_EMIT fileTransferFinished(QString(), false, i18n("Invalid Bluetooth device path."));
        return;
    }

    const QFileInfo fileInfo(filePath);
    if (!fileInfo.exists() || !fileInfo.isFile()) {
        Q_EMIT fileTransferFinished(QString(), false, i18n("File does not exist: %1", filePath));
        return;
    }

    QDBusInterface clientIface(QString::fromLatin1(BLUEZ_OBEX_SERVICE),
                               QStringLiteral("/org/bluez/obex"),
                               QString::fromLatin1(OBEX_CLIENT_IFACE),
                               QDBusConnection::sessionBus(),
                               this);
    if (!clientIface.isValid()) {
        Q_EMIT fileTransferFinished(QString(), false, i18n("BlueZ OBEX service is not available. Install/enable obexd."));
        return;
    }

    QVariantMap args;
    args.insert(QStringLiteral("Target"), QStringLiteral("opp"));
    const QDBusReply<QDBusObjectPath> sessionReply = clientIface.call(QStringLiteral("CreateSession"), targetAddress, args);
    if (!sessionReply.isValid() || sessionReply.value().path().isEmpty()) {
        Q_EMIT fileTransferFinished(QString(), false, sessionReply.error().message());
        return;
    }

    const QString sessionPath = sessionReply.value().path();
    QDBusInterface pushIface(QString::fromLatin1(BLUEZ_OBEX_SERVICE),
                             sessionPath,
                             QString::fromLatin1(OBEX_PUSH_IFACE),
                             QDBusConnection::sessionBus(),
                             this);
    if (!pushIface.isValid()) {
        clientIface.call(QStringLiteral("RemoveSession"), QDBusObjectPath(sessionPath));
        Q_EMIT fileTransferFinished(QString(), false, i18n("OBEX ObjectPush interface is not available."));
        return;
    }

    const QDBusMessage transferReply = pushIface.call(QStringLiteral("SendFile"), filePath);
    if (transferReply.type() == QDBusMessage::ErrorMessage || transferReply.arguments().isEmpty()) {
        clientIface.call(QStringLiteral("RemoveSession"), QDBusObjectPath(sessionPath));
        Q_EMIT fileTransferFinished(QString(), false, transferReply.errorMessage());
        return;
    }

    const QDBusObjectPath transferPath = qvariant_cast<QDBusObjectPath>(transferReply.arguments().constFirst());
    if (transferPath.path().isEmpty()) {
        clientIface.call(QStringLiteral("RemoveSession"), QDBusObjectPath(sessionPath));
        Q_EMIT fileTransferFinished(QString(), false, i18n("Failed to start OBEX transfer."));
        return;
    }

    m_transferToSession.insert(transferPath.path(), sessionPath);
    m_activeTransferPath = transferPath.path();
    QDBusConnection::sessionBus().connect(QString::fromLatin1(BLUEZ_OBEX_SERVICE),
                                          transferPath.path(),
                                          QString::fromLatin1(DBUS_PROPERTIES_IFACE),
                                          QStringLiteral("PropertiesChanged"),
                                          this,
                                          SLOT(handleTransferPropertiesChanged(QString, QVariantMap, QStringList)));

    Q_EMIT fileTransferStarted(transferPath.path());
}

void LaunchApp::handleTransferPropertiesChanged(const QString &interfaceName, const QVariantMap &changedProperties, const QStringList &invalidatedProperties)
{
    Q_UNUSED(invalidatedProperties)

    if (interfaceName != QStringLiteral("org.bluez.obex.Transfer1")) {
        return;
    }

    const QString transferPath = m_activeTransferPath;
    const QString status = changedProperties.value(QStringLiteral("Status")).toString();
    const qulonglong transferred = changedProperties.value(QStringLiteral("Transferred")).toULongLong();
    const qulonglong size = changedProperties.value(QStringLiteral("Size")).toULongLong();

    if (!transferPath.isEmpty() && (transferred > 0 || size > 0)) {
        Q_EMIT fileTransferProgress(transferPath, transferred, size);
    }

    if (status == QLatin1String("complete") || status == QLatin1String("error")) {
        const bool success = status == QLatin1String("complete");
        const QString errorText = success ? QString() : i18n("File transfer failed.");
        Q_EMIT fileTransferFinished(transferPath, success, errorText);

        const QString sessionPath = m_transferToSession.take(transferPath);
        m_activeTransferPath.clear();
        if (!sessionPath.isEmpty()) {
            QDBusInterface clientIface(QString::fromLatin1(BLUEZ_OBEX_SERVICE),
                                       QStringLiteral("/org/bluez/obex"),
                                       QString::fromLatin1(OBEX_CLIENT_IFACE),
                                       QDBusConnection::sessionBus(),
                                       this);
            if (clientIface.isValid()) {
                clientIface.call(QStringLiteral("RemoveSession"), QDBusObjectPath(sessionPath));
            }
        }
    }
}

QString LaunchApp::adapterPathFromUbi(const QString &ubi)
{
    if (ubi.isEmpty()) {
        return {};
    }

    if (!ubi.contains(QStringLiteral("/dev_"))) {
        return ubi;
    }

    const int idx = ubi.indexOf(QStringLiteral("/dev_"));
    if (idx <= 0) {
        return {};
    }
    return ubi.left(idx);
}

QString LaunchApp::deviceAddressFromUbi(const QString &ubi)
{
    const int idx = ubi.indexOf(QStringLiteral("/dev_"));
    if (idx < 0) {
        return {};
    }

    QString address = ubi.mid(idx + 5);
    address.replace(QLatin1Char('_'), QLatin1Char(':'));
    return address;
}

#include "moc_launchapp.cpp"
