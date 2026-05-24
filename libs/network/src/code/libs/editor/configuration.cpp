/*
    SPDX-FileCopyrightText: 2017 Jan Grulich <jgrulich@redhat.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "configuration.h"
#include <KConfigGroup>
#include <KSharedConfig>
#include <KUser>
#include <QMutexLocker>

namespace
{
KSharedConfigPtr openPrimaryConfig()
{
    return KSharedConfig::openConfig(QStringLiteral("mauikit-system-network"));
}

KSharedConfigPtr openLegacyConfig()
{
    return KSharedConfig::openConfig(QStringLiteral("plasma-nm"));
}

template<typename T>
T readGeneralEntryWithFallback(const QString &key, const T &defaultValue)
{
    KConfigGroup grp(openPrimaryConfig(), QStringLiteral("General"));
    if (grp.hasKey(key)) {
        return grp.readEntry(key, defaultValue);
    }

    KConfigGroup legacyGrp(openLegacyConfig(), QStringLiteral("General"));
    return legacyGrp.readEntry(key, defaultValue);
}
}

static bool propManageVirtualConnectionsInitialized = false;
static bool propManageVirtualConnections = false;
QMutex Configuration::sMutex;

Configuration &Configuration::self()
{
    static Configuration c;
    return c;
}

bool Configuration::unlockModemOnDetection() const
{
    return readGeneralEntryWithFallback(QStringLiteral("UnlockModemOnDetection"), true);
}

void Configuration::setUnlockModemOnDetection(bool unlock)
{
    KSharedConfigPtr config = openPrimaryConfig();
    KConfigGroup grp(config, QStringLiteral("General"));

    if (grp.isValid()) {
        grp.writeEntry(QStringLiteral("UnlockModemOnDetection"), unlock);
    }
}

bool Configuration::manageVirtualConnections() const
{
    // Avoid reading from the config file over and over
    if (propManageVirtualConnectionsInitialized) {
        return propManageVirtualConnections;
    }

    QMutexLocker locker(&sMutex);
    propManageVirtualConnections = readGeneralEntryWithFallback(QStringLiteral("ManageVirtualConnections"), false);
    propManageVirtualConnectionsInitialized = true;

    return propManageVirtualConnections;
}

void Configuration::setManageVirtualConnections(bool manage)
{
    KSharedConfigPtr config = openPrimaryConfig();
    KConfigGroup grp(config, QStringLiteral("General"));

    if (grp.isValid()) {
        {
            QMutexLocker locker(&sMutex);
            grp.writeEntry(QStringLiteral("ManageVirtualConnections"), manage);
            grp.sync();
            propManageVirtualConnections = manage;
        }
        Q_EMIT manageVirtualConnectionsChanged(manage);
    }
}

bool Configuration::airplaneModeEnabled() const
{
    return readGeneralEntryWithFallback(QStringLiteral("AirplaneModeEnabled"), false);
}

void Configuration::setAirplaneModeEnabled(bool enabled)
{
    KSharedConfigPtr config = openPrimaryConfig();
    KConfigGroup grp(config, QStringLiteral("General"));

    if (grp.isValid()) {
        grp.writeEntry(QStringLiteral("AirplaneModeEnabled"), enabled);
        grp.sync();
        Q_EMIT airplaneModeEnabledChanged();
    }
}

QString Configuration::hotspotName() const
{
    KUser currentUser;
    const QString defaultName = currentUser.loginName() + QLatin1String("-hotspot");
    return readGeneralEntryWithFallback(QStringLiteral("HotspotName"), defaultName);
}

void Configuration::setHotspotName(const QString &name)
{
    KSharedConfigPtr config = openPrimaryConfig();
    KConfigGroup grp(config, QStringLiteral("General"));

    if (grp.isValid()) {
        grp.writeEntry(QStringLiteral("HotspotName"), name);
    }
}

QString Configuration::hotspotPassword() const
{
    return readGeneralEntryWithFallback(QStringLiteral("HotspotPassword"), QString());
}

void Configuration::setHotspotPassword(const QString &password)
{
    KSharedConfigPtr config = openPrimaryConfig();
    KConfigGroup grp(config, QStringLiteral("General"));

    if (grp.isValid()) {
        grp.writeEntry(QStringLiteral("HotspotPassword"), password);
    }
}

QString Configuration::hotspotConnectionPath() const
{
    return readGeneralEntryWithFallback(QStringLiteral("HotspotConnectionPath"), QString());
}

void Configuration::setHotspotConnectionPath(const QString &path)
{
    KSharedConfigPtr config = openPrimaryConfig();
    KConfigGroup grp(config, QStringLiteral("General"));

    if (grp.isValid()) {
        grp.writeEntry(QStringLiteral("HotspotConnectionPath"), path);
    }
}

bool Configuration::showPasswordDialog() const
{
    return readGeneralEntryWithFallback(QStringLiteral("ShowPasswordDialog"), true);
}

bool Configuration::systemConnectionsByDefault() const
{
    return readGeneralEntryWithFallback(QStringLiteral("SystemConnectionsByDefault"), false);
}

#include "moc_configuration.cpp"
