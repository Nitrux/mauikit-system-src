#include "context.h"

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonParseError>
#include <QJsonValue>
#include <QProcess>
#include <QRegularExpression>
#include <QStandardPaths>
#include <QtGlobal>

namespace QPipeWireAudio
{
Context *Context::s_context = nullptr;
QString Context::s_applicationId;

const qint64 Context::NormalVolume = 100;
const qint64 Context::MinimalVolume = 0;
const qint64 Context::MaximalVolume = 150;

Context::Context(QObject *parent)
    : QObject(parent)
    , m_wpctl(QStandardPaths::findExecutable(QStringLiteral("wpctl")))
{
    refresh();
}

Context::~Context() = default;

Context *Context::instance()
{
    if (!s_context) {
        s_context = new Context();
    }
    return s_context;
}

void Context::ref()
{
    ++m_refCount;
}

void Context::unref()
{
    m_refCount = qMax(0, m_refCount - 1);
}

void Context::setApplicationId(const QString &applicationId)
{
    s_applicationId = applicationId;
}

QList<NodeSnapshot> Context::sinks() const
{
    return m_sinks;
}

QList<NodeSnapshot> Context::sources() const
{
    return m_sources;
}

QList<NodeSnapshot> Context::sinkInputs() const
{
    return m_sinkInputs;
}

QList<NodeSnapshot> Context::sourceOutputs() const
{
    return m_sourceOutputs;
}

QString Context::runWpctl(const QStringList &args)
{
    const QString wpctl = QStandardPaths::findExecutable(QStringLiteral("wpctl"));
    if (wpctl.isEmpty()) {
        return {};
    }

    QProcess proc;
    proc.start(wpctl, args);
    if (!proc.waitForFinished(2500)) {
        proc.kill();
        return {};
    }

    if (proc.exitStatus() != QProcess::NormalExit || proc.exitCode() != 0) {
        return {};
    }

    return QString::fromUtf8(proc.readAllStandardOutput());
}

QString Context::runPwDump()
{
    const QString pwdump = QStandardPaths::findExecutable(QStringLiteral("pw-dump"));
    if (pwdump.isEmpty()) {
        return {};
    }

    QProcess proc;
    proc.start(pwdump, {});
    if (!proc.waitForFinished(3500)) {
        proc.kill();
        return {};
    }

    if (proc.exitStatus() != QProcess::NormalExit || proc.exitCode() != 0) {
        return {};
    }

    return QString::fromUtf8(proc.readAllStandardOutput());
}

bool Context::parseWpctlVolume(const QString &out, qreal &volume, bool &muted)
{
    static const QRegularExpression volumeRe(QStringLiteral(R"(Volume:\s*([0-9]*\.?[0-9]+))"));
    const QRegularExpressionMatch match = volumeRe.match(out);
    if (!match.hasMatch()) {
        return false;
    }

    volume = match.captured(1).toDouble();
    muted = out.contains(QStringLiteral("MUTED"), Qt::CaseInsensitive);
    return true;
}

quint32 Context::parseWpctlObjectId(const QString &out)
{
    static const QRegularExpression idRe(QStringLiteral(R"(\bid\s+(\d+)\b)"));
    const QRegularExpressionMatch match = idRe.match(out);
    if (!match.hasMatch()) {
        return 0;
    }
    return match.captured(1).toUInt();
}

QString Context::parseWpctlNodeName(const QString &out)
{
    static const QRegularExpression nameRe(QStringLiteral(R"re(node\.name\s*=\s*"([^"]+)")re"));
    const QRegularExpressionMatch match = nameRe.match(out);
    if (!match.hasMatch()) {
        return {};
    }
    return match.captured(1).trimmed();
}

void Context::parsePwDumpDefaults(const QJsonArray &objects, QString &defaultSinkName, QString &defaultSourceName)
{
    for (const QJsonValue &value : objects) {
        if (!value.isObject()) {
            continue;
        }

        const QJsonObject object = value.toObject();
        if (!object.value(QStringLiteral("type")).toString().contains(QStringLiteral("Metadata"))) {
            continue;
        }

        const QJsonArray metadata = object.value(QStringLiteral("metadata")).toArray();
        for (const QJsonValue &metaValue : metadata) {
            if (!metaValue.isObject()) {
                continue;
            }

            const QJsonObject meta = metaValue.toObject();
            const QString key = meta.value(QStringLiteral("key")).toString();
            const QJsonValue valueField = meta.value(QStringLiteral("value"));

            QString name;
            if (valueField.isObject()) {
                name = valueField.toObject().value(QStringLiteral("name")).toString();
            } else if (valueField.isString()) {
                const QJsonDocument valueDoc = QJsonDocument::fromJson(valueField.toString().toUtf8());
                if (valueDoc.isObject()) {
                    name = valueDoc.object().value(QStringLiteral("name")).toString();
                }
            }

            if (name.isEmpty()) {
                continue;
            }

            if (key == QLatin1String("default.audio.sink") || key == QLatin1String("default.configured.audio.sink")) {
                if (defaultSinkName.isEmpty()) {
                    defaultSinkName = name;
                }
            } else if (key == QLatin1String("default.audio.source") || key == QLatin1String("default.configured.audio.source")) {
                if (defaultSourceName.isEmpty()) {
                    defaultSourceName = name;
                }
            }
        }
    }
}

void Context::applyWpctlVolumeIfAvailable(quint32 index, NodeSnapshot &node)
{
    const QString getVolumeOut = runWpctl({QStringLiteral("get-volume"), QString::number(index)});
    qreal wpVolume = 0.0;
    bool muted = false;
    if (!parseWpctlVolume(getVolumeOut, wpVolume, muted)) {
        return;
    }

    node.volume = toBackendVolume(wpVolume);
    node.muted = muted;
    node.hasVolume = true;
    node.volumeWritable = true;
}

qint64 Context::toBackendVolume(qreal wpVolume)
{
    const qreal clamped = qBound<qreal>(0.0, wpVolume, 1.5);
    return qRound(clamped * NormalVolume);
}

void Context::refresh()
{
    QList<NodeSnapshot> sinks;
    QList<NodeSnapshot> sources;
    QList<NodeSnapshot> sinkInputs;
    QList<NodeSnapshot> sourceOutputs;

    QString defaultSinkName;
    QString defaultSourceName;

    if (!m_wpctl.isEmpty()) {
        const QString defaultSinkInspect = runWpctl({QStringLiteral("inspect"), QStringLiteral("@DEFAULT_AUDIO_SINK@")});
        const QString defaultSourceInspect = runWpctl({QStringLiteral("inspect"), QStringLiteral("@DEFAULT_AUDIO_SOURCE@")});

        defaultSinkName = parseWpctlNodeName(defaultSinkInspect);
        defaultSourceName = parseWpctlNodeName(defaultSourceInspect);

        if (defaultSinkName.isEmpty()) {
            const quint32 id = parseWpctlObjectId(defaultSinkInspect);
            if (id != 0) {
                defaultSinkName = QString::number(id);
            }
        }

        if (defaultSourceName.isEmpty()) {
            const quint32 id = parseWpctlObjectId(defaultSourceInspect);
            if (id != 0) {
                defaultSourceName = QString::number(id);
            }
        }
    }

    const QString dumpText = runPwDump();
    if (!dumpText.isEmpty()) {
        QJsonParseError error;
        const QJsonDocument doc = QJsonDocument::fromJson(dumpText.toUtf8(), &error);

        if (error.error == QJsonParseError::NoError && doc.isArray()) {
            const QJsonArray objects = doc.array();

            if (defaultSinkName.isEmpty() || defaultSourceName.isEmpty()) {
                parsePwDumpDefaults(objects, defaultSinkName, defaultSourceName);
            }

            for (const QJsonValue &value : objects) {
                if (!value.isObject()) {
                    continue;
                }

                const QJsonObject object = value.toObject();
                if (object.value(QStringLiteral("type")).toString() != QLatin1String("PipeWire:Interface:Node")) {
                    continue;
                }

                const quint32 id = object.value(QStringLiteral("id")).toInt();
                const QJsonObject info = object.value(QStringLiteral("info")).toObject();
                const QJsonObject props = info.value(QStringLiteral("props")).toObject();

                const QString mediaClass = props.value(QStringLiteral("media.class")).toString();
                if (mediaClass.isEmpty()) {
                    continue;
                }

                NodeSnapshot node;
                node.index = id;
                node.deviceIndex = props.value(QStringLiteral("node.target")).toInt();
                if (node.deviceIndex == 0) {
                    node.deviceIndex = props.value(QStringLiteral("target.object")).toInt();
                }

                const QString nodeName = props.value(QStringLiteral("node.name")).toString();
                const QString nodeNick = props.value(QStringLiteral("node.nick")).toString();
                const QString nodeDesc = props.value(QStringLiteral("node.description")).toString();
                const QString appName = props.value(QStringLiteral("application.name")).toString();
                const QString mediaName = props.value(QStringLiteral("media.name")).toString();

                node.name = !nodeName.isEmpty() ? nodeName : (!nodeNick.isEmpty() ? nodeNick : appName);
                node.description = !nodeDesc.isEmpty() ? nodeDesc : (!mediaName.isEmpty() ? mediaName : (!nodeNick.isEmpty() ? nodeNick : node.name));
                node.virtualDevice = props.value(QStringLiteral("node.virtual")).toBool(false) || mediaClass.contains(QStringLiteral("Virtual"), Qt::CaseInsensitive);
                node.channels = {QStringLiteral("FL"), QStringLiteral("FR")};
                node.volume = NormalVolume;
                node.muted = false;
                node.hasVolume = true;
                node.volumeWritable = true;

                const QJsonObject params = info.value(QStringLiteral("params")).toObject();
                const QJsonArray propParams = params.value(QStringLiteral("Props")).toArray();
                if (!propParams.isEmpty() && propParams.first().isObject()) {
                    const QJsonObject paramObj = propParams.first().toObject();
                    if (paramObj.contains(QStringLiteral("volume"))) {
                        node.volume = toBackendVolume(paramObj.value(QStringLiteral("volume")).toDouble(1.0));
                    }
                    if (paramObj.contains(QStringLiteral("mute"))) {
                        node.muted = paramObj.value(QStringLiteral("mute")).toBool(false);
                    }
                }

                applyWpctlVolumeIfAvailable(node.index, node);

                const bool isSink = mediaClass.startsWith(QStringLiteral("Audio/Sink"), Qt::CaseInsensitive);
                const bool isSource = mediaClass.startsWith(QStringLiteral("Audio/Source"), Qt::CaseInsensitive);
                const bool isSinkInput = mediaClass.contains(QStringLiteral("Stream/Output/Audio"), Qt::CaseInsensitive);
                const bool isSourceOutput = mediaClass.contains(QStringLiteral("Stream/Input/Audio"), Qt::CaseInsensitive);

                if (isSink) {
                    node.isDefault = (!defaultSinkName.isEmpty() && (node.name == defaultSinkName || QString::number(node.index) == defaultSinkName));
                    sinks << node;
                } else if (isSource) {
                    node.isDefault = (!defaultSourceName.isEmpty() && (node.name == defaultSourceName || QString::number(node.index) == defaultSourceName));
                    sources << node;
                } else if (isSinkInput) {
                    node.isDefault = false;
                    sinkInputs << node;
                } else if (isSourceOutput) {
                    node.isDefault = false;
                    sourceOutputs << node;
                }
            }
        }
    }

    m_sinks = sinks;
    m_sources = sources;
    m_sinkInputs = sinkInputs;
    m_sourceOutputs = sourceOutputs;

    Q_EMIT updated();
}

bool Context::setNodeVolume(quint32 index, qint64 volume)
{
    if (m_wpctl.isEmpty()) {
        return false;
    }

    const qreal normalized = qBound<qreal>(0.0, volume / static_cast<qreal>(NormalVolume), 1.5);
    const QString value = QString::number(normalized, 'f', 2);

    QProcess proc;
    proc.start(m_wpctl, {QStringLiteral("set-volume"), QString::number(index), value});
    if (!proc.waitForFinished(2500) || proc.exitCode() != 0) {
        return false;
    }

    refresh();
    return true;
}

bool Context::setNodeMuted(quint32 index, bool muted)
{
    if (m_wpctl.isEmpty()) {
        return false;
    }

    QProcess proc;
    proc.start(m_wpctl, {QStringLiteral("set-mute"), QString::number(index), muted ? QStringLiteral("1") : QStringLiteral("0")});
    if (!proc.waitForFinished(2500) || proc.exitCode() != 0) {
        return false;
    }

    refresh();
    return true;
}

bool Context::setDefaultNode(quint32 index)
{
    if (m_wpctl.isEmpty()) {
        return false;
    }

    QProcess proc;
    proc.start(m_wpctl, {QStringLiteral("set-default"), QString::number(index)});
    if (!proc.waitForFinished(2500) || proc.exitCode() != 0) {
        return false;
    }

    refresh();
    return true;
}

} // namespace QPipeWireAudio
