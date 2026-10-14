/*
    SPDX-FileCopyrightText: 2026 Nitrux Latinoamericana S.C.

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "secretsbackend.h"

#include <NetworkManagerQt/Manager>
#include <NetworkManagerQt/Settings>

#include <QHash>
#include <QList>
#include <QPair>
#include <QDBusConnection>
#include <QDBusMetaType>
#include <QDebug>
#include <QObject>
#include <QStringList>
#include <QStringBuilder>
#include <utility>
#include <QVariant>

#if MAUIKIT_SYSTEM_NETWORK_HAS_QTKEYCHAIN
#if __has_include(<qt6keychain/keychain.h>)
#include <qt6keychain/keychain.h>
#elif __has_include(<qt5keychain/keychain.h>)
#include <qt5keychain/keychain.h>
#else
#include <keychain.h>
#endif
#endif

namespace
{
using SecretFlagList = QHash<QString, QStringList>;
const QString s_keychainService = QStringLiteral("org.mauikit.system.network");

SecretFlagList knownSecretFlagKeys()
{
    return {
        {QStringLiteral("802-11-wireless-security"), QStringList{
                                                       QStringLiteral("psk-flags"),
                                                       QStringLiteral("wep-key0-flags"),
                                                       QStringLiteral("wep-key1-flags"),
                                                       QStringLiteral("wep-key2-flags"),
                                                       QStringLiteral("wep-key3-flags"),
                                                       QStringLiteral("leap-password-flags"),
                                                   }},
        {QStringLiteral("802-1x"), QStringList{
                                      QStringLiteral("password-flags"),
                                      QStringLiteral("private-key-password-flags"),
                                      QStringLiteral("phase2-private-key-password-flags"),
                                  }},
        {QStringLiteral("gsm"), QStringList{
                                    QStringLiteral("password-flags"),
                                    QStringLiteral("pin-flags"),
                                }},
        {QStringLiteral("cdma"), QStringList{
                                     QStringLiteral("password-flags"),
                                 }},
        {QStringLiteral("pppoe"), QStringList{
                                      QStringLiteral("password-flags"),
                                  }},
        {QStringLiteral("wireguard"), QStringList{
                                          QStringLiteral("private-key-flags"),
                                          QStringLiteral("preshared-key-flags"),
                                      }},
        {QStringLiteral("vpn"), QStringList{
                                   QStringLiteral("password-flags"),
                                   QStringLiteral("proxy-password-flags"),
                                   QStringLiteral("http-proxy-password-flags"),
                                   QStringLiteral("cert-pass-flags"),
                               }},
    };
}
}

void SecretsBackend::applySecretFlags(NMVariantMapMap &connectionMap) const
{
    applySecretFlagsForStorage(connectionMap, storageFlags());
}

void SecretsBackend::applySecretFlagsForStorage(NMVariantMapMap &connectionMap, NetworkManager::Setting::SecretFlags flags)
{
    const QVariant flagValue = QVariant::fromValue(static_cast<quint32>(flags));
    const auto secretFlags = knownSecretFlagKeys();

    for (auto it = secretFlags.cbegin(); it != secretFlags.cend(); ++it) {
        if (!connectionMap.contains(it.key())) {
            continue;
        }

        QVariantMap settingMap = connectionMap.value(it.key());
        for (const QString &flagKey : it.value()) {
            settingMap.insert(flagKey, flagValue);
        }
        connectionMap.insert(it.key(), settingMap);
    }
}

QString NmOwnedBackend::id() const
{
    return QStringLiteral("nm-owned");
}

void NmOwnedBackend::get(const QString &uuid, const QString &setting, const QString &key, ReadCallback callback) const
{
    Q_UNUSED(uuid)
    Q_UNUSED(setting)
    Q_UNUSED(key)
    if (callback) {
        callback(QString(), QString());
    }
}

void NmOwnedBackend::set(const QString &uuid, const QString &setting, const QString &key, const QString &value, WriteCallback callback)
{
    Q_UNUSED(uuid)
    Q_UNUSED(setting)
    Q_UNUSED(key)
    Q_UNUSED(value)
    if (callback) {
        callback(true, QString());
    }
}

void NmOwnedBackend::remove(const QString &uuid, const QString &setting, const QString &key, WriteCallback callback)
{
    Q_UNUSED(uuid)
    Q_UNUSED(setting)
    Q_UNUSED(key)
    if (callback) {
        callback(true, QString());
    }
}

NetworkManager::Setting::SecretFlags NmOwnedBackend::storageFlags() const
{
    return NetworkManager::Setting::None;
}

KeychainBackend::KeychainBackend(bool secretServiceAvailable, QObject *parent)
    : QObject(parent)
    , m_secretServiceAvailable(secretServiceAvailable)
{
}

QString KeychainBackend::id() const
{
    return QStringLiteral("keychain");
}

bool KeychainBackend::available() const
{
#if !MAUIKIT_SYSTEM_NETWORK_HAS_QTKEYCHAIN
    return false;
#endif
    return m_secretServiceAvailable;
}

QString KeychainBackend::keyForSecret(const QString &uuid, const QString &setting, const QString &key)
{
    return uuid % QLatin1Char('/') % setting % QLatin1Char('/') % key;
}

void KeychainBackend::get(const QString &uuid, const QString &setting, const QString &key, ReadCallback callback) const
{
    const QString secretKey = keyForSecret(uuid, setting, key);
    if (secretKey.isEmpty()) {
        if (callback) {
            callback(QString(), QStringLiteral("Invalid secret key."));
        }
        return;
    }

    if (!available()) {
        if (callback) {
            callback(QString(), QStringLiteral("Keychain backend is unavailable."));
        }
        return;
    }

#if MAUIKIT_SYSTEM_NETWORK_HAS_QTKEYCHAIN
    auto *job = new QKeychain::ReadPasswordJob(s_keychainService, const_cast<KeychainBackend *>(this));
    job->setKey(secretKey);
    QObject::connect(job, &QKeychain::Job::finished, job, [job, callback](QKeychain::Job *baseJob) {
        if (baseJob->error() == QKeychain::NoError) {
            if (auto *readJob = qobject_cast<QKeychain::ReadPasswordJob *>(baseJob)) {
                const QString value = readJob->textData();
                if (callback) {
                    callback(value, QString());
                }
            } else if (callback) {
                callback(QString(), QStringLiteral("Internal keychain read error."));
            }
        } else if (baseJob->error() == QKeychain::EntryNotFound) {
            if (callback) {
                callback(QString(), QString());
            }
        } else if (callback) {
            callback(QString(), baseJob->errorString());
        }

        job->deleteLater();
    });
    job->start();
#endif
}

void KeychainBackend::set(const QString &uuid, const QString &setting, const QString &key, const QString &value, WriteCallback callback)
{
    const QString secretKey = keyForSecret(uuid, setting, key);
    if (secretKey.isEmpty()) {
        if (callback) {
            callback(false, QStringLiteral("Invalid secret key."));
        }
        return;
    }

    if (!available()) {
        if (callback) {
            callback(false, QStringLiteral("Keychain backend is unavailable."));
        }
        return;
    }

#if MAUIKIT_SYSTEM_NETWORK_HAS_QTKEYCHAIN
    auto *job = new QKeychain::WritePasswordJob(s_keychainService, this);
    job->setKey(secretKey);
    job->setTextData(value);
    QObject::connect(job, &QKeychain::Job::finished, job, [callback, job](QKeychain::Job *baseJob) {
        const bool ok = baseJob->error() == QKeychain::NoError;
        if (callback) {
            callback(ok, ok ? QString() : baseJob->errorString());
        }
        job->deleteLater();
    });
    job->start();
#endif
}

void KeychainBackend::remove(const QString &uuid, const QString &setting, const QString &key, WriteCallback callback)
{
    const QString secretKey = keyForSecret(uuid, setting, key);
    if (secretKey.isEmpty()) {
        if (callback) {
            callback(false, QStringLiteral("Invalid secret key."));
        }
        return;
    }


    if (!available()) {
        if (callback) {
            callback(false, QStringLiteral("Keychain backend is unavailable."));
        }
        return;
    }

#if MAUIKIT_SYSTEM_NETWORK_HAS_QTKEYCHAIN
    auto *job = new QKeychain::DeletePasswordJob(s_keychainService, this);
    job->setKey(secretKey);
    QObject::connect(job, &QKeychain::Job::finished, job, [callback, job](QKeychain::Job *baseJob) {
        const bool ok = baseJob->error() == QKeychain::NoError || baseJob->error() == QKeychain::EntryNotFound;
        if (callback) {
            callback(ok, ok ? QString() : baseJob->errorString());
        }
        job->deleteLater();
    });
    job->start();
#endif
}

NetworkManager::Setting::SecretFlags KeychainBackend::storageFlags() const
{
    return NetworkManager::Setting::AgentOwned;
}

namespace
{
struct SecretEntry
{
    QString setting;
    QString key;
    QString value;
};

QStringList secretKeysForSetting(const QString &setting)
{
    QStringList keys;
    const QStringList flagKeys = knownSecretFlagKeys().value(setting);
    for (const QString &flagKey : flagKeys) {
        if (flagKey.endsWith(QStringLiteral("-flags"))) {
            keys.append(flagKey.left(flagKey.size() - 6));
        }
    }
    return keys;
}

QList<SecretEntry> connectionSecretEntries(const NMVariantMapMap &connection)
{
    QList<SecretEntry> entries;
    for (auto settingIt = connection.cbegin(); settingIt != connection.cend(); ++settingIt) {
        const QString settingName = settingIt.key();
        const QVariantMap setting = settingIt.value();
        const QStringList keys = secretKeysForSetting(settingName);

        if (settingName == QStringLiteral("vpn")) {
            const NMStringMap vpnSecrets = qdbus_cast<NMStringMap>(setting.value(QStringLiteral("secrets")));
            for (const QString &key : keys) {
                if (vpnSecrets.contains(key)) {
                    entries.append({settingName, key, vpnSecrets.value(key)});
                }
            }
            continue;
        }

        for (const QString &key : keys) {
            if (setting.contains(key)) {
                entries.append({settingName, key, setting.value(key).toString()});
            }
        }
    }
    return entries;
}

QList<QPair<QString, QString>> allSecretKeys()
{
    QList<QPair<QString, QString>> keys;
    const SecretFlagList flags = knownSecretFlagKeys();
    for (auto settingIt = flags.cbegin(); settingIt != flags.cend(); ++settingIt) {
        for (const QString &key : secretKeysForSetting(settingIt.key())) {
            keys.append({settingIt.key(), key});
        }
    }
    return keys;
}

struct BatchState
{
    int remaining = 0;
    bool success = true;
    QString errorMessage;
    KeychainBackend::BatchCallback callback;
};
}

void KeychainBackend::storeConnectionSecrets(const QString &uuid, const NMVariantMapMap &connection, BatchCallback callback)
{
    if (uuid.isEmpty()) {
        if (callback) {
            callback(false, QStringLiteral("The connection has no UUID."));
        }
        return;
    }
    if (!available()) {
        if (callback) {
            callback(false, QStringLiteral("Keychain backend is unavailable."));
        }
        return;
    }

    const QList<SecretEntry> entries = connectionSecretEntries(connection);
    if (entries.isEmpty()) {
        if (callback) {
            callback(true, QString());
        }
        return;
    }

    auto state = QSharedPointer<BatchState>::create();
    state->remaining = entries.size();
    state->callback = std::move(callback);
    for (const SecretEntry &entry : entries) {
        set(uuid, entry.setting, entry.key, entry.value, [state](bool success, const QString &errorMessage) {
            if (!success) {
                state->success = false;
                if (state->errorMessage.isEmpty()) {
                    state->errorMessage = errorMessage;
                }
            }
            --state->remaining;
            if (state->remaining == 0 && state->callback) {
                const auto callback = std::move(state->callback);
                callback(state->success, state->errorMessage);
            }
        });
    }
}

void KeychainBackend::removeConnectionSecrets(const QString &uuid, BatchCallback callback)
{
    if (uuid.isEmpty()) {
        if (callback) {
            callback(false, QStringLiteral("The connection has no UUID."));
        }
        return;
    }
    if (!available()) {
        if (callback) {
            callback(false, QStringLiteral("Keychain backend is unavailable."));
        }
        return;
    }

    const QList<QPair<QString, QString>> keys = allSecretKeys();
    auto state = QSharedPointer<BatchState>::create();
    state->remaining = keys.size();
    state->callback = std::move(callback);
    for (const auto &entry : keys) {
        remove(uuid, entry.first, entry.second, [state](bool success, const QString &errorMessage) {
            if (!success) {
                state->success = false;
                if (state->errorMessage.isEmpty()) {
                    state->errorMessage = errorMessage;
                }
            }
            --state->remaining;
            if (state->remaining == 0 && state->callback) {
                const auto callback = std::move(state->callback);
                callback(state->success, state->errorMessage);
            }
        });
    }
}

struct KeychainSecretAgent::PendingGetSecrets
{
    QString id;
    QString uuid;
    QString settingName;
    QDBusMessage message;
    QHash<QString, QString> values;
    int remaining = 0;
    bool canceled = false;
    QString errorMessage;
};

KeychainSecretAgent::KeychainSecretAgent(KeychainBackend *backend, QObject *parent)
    : NetworkManager::SecretAgent(QStringLiteral("org.mauikit.system.network"), parent)
    , m_backend(backend)
{
}

QString KeychainSecretAgent::requestId(const QDBusObjectPath &connectionPath, const QString &settingName)
{
    return connectionPath.path() + QLatin1Char(0x0a) + settingName;
}

QString KeychainSecretAgent::connectionUuid(const NMVariantMapMap &connection, const QDBusObjectPath &connectionPath)
{
    const QString uuid = connection.value(QStringLiteral("connection")).value(QStringLiteral("uuid")).toString();
    if (!uuid.isEmpty()) {
        return uuid;
    }

    const auto profile = NetworkManager::findConnection(connectionPath.path());
    return profile ? profile->uuid() : QString();
}

NMVariantMapMap KeychainSecretAgent::GetSecrets(const NMVariantMapMap &connection,
                                                const QDBusObjectPath &connectionPath,
                                                const QString &settingName,
                                                const QStringList &hints,
                                                uint flags)
{
    Q_UNUSED(hints)
    Q_UNUSED(flags)

    setDelayedReply(true);
    const QDBusMessage callMessage = message();
    const QString uuid = connectionUuid(connection, connectionPath);
    const QStringList keys = secretKeysForSetting(settingName);
    if (!m_backend || uuid.isEmpty() || keys.isEmpty()) {
        sendError(uuid.isEmpty() ? InvalidConnection : NoSecrets,
                  uuid.isEmpty() ? QStringLiteral("The connection has no UUID.") : QStringLiteral("No stored secrets are available."),
                  callMessage);
        return {};
    }

    const QString id = requestId(connectionPath, settingName);
    if (m_pendingGetSecrets.contains(id)) {
        CancelGetSecrets(connectionPath, settingName);
    }

    auto request = QSharedPointer<PendingGetSecrets>::create();
    request->id = id;
    request->uuid = uuid;
    request->settingName = settingName;
    request->message = callMessage;
    request->remaining = keys.size();
    m_pendingGetSecrets.insert(id, request);

    for (const QString &key : keys) {
        m_backend->get(uuid, settingName, key, [this, request, key](const QString &value, const QString &errorMessage) {
            if (request->canceled) {
                return;
            }
            if (!errorMessage.isEmpty() && request->errorMessage.isEmpty()) {
                request->errorMessage = errorMessage;
            } else if (!value.isEmpty()) {
                request->values.insert(key, value);
            }
            --request->remaining;
            if (request->remaining == 0) {
                finishGetSecrets(request);
            }
        });
    }

    return {};
}

void KeychainSecretAgent::finishGetSecrets(const QSharedPointer<PendingGetSecrets> &request)
{
    if (request->canceled || request->remaining != 0) {
        return;
    }

    m_pendingGetSecrets.remove(request->id);
    if (!request->errorMessage.isEmpty()) {
        sendError(InternalError, request->errorMessage, request->message);
        return;
    }
    if (request->values.isEmpty()) {
        sendError(NoSecrets, QStringLiteral("No stored secrets are available."), request->message);
        return;
    }

    QVariantMap setting;
    setting.insert(QStringLiteral("name"), request->settingName);
    if (request->settingName == QStringLiteral("vpn")) {
        NMStringMap secrets;
        for (auto it = request->values.cbegin(); it != request->values.cend(); ++it) {
            secrets.insert(it.key(), it.value());
        }
        setting.insert(QStringLiteral("secrets"), QVariant::fromValue(secrets));
    } else {
        for (auto it = request->values.cbegin(); it != request->values.cend(); ++it) {
            setting.insert(it.key(), it.value());
        }
    }

    NMVariantMapMap result;
    result.insert(request->settingName, setting);
    sendReply(request->message.createReply({QVariant::fromValue(result)}));
}

void KeychainSecretAgent::CancelGetSecrets(const QDBusObjectPath &connectionPath, const QString &settingName)
{
    const QString id = requestId(connectionPath, settingName);
    const auto request = m_pendingGetSecrets.take(id);
    if (!request) {
        return;
    }

    request->canceled = true;
    sendError(AgentCanceled, QStringLiteral("The secret request was canceled."), request->message);
}

void KeychainSecretAgent::SaveSecrets(const NMVariantMapMap &connection, const QDBusObjectPath &connectionPath)
{
    setDelayedReply(true);
    const QDBusMessage callMessage = message();
    const QString uuid = connectionUuid(connection, connectionPath);
    if (!m_backend || uuid.isEmpty()) {
        sendError(InvalidConnection, QStringLiteral("The connection has no UUID."), callMessage);
        return;
    }

    m_backend->storeConnectionSecrets(uuid, connection, [this, callMessage](bool success, const QString &errorMessage) {
        if (!success) {
            sendError(InternalError, errorMessage, callMessage);
            return;
        }
        sendReply(callMessage.createReply());
    });
}

void KeychainSecretAgent::DeleteSecrets(const NMVariantMapMap &connection, const QDBusObjectPath &connectionPath)
{
    setDelayedReply(true);
    const QDBusMessage callMessage = message();
    const QString uuid = connectionUuid(connection, connectionPath);
    if (!m_backend || uuid.isEmpty()) {
        sendError(InvalidConnection, QStringLiteral("The connection has no UUID."), callMessage);
        return;
    }

    m_backend->removeConnectionSecrets(uuid, [this, callMessage](bool success, const QString &errorMessage) {
        if (!success) {
            sendError(InternalError, errorMessage, callMessage);
            return;
        }
        sendReply(callMessage.createReply());
    });
}

void KeychainSecretAgent::sendReply(const QDBusMessage &reply) const
{
    if (!QDBusConnection::systemBus().send(reply)) {
        qWarning() << "Failed to send NetworkManager secret-agent reply:" << reply.errorMessage();
    }
}
