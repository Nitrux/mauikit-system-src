#pragma once

#include "audioobject.h"

namespace QPipeWireAudio
{
class VolumeObject : public AudioObject
{
    Q_OBJECT
    Q_PROPERTY(qint64 volume READ volume WRITE setVolume NOTIFY volumeChanged)
    Q_PROPERTY(bool muted READ isMuted WRITE setMuted NOTIFY mutedChanged)
    Q_PROPERTY(bool hasVolume READ hasVolume WRITE setHasVolume NOTIFY hasVolumeChanged)
    Q_PROPERTY(bool volumeWritable READ isVolumeWritable WRITE setVolumeWritable NOTIFY isVolumeWritableChanged)
    Q_PROPERTY(QStringList channels READ channels WRITE setChannels NOTIFY channelsChanged)
    Q_PROPERTY(QStringList rawChannels READ rawChannels WRITE setRawChannels NOTIFY rawChannelsChanged)
    Q_PROPERTY(QVector<qint64> channelVolumes READ channelVolumes WRITE setChannelVolumes NOTIFY channelVolumesChanged)
public:
    explicit VolumeObject(QObject *parent = nullptr);
    ~VolumeObject() override;

    qint64 volume() const;
    virtual void setVolume(qint64 volume);

    bool isMuted() const;
    virtual void setMuted(bool muted);

    bool hasVolume() const;
    void setHasVolume(bool hasVolume);

    bool isVolumeWritable() const;
    void setVolumeWritable(bool writable);

    QStringList channels() const;
    void setChannels(const QStringList &channels);

    QStringList rawChannels() const;
    void setRawChannels(const QStringList &rawChannels);

    QVector<qint64> channelVolumes() const;
    virtual void setChannelVolumes(const QVector<qint64> &channelVolumes);
    Q_INVOKABLE virtual void setChannelVolume(int channel, qint64 volume);

Q_SIGNALS:
    void volumeChanged();
    void mutedChanged();
    void hasVolumeChanged();
    void isVolumeWritableChanged();
    void channelsChanged();
    void rawChannelsChanged();
    void channelVolumesChanged();

protected:
    qint64 m_volume = 0;
    bool m_muted = false;
    bool m_hasVolume = true;
    bool m_volumeWritable = true;
    QStringList m_channels;
    QStringList m_rawChannels;
    QVector<qint64> m_channelVolumes;
};
}
