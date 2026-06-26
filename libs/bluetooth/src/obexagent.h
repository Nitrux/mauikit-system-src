/*
    SPDX-FileCopyrightText: 2026 Nitrux Latinoamericana S.C.

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#pragma once

#include <QObject>
#include <QProcess>
#include <QTimer>

namespace BluezQt
{
class InitObexManagerJob;
class ObexManager;
class PendingCall;
}

class ObexReceiveAgent;

class ObexAgent : public QObject
{
    Q_OBJECT

public:
    explicit ObexAgent(QObject *parent = nullptr);
    ~ObexAgent() override;

    bool initialize();
    QString uniqueDestinationFor(const QString &fileName) const;
    void handleAgentReleased();

private Q_SLOTS:
    void handleObexdError(QProcess::ProcessError error);
    void handleObexdFinished(int exitCode, QProcess::ExitStatus exitStatus);
    void handleManagerInitResult(BluezQt::InitObexManagerJob *job);
    void handleOperationalChanged(bool operational);
    void handleRegisterAgentFinished(BluezQt::PendingCall *call);
    void retryInitialization();

private:
    static QString obexRootPath();
    static QString obexdBinary();

    bool ensureObexdRunning();
    bool isObexServiceRegistered() const;
    void initializeManager();
    void registerAgent();

    QProcess m_obexdProcess;
    QTimer m_retryTimer;
    BluezQt::ObexManager *m_obexManager = nullptr;
    ObexReceiveAgent *m_receiveAgent = nullptr;
    bool m_startedObexd = false;
    bool m_managerInitStarted = false;
    bool m_registerPending = false;
    bool m_agentRegistered = false;
};
