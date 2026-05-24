/*
    SPDX-FileCopyrightText: 2026 Nitrux Latinoamericana S.C.

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "secretsbackend.h"

#include <QHash>
#include <QObject>
#include <QStringList>
#include <QStringBuilder>
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
                                                       QStringLiteral("wep-key-flags"),
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

KeychainBackend::KeychainBackend(bool secretServiceAvailable)
    : m_secretServiceAvailable(secretServiceAvailable)
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

    auto cached = m_secretCache.constFind(secretKey);
    if (cached != m_secretCache.constEnd()) {
        if (callback) {
            callback(*cached, QString());
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
    auto *job = new QKeychain::ReadPasswordJob(s_keychainService);
    job->setKey(secretKey);
    QObject::connect(job, &QKeychain::Job::finished, job, [this, job, secretKey, callback](QKeychain::Job *baseJob) {
        if (baseJob->error() == QKeychain::NoError) {
            if (auto *readJob = qobject_cast<QKeychain::ReadPasswordJob *>(baseJob)) {
                const QString value = readJob->textData();
                m_secretCache.insert(secretKey, value);
                if (callback) {
                    callback(value, QString());
                }
            } else if (callback) {
                callback(QString(), QStringLiteral("Internal keychain read error."));
            }
        } else if (baseJob->error() == QKeychain::EntryNotFound) {
            m_secretCache.remove(secretKey);
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

    m_secretCache.insert(secretKey, value);

    if (!available()) {
        if (callback) {
            callback(false, QStringLiteral("Keychain backend is unavailable."));
        }
        return;
    }

#if MAUIKIT_SYSTEM_NETWORK_HAS_QTKEYCHAIN
    auto *job = new QKeychain::WritePasswordJob(s_keychainService);
    job->setKey(secretKey);
    job->setTextData(value);
    QObject::connect(job, &QKeychain::Job::finished, job, [callback, job](QKeychain::Job *baseJob) {
        if (callback) {
            callback(baseJob->error() == QKeychain::NoError, baseJob->error() == QKeychain::NoError ? QString() : baseJob->errorString());
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

    m_secretCache.remove(secretKey);

    if (!available()) {
        if (callback) {
            callback(false, QStringLiteral("Keychain backend is unavailable."));
        }
        return;
    }

#if MAUIKIT_SYSTEM_NETWORK_HAS_QTKEYCHAIN
    auto *job = new QKeychain::DeletePasswordJob(s_keychainService);
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
