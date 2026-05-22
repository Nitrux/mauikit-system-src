#pragma once

#include "audioobject.h"

namespace QPipeWireAudio
{
class StreamRestore : public AudioObject
{
    Q_OBJECT
    Q_PROPERTY(QString name READ name NOTIFY nameChanged)
    Q_PROPERTY(QString device READ device WRITE setDevice NOTIFY deviceChanged)
    Q_PROPERTY(qint64 volume READ volume WRITE setVolume NOTIFY volumeChanged)
    Q_PROPERTY(bool muted READ isMuted WRITE setMuted NOTIFY mutedChanged)
    Q_PROPERTY(bool hasVolume READ hasVolume CONSTANT)
    Q_PROPERTY(bool volumeWritable READ isVolumeWritable CONSTANT)
    Q_PROPERTY(QStringList channels READ channels NOTIFY channelsChanged)
    Q_PROPERTY(QList<qreal> channelVolumes READ channelVolumes NOTIFY channelVolumesChanged)
    Q_PROPERTY(quint32 deviceIndex READ deviceIndex WRITE setDeviceIndex NOTIFY deviceIndexChanged)
public:
    explicit StreamRestore(QObject *parent = nullptr);

    QString name() const;

    QString device() const;
    void setDevice(const QString &device);

    qint64 volume() const;
    void setVolume(qint64 volume);

    bool isMuted() const;
    void setMuted(bool muted);

    bool hasVolume() const;
    bool isVolumeWritable() const;

    QStringList channels() const;
    QList<qreal> channelVolumes() const;

    quint32 deviceIndex() const;
    void setDeviceIndex(quint32 deviceIndex);

    Q_INVOKABLE void setChannelVolume(int channel, qint64 volume);

Q_SIGNALS:
    void nameChanged();
    void deviceChanged();
    void volumeChanged();
    void mutedChanged();
    void channelsChanged();
    void channelVolumesChanged();
    void deviceIndexChanged();

private:
    QString m_name;
    QString m_device;
    qint64 m_volume = 0;
    bool m_muted = false;
    QStringList m_channels = {QStringLiteral("FL"), QStringLiteral("FR")};
    QList<qreal> m_channelVolumes = {1.0, 1.0};
    quint32 m_deviceIndex = 0;
};
}
