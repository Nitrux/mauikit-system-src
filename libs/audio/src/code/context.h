#pragma once

#include <QObject>
#include <QString>
#include <QStringList>
#include <QJsonArray>

namespace QPipeWireAudio
{
struct NodeSnapshot {
    quint32 index = 0;
    quint32 deviceIndex = 0;
    QString name;
    QString description;
    qint64 volume = 0;
    bool muted = false;
    bool isDefault = false;
    bool hasVolume = true;
    bool volumeWritable = true;
    bool virtualDevice = false;
    QStringList channels;
};

class Context : public QObject
{
    Q_OBJECT
public:
    explicit Context(QObject *parent = nullptr);
    ~Context() override;

    static Context *instance();

    static const qint64 NormalVolume;
    static const qint64 MinimalVolume;
    static const qint64 MaximalVolume;

    void ref();
    void unref();

    static void setApplicationId(const QString &applicationId);

    QList<NodeSnapshot> sinks() const;
    QList<NodeSnapshot> sources() const;
    QList<NodeSnapshot> sinkInputs() const;
    QList<NodeSnapshot> sourceOutputs() const;

    bool setNodeVolume(quint32 index, qint64 volume);
    bool setNodeMuted(quint32 index, bool muted);
    bool setDefaultNode(quint32 index);

public Q_SLOTS:
    void refresh();

Q_SIGNALS:
    void updated();

private:
    static QString runWpctl(const QStringList &args);
    static QString runPwDump();
    static bool parseWpctlVolume(const QString &out, qreal &volume, bool &muted);
    static quint32 parseWpctlObjectId(const QString &out);
    static QString parseWpctlNodeName(const QString &out);
    static void parsePwDumpDefaults(const QJsonArray &objects, QString &defaultSinkName, QString &defaultSourceName);
    static void applyWpctlVolumeIfAvailable(quint32 index, NodeSnapshot &node);
    static qint64 toBackendVolume(qreal wpVolume);

    QString m_wpctl;
    int m_refCount = 0;

    QList<NodeSnapshot> m_sinks;
    QList<NodeSnapshot> m_sources;
    QList<NodeSnapshot> m_sinkInputs;
    QList<NodeSnapshot> m_sourceOutputs;

    static Context *s_context;
    static QString s_applicationId;
};
}
