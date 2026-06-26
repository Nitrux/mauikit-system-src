#pragma once

#include <QString>
#include <QStringList>
#include <QVariantList>
#include "control_export.h"

namespace MauiKitSystem
{
CONTROL_EXPORT bool runCommandText(const QString &program,
                    const QStringList &arguments,
                    QString *stdOut = nullptr,
                    int timeoutMs = 350);

CONTROL_EXPORT int parseBatteryPercent(const QString &value);
CONTROL_EXPORT QString systemUserRealName();
CONTROL_EXPORT QString normalizeControlCenterNetworkMode(const QString &value);
CONTROL_EXPORT QString normalizeControlCenterBluetoothState(const QString &value);
CONTROL_EXPORT QString normalizeControlCenterVolumeState(const QString &value);
CONTROL_EXPORT QString normalizeControlCenterIconMode(const QString &value);
CONTROL_EXPORT QString normalizeBatteryPercentage(const QString &value);
CONTROL_EXPORT QString defaultRouteInterface();
CONTROL_EXPORT bool isWirelessInterface(const QString &interfaceName);
CONTROL_EXPORT bool controlCenterWirelessAvailable();
CONTROL_EXPORT QString networkStateFromNmcliStatus();
CONTROL_EXPORT bool controlCenterBluetoothAvailable();
CONTROL_EXPORT bool controlCenterBluetoothEnabled();
CONTROL_EXPORT int controlCenterBluetoothConnectedDeviceCount();
CONTROL_EXPORT bool setControlCenterBluetoothEnabled(bool enabled);
CONTROL_EXPORT bool readCpuUsagePercent(int *percent);
CONTROL_EXPORT bool readRamUsagePercent(int *percent);
CONTROL_EXPORT bool readDiskUsage(const QString &path, QString *usageText, int *percent);
CONTROL_EXPORT QStringList powerProfilesFromPowerProfilesCtl();
CONTROL_EXPORT bool currentPowerProfile(QString *currentProfile);
CONTROL_EXPORT bool setCurrentPowerProfile(const QString &profile);
CONTROL_EXPORT bool readBatteryCharge(const QString &batteryPath, QString *capacityText, QString *statusText);
CONTROL_EXPORT bool batteryPowerSupplyState(QString *batteryPath, bool *mainsOnline);
CONTROL_EXPORT QVariantList diskUsagePartitionOptions(const QString &currentPath);
CONTROL_EXPORT bool currentControlCenterVolumeState(QString *percentText, bool *muted);
CONTROL_EXPORT bool setControlCenterVolumePercent(int percent);
CONTROL_EXPORT bool currentControlCenterBrightnessPercent(QString *percentText);
CONTROL_EXPORT bool setControlCenterBrightnessPercent(int percent);
CONTROL_EXPORT bool controlCenterNightLightAvailable();
CONTROL_EXPORT bool controlCenterNightLightRunning();
CONTROL_EXPORT bool startControlCenterNightLight();
CONTROL_EXPORT bool stopControlCenterNightLight();
CONTROL_EXPORT bool executeControlCenterPowerCommand(const QString &command);
CONTROL_EXPORT bool executeControlCenterSettingsCommand(const QString &command);
CONTROL_EXPORT bool processRunning(const QString &processName);
CONTROL_EXPORT bool commandAvailable(const QString &program);
CONTROL_EXPORT bool stopProcessByName(const QString &processName);
CONTROL_EXPORT bool systemSupportsBacklightAdjustment();
}
