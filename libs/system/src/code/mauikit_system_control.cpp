#include "mauikit_system_control.h"
#include "bluetoothcontrolbackend.h"

#include <QByteArray>
#include <QDir>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QVariant>
#include <QVariantMap>
#include <QFile>
#include <QFileInfo>
#include <QProcess>
#include <QRegularExpression>
#include <QtGlobal>
#include <cmath>
#include <pwd.h>
#include <unistd.h>

namespace MauiKitSystem
{
bool runCommandText(const QString &program,
                    const QStringList &arguments,
                    QString *stdOut,
                    int timeoutMs)
{
    QProcess process;
    process.start(program, arguments);

    if (!process.waitForStarted(250))
        return false;

    if (!process.waitForFinished(timeoutMs))
    {
        process.kill();
        process.waitForFinished(250);
        return false;
    }

    if (process.exitStatus() != QProcess::NormalExit || process.exitCode() != 0)
        return false;

    if (stdOut)
        *stdOut = QString::fromLocal8Bit(process.readAllStandardOutput()).trimmed();
    return true;
}

int parseBatteryPercent(const QString &value)
{
    QString numeric = value.trimmed();
    if (numeric.endsWith(QLatin1Char('%')))
        numeric.chop(1);

    bool ok = false;
    const int parsed = numeric.toInt(&ok);
    if (!ok)
        return 0;

    return qBound(0, parsed, 100);
}

QString normalizeControlCenterNetworkMode(const QString &value)
{
    const QString normalized = value.trimmed().toLower();
    if (normalized == QLatin1String("wired") || normalized == QLatin1String("wireless")
        || normalized == QLatin1String("hotspot") || normalized == QLatin1String("vpn")
        || normalized == QLatin1String("cellular") || normalized == QLatin1String("offline")
        || normalized == QLatin1String("auto"))
    {
        return normalized;
    }

    return QStringLiteral("auto");
}

QString normalizeControlCenterBluetoothState(const QString &value)
{
    const QString normalized = value.trimmed().toLower();
    if (normalized == QLatin1String("on") || normalized == QLatin1String("off")
        || normalized == QLatin1String("auto"))
    {
        return normalized;
    }

    return QStringLiteral("auto");
}

QString normalizeControlCenterVolumeState(const QString &value)
{
    const QString normalized = value.trimmed().toLower();
    if (normalized == QLatin1String("muted") || normalized == QLatin1String("low")
        || normalized == QLatin1String("medium") || normalized == QLatin1String("high")
        || normalized == QLatin1String("auto"))
    {
        return normalized;
    }

    return QStringLiteral("auto");
}

QString normalizeControlCenterIconMode(const QString &value)
{
    const QString normalized = value.trimmed().toLower();
    if (normalized == QLatin1String("system16") || normalized == QLatin1String("nerd"))
        return normalized;

    if (normalized == QLatin1String("auto") || normalized.isEmpty())
        return QStringLiteral("system16");

    return QStringLiteral("system16");
}

QString normalizeBatteryPercentage(const QString &value)
{
    const QString normalized = value.trimmed();
    if (normalized.isEmpty())
        return QStringLiteral("0%");

    QString numeric = normalized;
    if (numeric.endsWith(QLatin1Char('%')))
        numeric.chop(1);
    numeric = numeric.trimmed();

    bool ok = false;
    const int parsed = numeric.toInt(&ok);
    if (!ok)
        return QStringLiteral("0%");

    const int bounded = qBound(0, parsed, 100);
    return QStringLiteral("%1%").arg(bounded);
}

QString systemUserRealName()
{
#if defined(Q_OS_UNIX)
    const struct passwd *userInfo = getpwuid(getuid());
    if (!userInfo)
        return QStringLiteral("User");

    const QByteArray userName = userInfo->pw_name ? QByteArray(userInfo->pw_name) : QByteArray();
    QString realName = userInfo->pw_gecos ? QString::fromLocal8Bit(userInfo->pw_gecos) : QString();
    realName = realName.section(QLatin1Char(','), 0, 0).trimmed();

    if (realName.contains(QLatin1Char('&')) && !userName.isEmpty())
    {
        QString replacement = QString::fromLocal8Bit(userName);
        if (!replacement.isEmpty())
        {
            replacement[0] = replacement[0].toUpper();
            realName.replace(QLatin1Char('&'), replacement);
            realName = realName.trimmed();
        }
    }

    if (!realName.isEmpty())
        return realName;

    if (!userName.isEmpty())
        return QString::fromLocal8Bit(userName);
#endif
    return QStringLiteral("User");
}

QString defaultRouteInterface()
{
    QFile routeFile(QStringLiteral("/proc/net/route"));
    if (!routeFile.open(QIODevice::ReadOnly | QIODevice::Text))
        return {};

    while (!routeFile.atEnd())
    {
        const QString line = QString::fromLocal8Bit(routeFile.readLine()).trimmed();
        if (line.isEmpty() || line.startsWith(QStringLiteral("Iface")))
            continue;

        const QStringList fields = line.split(QRegularExpression(QStringLiteral("\\s+")), Qt::SkipEmptyParts);
        if (fields.size() < 2)
            continue;

        if (fields.at(1) == QLatin1String("00000000"))
            return fields.at(0).trimmed();
    }

    return {};
}

bool isWirelessInterface(const QString &interfaceName)
{
    if (interfaceName.isEmpty())
        return false;

    const QFileInfo wirelessDir(QStringLiteral("/sys/class/net/%1/wireless").arg(interfaceName));
    if (wirelessDir.exists() && wirelessDir.isDir())
        return true;

    return interfaceName.startsWith(QLatin1String("wl"), Qt::CaseInsensitive);
}

bool controlCenterWirelessAvailable()
{
    QDir netDir(QStringLiteral("/sys/class/net"));
    const QStringList interfaces = netDir.entryList(QDir::Dirs | QDir::NoDotAndDotDot);
    for (const QString &interfaceName : interfaces)
    {
        if (isWirelessInterface(interfaceName))
            return true;
    }

    return false;
}

QString networkStateFromNmcliStatus()
{
    QString output;
    if (!runCommandText(QStringLiteral("nmcli"),
                        QStringList { QStringLiteral("-t"), QStringLiteral("-f"), QStringLiteral("DEVICE,TYPE,STATE"), QStringLiteral("device"), QStringLiteral("status") },
                        &output))
    {
        return QStringLiteral("offline");
    }

    bool hasWired = false;
    bool hasWireless = false;

    const QStringList lines = output.split(QLatin1Char('\n'), Qt::SkipEmptyParts);
    for (const QString &line : lines)
    {
        const QStringList fields = line.split(QLatin1Char(':'));
        if (fields.size() < 3)
            continue;

        const QString type = fields.at(1).trimmed().toLower();
        const QString state = fields.at(2).trimmed().toLower();
        if (!state.startsWith(QStringLiteral("connected")))
            continue;

        if (type == QLatin1String("ethernet"))
            hasWired = true;
        else if (type == QLatin1String("wifi") || type == QLatin1String("wireless"))
            hasWireless = true;
    }

    if (hasWired)
        return QStringLiteral("wired");
    if (hasWireless)
        return QStringLiteral("wireless");

    return QStringLiteral("offline");
}

bool controlCenterBluetoothAvailable()
{
    return BluetoothControlBackend::instance()->isAvailable();
}

bool controlCenterBluetoothEnabled()
{
    return BluetoothControlBackend::instance()->isEnabled();
}

int controlCenterBluetoothConnectedDeviceCount()
{
    return BluetoothControlBackend::instance()->connectedDeviceCount();
}

bool setControlCenterBluetoothEnabled(bool enabled)
{
    return BluetoothControlBackend::instance()->setEnabled(enabled);
}

bool readCpuUsagePercent(int *percent)
{
    QString output;
    if (!runCommandText(QStringLiteral("top"), QStringList { QStringLiteral("-bn1") }, &output, 1000))
        return false;

    const QStringList lines = output.split(QLatin1Char('\n'), Qt::SkipEmptyParts);
    for (const QString &line : lines)
    {
        if (!line.contains(QStringLiteral("Cpu(s)"), Qt::CaseInsensitive))
            continue;

        const QRegularExpression idlePattern(QStringLiteral(R"(([0-9]+(?:\.[0-9]+)?)\s*id)"));
        const QRegularExpressionMatch match = idlePattern.match(line);
        if (!match.hasMatch())
            return false;

        bool ok = false;
        const double idle = match.captured(1).toDouble(&ok);
        if (!ok)
            return false;

        if (percent)
            *percent = qBound(0, qRound(100.0 - idle), 100);
        return true;
    }

    return false;
}

bool readRamUsagePercent(int *percent)
{
    QString output;
    if (!runCommandText(QStringLiteral("free"), QStringList(), &output, 1000))
        return false;

    const QStringList lines = output.split(QLatin1Char('\n'), Qt::SkipEmptyParts);
    for (const QString &line : lines)
    {
        if (!line.startsWith(QStringLiteral("Mem:")))
            continue;

        const QStringList fields = line.split(QRegularExpression(QStringLiteral(R"(\s+)")), Qt::SkipEmptyParts);
        if (fields.size() < 3)
            return false;

        bool totalOk = false;
        bool usedOk = false;
        const double totalKb = fields.at(1).toDouble(&totalOk);
        const double usedKb = fields.at(2).toDouble(&usedOk);
        if (!totalOk || !usedOk || totalKb <= 0.0)
            return false;

        if (percent)
            *percent = qBound(0, qRound((usedKb / totalKb) * 100.0), 100);
        return true;
    }

    return false;
}

bool batteryPowerSupplyState(QString *batteryPath, bool *mainsOnline)
{
    if (batteryPath)
        batteryPath->clear();
    if (mainsOnline)
        *mainsOnline = false;

    QDir powerSupplyDir(QStringLiteral("/sys/class/power_supply"));
    const QStringList entries = powerSupplyDir.entryList(QDir::Dirs | QDir::NoDotAndDotDot);

    for (const QString &entry : entries)
    {
        const QString supplyPath = powerSupplyDir.absoluteFilePath(entry);
        const QFileInfo typeInfo(supplyPath + QStringLiteral("/type"));
        if (!typeInfo.exists())
            continue;

        QFile typeFile(typeInfo.filePath());
        if (!typeFile.open(QIODevice::ReadOnly))
            continue;

        const QString type = QString::fromUtf8(typeFile.readAll()).trimmed();
        if (type == QLatin1String("Battery") && batteryPath && batteryPath->isEmpty())
        {
            *batteryPath = supplyPath;
            continue;
        }

        if (type == QLatin1String("Mains") && mainsOnline)
        {
            QFile onlineFile(supplyPath + QStringLiteral("/online"));
            if (onlineFile.open(QIODevice::ReadOnly))
            {
                const QString online = QString::fromUtf8(onlineFile.readAll()).trimmed();
                *mainsOnline = *mainsOnline || (online == QLatin1String("1"));
            }
        }
    }

    return batteryPath && !batteryPath->trimmed().isEmpty();
}

bool readBatteryCharge(const QString &batteryPath, QString *capacityText, QString *statusText)
{
    if (capacityText)
        capacityText->clear();
    if (statusText)
        statusText->clear();

    const QString normalizedPath = batteryPath.trimmed();
    if (normalizedPath.isEmpty())
        return false;

    if (capacityText)
    {
        QFile capacityFile(normalizedPath + QStringLiteral("/capacity"));
        if (capacityFile.open(QIODevice::ReadOnly))
            *capacityText = QString::fromUtf8(capacityFile.readAll()).trimmed();
    }

    if (statusText)
    {
        QFile statusFile(normalizedPath + QStringLiteral("/status"));
        if (statusFile.open(QIODevice::ReadOnly))
            *statusText = QString::fromUtf8(statusFile.readAll()).trimmed();
    }

    return true;
}

bool readDiskUsage(const QString &path, QString *usageText, int *percent)
{
    const QString targetPath = path.trimmed().isEmpty() ? QStringLiteral("/") : path.trimmed();
    QString output;
    if (!runCommandText(QStringLiteral("df"), QStringList { QStringLiteral("-h"), QStringLiteral("--output=used,size,pcent"), targetPath }, &output, 1200))
        return false;

    const QStringList lines = output.split(QLatin1Char('\n'), Qt::SkipEmptyParts);
    if (lines.size() < 2)
        return false;

    const QStringList fields = lines.at(1).split(QRegularExpression(QStringLiteral("\\s+")), Qt::SkipEmptyParts);
    if (fields.size() < 3)
        return false;

    if (usageText)
        *usageText = QStringLiteral("%1 / %2").arg(fields.at(0), fields.at(1));

    if (percent)
    {
        QString p = fields.at(2).trimmed();
        if (p.endsWith(QLatin1Char('%')))
            p.chop(1);
        bool ok = false;
        const int parsed = p.toInt(&ok);
        if (!ok)
            return false;
        *percent = qBound(0, parsed, 100);
    }

    return true;
}

QVariantList diskUsagePartitionOptions(const QString &currentPath)
{
    QVariantList options;

    auto appendEntry = [&options](const QJsonObject &device, const auto &appendEntryRef) -> void {
        const QString type = device.value(QStringLiteral("type")).toString().trimmed().toLower();
        const QString mountPoint = device.value(QStringLiteral("mountpoint")).toString().trimmed();
        const QString pathValue = device.value(QStringLiteral("path")).toString().trimmed();
        const QString label = device.value(QStringLiteral("label")).toString().trimmed();

        if (type == QLatin1String("part") && !mountPoint.isEmpty())
        {
            const QString display = label.isEmpty() ? mountPoint : QStringLiteral("%1 (%2)").arg(label, mountPoint);
            options.push_back(QVariantMap {
                {QStringLiteral("path"), mountPoint},
                {QStringLiteral("label"), label.isEmpty() ? mountPoint : label},
                {QStringLiteral("display"), display},
                {QStringLiteral("device"), pathValue},
            });
        }

        const QJsonArray children = device.value(QStringLiteral("children")).toArray();
        for (const QJsonValue &childValue : children)
            appendEntryRef(childValue.toObject(), appendEntryRef);
    };

    QString output;
    if (!runCommandText(QStringLiteral("lsblk"), QStringList { QStringLiteral("-J"), QStringLiteral("-o"), QStringLiteral("PATH,LABEL,MOUNTPOINT,TYPE"), QStringLiteral("-e"), QStringLiteral("7") }, &output, 1500))
    {
        options.push_back(QVariantMap {
            {QStringLiteral("path"), QStringLiteral("/")},
            {QStringLiteral("label"), QStringLiteral("Root")},
            {QStringLiteral("display"), QStringLiteral("Root (/)")},
        });
        return options;
    }

    const QJsonDocument document = QJsonDocument::fromJson(output.toUtf8());
    if (!document.isObject())
        return options;

    const QJsonArray blockDevices = document.object().value(QStringLiteral("blockdevices")).toArray();
    for (const QJsonValue &deviceValue : blockDevices)
        appendEntry(deviceValue.toObject(), appendEntry);

    const QString normalizedCurrentPath = currentPath.trimmed().isEmpty() ? QStringLiteral("/") : currentPath.trimmed();
    bool hasCurrentPath = false;
    for (const QVariant &entryVariant : options)
    {
        const QVariantMap entry = entryVariant.toMap();
        if (entry.value(QStringLiteral("path")).toString() == normalizedCurrentPath)
        {
            hasCurrentPath = true;
            break;
        }
    }

    if (!hasCurrentPath)
    {
        options.prepend(QVariantMap {
            {QStringLiteral("path"), normalizedCurrentPath},
            {QStringLiteral("label"), QStringLiteral("Current")},
            {QStringLiteral("display"), QStringLiteral("Current (%1)").arg(normalizedCurrentPath)},
        });
    }

    if (options.isEmpty())
    {
        options.push_back(QVariantMap {
            {QStringLiteral("path"), QStringLiteral("/")},
            {QStringLiteral("label"), QStringLiteral("Root")},
            {QStringLiteral("display"), QStringLiteral("Root (/)")},
        });
    }

    return options;
}

bool currentPowerProfile(QString *currentProfile)
{
    if (currentProfile)
        currentProfile->clear();

    QString current;
    if (!runCommandText(QStringLiteral("powerprofilesctl"), QStringList { QStringLiteral("get") }, &current))
        return false;

    const QString normalizedCurrent = current.trimmed().toLower();
    if (normalizedCurrent.isEmpty())
        return false;

    if (currentProfile)
        *currentProfile = normalizedCurrent;
    return true;
}

bool setCurrentPowerProfile(const QString &profile)
{
    const QString normalized = profile.trimmed().toLower();
    if (normalized.isEmpty())
        return false;

    if (!commandAvailable(QStringLiteral("powerprofilesctl")))
        return false;

    return runCommandText(QStringLiteral("powerprofilesctl"), QStringList { QStringLiteral("set"), normalized }, nullptr, 1200);
}

QStringList powerProfilesFromPowerProfilesCtl()
{
    QString listedProfiles;
    if (!runCommandText(QStringLiteral("powerprofilesctl"), QStringList { QStringLiteral("list") }, &listedProfiles))
        return {};

    QStringList profiles;
    const QStringList lines = listedProfiles.split(QLatin1Char('\n'), Qt::SkipEmptyParts);
    for (QString line : lines)
    {
        line = line.trimmed();
        if (line.startsWith(QLatin1Char('*')))
            line.remove(0, 1);

        const int colon = line.indexOf(QLatin1Char(':'));
        if (colon < 0)
            continue;

        const QString profileName = line.left(colon).trimmed().toLower();
        if (profileName.isEmpty() || profiles.contains(profileName))
            continue;

        profiles.push_back(profileName);
    }

    return profiles;
}

bool currentControlCenterVolumeState(QString *percentText, bool *muted)
{
    if (percentText)
        percentText->clear();
    if (muted)
        *muted = false;

    QString output;
    if (!runCommandText(QStringLiteral("wpctl"),
                        QStringList { QStringLiteral("get-volume"), QStringLiteral("@DEFAULT_AUDIO_SINK@") },
                        &output))
    {
        return false;
    }

    if (muted)
        *muted = output.contains(QStringLiteral("[MUTED]"), Qt::CaseInsensitive);

    QRegularExpressionMatch match = QRegularExpression(QStringLiteral(R"(([0-9]+(?:\.[0-9]+)?))")).match(output);
    double volumeRatio = 0.0;
    if (match.hasMatch())
        volumeRatio = match.captured(1).toDouble();

    const int percent = qBound(0, static_cast<int>(qRound(volumeRatio * 100.0)), 100);
    if (percentText)
        *percentText = QStringLiteral("%1%").arg(percent);
    return true;
}

bool setControlCenterVolumePercent(int percent)
{
    percent = qBound(0, percent, 100);
    if (!commandAvailable(QStringLiteral("wpctl")))
        return false;

    const QString percentText = QStringLiteral("%1%").arg(percent);
    const bool ok = runCommandText(QStringLiteral("wpctl"), QStringList { QStringLiteral("set-volume"), QStringLiteral("@DEFAULT_AUDIO_SINK@"), percentText }, nullptr, 1200);
    if (ok)
    {
        if (percent <= 0)
            runCommandText(QStringLiteral("wpctl"), QStringList { QStringLiteral("set-mute"), QStringLiteral("@DEFAULT_AUDIO_SINK@"), QStringLiteral("1") }, nullptr, 1200);
        else
            runCommandText(QStringLiteral("wpctl"), QStringList { QStringLiteral("set-mute"), QStringLiteral("@DEFAULT_AUDIO_SINK@"), QStringLiteral("0") }, nullptr, 1200);
    }
    return ok;
}

bool currentControlCenterBrightnessPercent(QString *percentText)
{
    if (percentText)
        percentText->clear();

    if (!commandAvailable(QStringLiteral("brightnessctl")) || !systemSupportsBacklightAdjustment())
        return false;

    QString output;
    if (!runCommandText(QStringLiteral("brightnessctl"), QStringList { QStringLiteral("info") }, &output, 1200))
        return false;

    const QRegularExpressionMatch match = QRegularExpression(QStringLiteral(R"(([0-9]+(?:\.[0-9]+)?)\s*%)")).match(output);
    if (!match.hasMatch())
        return false;

    const int percent = qBound(0, static_cast<int>(std::lround(match.captured(1).toDouble())), 100);
    if (percentText)
        *percentText = QStringLiteral("%1%").arg(percent);
    return true;
}

bool setControlCenterBrightnessPercent(int percent)
{
    percent = qBound(0, percent, 100);
    if (!commandAvailable(QStringLiteral("brightnessctl")) || !systemSupportsBacklightAdjustment())
        return false;

    const QString percentText = QStringLiteral("%1%").arg(percent);
    return runCommandText(QStringLiteral("brightnessctl"), QStringList { QStringLiteral("set"), percentText, QStringLiteral("--quiet") }, nullptr, 1200);
}

bool controlCenterNightLightState(bool *enabled)
{
    if (enabled)
        *enabled = false;

    if (!commandAvailable(QStringLiteral("hyprsunset")))
        return false;

    if (enabled)
        *enabled = processRunning(QStringLiteral("hyprsunset"));
    return true;
}

bool controlCenterNightLightAvailable()
{
    return controlCenterNightLightState(nullptr);
}

bool controlCenterNightLightRunning()
{
    bool enabled = false;
    return controlCenterNightLightState(&enabled) && enabled;
}

bool startControlCenterNightLight()
{
    if (!commandAvailable(QStringLiteral("hyprsunset")))
        return false;

    if (processRunning(QStringLiteral("hyprsunset")))
        return true;

    return QProcess::startDetached(QStringLiteral("hyprsunset"));
}

bool stopControlCenterNightLight()
{
    if (!processRunning(QStringLiteral("hyprsunset")))
        return true;

    return stopProcessByName(QStringLiteral("hyprsunset"));
}

bool executeControlCenterPowerCommand(const QString &command)
{
    if (QProcess::startDetached(QStringLiteral("/bin/sh"), QStringList { QStringLiteral("-lc"), command }))
        return true;

    if (command.compare(QStringLiteral("qmlogout"), Qt::CaseInsensitive) != 0)
        return QProcess::startDetached(QStringLiteral("/bin/sh"), QStringList { QStringLiteral("-lc"), QStringLiteral("qmlogout") });

    return false;
}

bool executeControlCenterSettingsCommand(const QString &command)
{
    const QStringList candidates = {
        command.trimmed(),
        QStringLiteral("systemsettings"),
        QStringLiteral("maui-settings"),
    };

    for (const QString &candidate : candidates)
    {
        if (candidate.isEmpty())
            continue;

        if (QProcess::startDetached(QStringLiteral("/bin/sh"), QStringList { QStringLiteral("-lc"), candidate }))
            return true;
    }

    return false;
}

bool processRunning(const QString &processName)
{
    QString output;
    return runCommandText(QStringLiteral("pgrep"), QStringList { QStringLiteral("-x"), processName }, &output, 250);
}

bool commandAvailable(const QString &program)
{
    QString output;
    return runCommandText(QStringLiteral("sh"), QStringList { QStringLiteral("-lc"), QStringLiteral("command -v %1").arg(program) }, &output, 250) && !output.trimmed().isEmpty();
}

bool stopProcessByName(const QString &processName)
{
    if (runCommandText(QStringLiteral("pkill"), QStringList { QStringLiteral("-x"), processName }, nullptr, 350))
        return true;

    return runCommandText(QStringLiteral("killall"), QStringList { processName }, nullptr, 350);
}

bool systemSupportsBacklightAdjustment()
{
    QDir backlightDir(QStringLiteral("/sys/class/backlight"));
    const QStringList entries = backlightDir.entryList(QDir::Dirs | QDir::NoDotAndDotDot);
    return !entries.isEmpty();
}
}
