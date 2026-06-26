/*
    SPDX-FileCopyrightText: 2026 Nitrux Latinoamericana S.C.

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#pragma once

#include <QDBusObjectPath>
#include <QObject>
#include <QProcess>
#include <QVariantMap>

class QDBusServiceWatcher;

class ObexAgent : public QObject
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.bluez.obex.Agent1")

public:
    explicit ObexAgent(QObject *parent = nullptr);
    ~ObexAgent() override;

    bool initialize();

public Q_SLOTS:
    QString AuthorizePush(const QDBusObjectPath &transfer);
    void Cancel();
    void Release();

private Q_SLOTS:
    void handleObexServiceRegistered(const QString &serviceName);
    void handleObexServiceUnregistered(const QString &serviceName);
    void handleObexdError(QProcess::ProcessError error);
    void handleObexdFinished(int exitCode, QProcess::ExitStatus exitStatus);

private:
    static QString obexServiceName();
    static QString obexRootPath();
    static QString obexdBinary();

    bool ensureObexdRunning();
    bool isObexServiceRegistered() const;
    bool registerAgent();
    void unregisterAgent();
    QString transferName(const QDBusObjectPath &transfer) const;
    QString uniqueDestinationFor(const QString &fileName) const;

    QProcess m_obexdProcess;
    QDBusServiceWatcher *m_serviceWatcher = nullptr;
    bool m_startedObexd = false;
    bool m_agentRegistered = false;
};
