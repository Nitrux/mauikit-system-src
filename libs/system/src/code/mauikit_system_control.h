#pragma once

#include <QString>
#include <QStringList>
#include <QVariantList>

namespace MauiKitSystem
{
bool runCommandText(const QString &program,
                    const QStringList &arguments,
                    QString *stdOut = nullptr,
                    int timeoutMs = 350);

int parseBatteryPercent(const QString &value);
QString systemUserRealName();
QString normalizeControlCenterNetworkMode(const QString &value);
QString normalizeControlCenterBluetoothState(const QString &value);
QString normalizeControlCenterVolumeState(const QString &value);
QString normalizeControlCenterIconMode(const QString &value);
QString normalizeBatteryPercentage(const QString &value);
QString defaultRouteInterface();
bool isWirelessInterface(const QString &interfaceName);
QString networkStateFromNmcliStatus();
bool controlCenterBluetoothAvailable();
bool controlCenterBluetoothEnabled();
bool readCpuUsagePercent(int *percent);
bool readRamUsagePercent(int *percent);
bool readDiskUsage(const QString &path, QString *usageText, int *percent);
QStringList powerProfilesFromPowerProfilesCtl();
bool currentPowerProfile(QString *currentProfile);
bool setCurrentPowerProfile(const QString &profile);
bool readBatteryCharge(const QString &batteryPath, QString *capacityText, QString *statusText);
bool batteryPowerSupplyState(QString *batteryPath, bool *mainsOnline);
QVariantList diskUsagePartitionOptions(const QString &currentPath);
bool currentControlCenterVolumeState(QString *percentText, bool *muted);
bool setControlCenterVolumePercent(int percent);
bool currentControlCenterBrightnessPercent(QString *percentText);
bool setControlCenterBrightnessPercent(int percent);
bool controlCenterNightLightAvailable();
bool controlCenterNightLightRunning();
bool startControlCenterNightLight();
bool stopControlCenterNightLight();
bool executeControlCenterPowerCommand(const QString &command);
bool processRunning(const QString &processName);
bool commandAvailable(const QString &program);
bool stopProcessByName(const QString &processName);
bool systemSupportsBacklightAdjustment();
}
