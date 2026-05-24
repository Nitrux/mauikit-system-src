/*
    SPDX-FileCopyrightText: 2026 Nitrux Latinoamericana S.C.

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef MAUIKIT_SYSTEM_NETWORK_SECRETS_BACKEND_H
#define MAUIKIT_SYSTEM_NETWORK_SECRETS_BACKEND_H

#include <NetworkManagerQt/Setting>
#include <NetworkManagerQt/Utils>

#include <functional>
#include <QHash>
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

class KeychainBackend final : public SecretsBackend
{
public:
    explicit KeychainBackend(bool secretServiceAvailable);

    QString id() const override;
    bool available() const override;
    void get(const QString &uuid, const QString &setting, const QString &key, ReadCallback callback) const override;
    void set(const QString &uuid, const QString &setting, const QString &key, const QString &value, WriteCallback callback = {}) override;
    void remove(const QString &uuid, const QString &setting, const QString &key, WriteCallback callback = {}) override;
    NetworkManager::Setting::SecretFlags storageFlags() const override;

private:
    static QString keyForSecret(const QString &uuid, const QString &setting, const QString &key);
    mutable QHash<QString, QString> m_secretCache;
    bool m_secretServiceAvailable = false;
};

#endif // MAUIKIT_SYSTEM_NETWORK_SECRETS_BACKEND_H
