/*
    SPDX-FileCopyrightText: 2014 David Rosca <nowrep@gmail.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#pragma once

#include <QStringList>
#include <QObject>
#include <QHash>
#include <QVariantMap>

#include <qqmlregistration.h>

class LaunchApp : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    QML_SINGLETON

public:
    explicit LaunchApp(QObject *parent = nullptr);

public Q_SLOTS:
    void launchWizard();
    void launchSendFile(const QString &ubi);
    void startDiscovery(const QString &adapterUbi = QString());
    void pairDevice(const QString &ubi);
    void sendFile(const QString &ubi, const QString &filePath);

Q_SIGNALS:
    void discoveryStarted(bool success, const QString &error);
    void pairingFinished(const QString &ubi, bool success, const QString &error);
    void fileTransferStarted(const QString &transferPath);
    void fileTransferProgress(const QString &transferPath, qulonglong transferred, qulonglong size);
    void fileTransferFinished(const QString &transferPath, bool success, const QString &error);

private Q_SLOTS:
    void handleTransferPropertiesChanged(const QString &interfaceName, const QVariantMap &changedProperties, const QStringList &invalidatedProperties);

private:
    static QString adapterPathFromUbi(const QString &ubi);
    static QString deviceAddressFromUbi(const QString &ubi);

    QHash<QString, QString> m_transferToSession;
    QString m_activeTransferPath;
};
