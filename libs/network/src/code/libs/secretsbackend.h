/*
    SPDX-FileCopyrightText: 2026 Nitrux Latinoamericana S.C.

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef MAUIKIT_SYSTEM_NETWORK_SECRETS_BACKEND_H
#define MAUIKIT_SYSTEM_NETWORK_SECRETS_BACKEND_H

#include <NetworkManagerQt/Setting>
#include <NetworkManagerQt/SecretAgent>
#include <NetworkManagerQt/Utils>

#include <QDBusMessage>
#include <QDBusObjectPath>
#include <QStringList>

#include <functional>
#include <QHash>
#include <QSharedPointer>
#include <QObject>
#include <QString>

class SecretsBackend
{
public:
    using ReadCallback = std::function<void(const QString &value, const QString &errorMessage)>;
    using WriteCallback = std::function<void(bool success, const QString &errorMessage)>;

    virtual ~SecretsBackend() = default;

    virtual QString id() const = 0;
    virtual bool available() const
    {
        return true;
    }

    virtual void get(const QString &uuid, const QString &setting, const QString &key, ReadCallback callback) const = 0;
    virtual void set(const QString &uuid, const QString &setting, const QString &key, const QString &value, WriteCallback callback = {}) = 0;
    virtual void remove(const QString &uuid, const QString &setting, const QString &key, WriteCallback callback = {}) = 0;

    virtual NetworkManager::Setting::SecretFlags storageFlags() const = 0;

    void applySecretFlags(NMVariantMapMap &connectionMap) const;

protected:
    static void applySecretFlagsForStorage(NMVariantMapMap &connectionMap, NetworkManager::Setting::SecretFlags flags);
};

class NmOwnedBackend final : public SecretsBackend
{
public:
    QString id() const override;
    void get(const QString &uuid, const QString &setting, const QString &key, ReadCallback callback) const override;
    void set(const QString &uuid, const QString &setting, const QString &key, const QString &value, WriteCallback callback = {}) override;
    void remove(const QString &uuid, const QString &setting, const QString &key, WriteCallback callback = {}) override;
    NetworkManager::Setting::SecretFlags storageFlags() const override;
};

class KeychainBackend final : public QObject, public SecretsBackend
{
public:
    using BatchCallback = std::function<void(bool success, const QString &errorMessage)>;

    explicit KeychainBackend(bool secretServiceAvailable, QObject *parent = nullptr);

    QString id() const override;
    bool available() const override;
    void get(const QString &uuid, const QString &setting, const QString &key, ReadCallback callback) const override;
    void set(const QString &uuid, const QString &setting, const QString &key, const QString &value, WriteCallback callback = {}) override;
    void remove(const QString &uuid, const QString &setting, const QString &key, WriteCallback callback = {}) override;
    NetworkManager::Setting::SecretFlags storageFlags() const override;

    void storeConnectionSecrets(const QString &uuid, const NMVariantMapMap &connection, BatchCallback callback = {});
    void removeConnectionSecrets(const QString &uuid, BatchCallback callback = {});

private:
    static QString keyForSecret(const QString &uuid, const QString &setting, const QString &key);
    bool m_secretServiceAvailable = false;
};

class KeychainSecretAgent final : public NetworkManager::SecretAgent
{
    Q_OBJECT

public:
    explicit KeychainSecretAgent(KeychainBackend *backend, QObject *parent = nullptr);

public Q_SLOTS:
    void CancelGetSecrets(const QDBusObjectPath &connectionPath, const QString &settingName) override;
    void DeleteSecrets(const NMVariantMapMap &connection, const QDBusObjectPath &connectionPath) override;
    NMVariantMapMap GetSecrets(const NMVariantMapMap &connection,
                                const QDBusObjectPath &connectionPath,
                                const QString &settingName,
                                const QStringList &hints,
                                uint flags) override;
    void SaveSecrets(const NMVariantMapMap &connection, const QDBusObjectPath &connectionPath) override;

private:
    struct PendingGetSecrets;

    static QString requestId(const QDBusObjectPath &connectionPath, const QString &settingName);
    static QString connectionUuid(const NMVariantMapMap &connection, const QDBusObjectPath &connectionPath);
    void finishGetSecrets(const QSharedPointer<PendingGetSecrets> &request);
    void sendReply(const QDBusMessage &message) const;

    KeychainBackend *const m_backend;
    QHash<QString, QSharedPointer<PendingGetSecrets>> m_pendingGetSecrets;
};

#endif // MAUIKIT_SYSTEM_NETWORK_SECRETS_BACKEND_H
