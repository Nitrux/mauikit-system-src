#include "volumeobject.h"

namespace QPipeWireAudio
{
VolumeObject::VolumeObject(QObject *parent)
    : AudioObject(parent)
{
}

VolumeObject::~VolumeObject() = default;

qint64 VolumeObject::volume() const
{
    return m_volume;
}

void VolumeObject::setVolume(qint64 volume)
{
    if (m_volume == volume) {
        return;
    }
    m_volume = volume;
    Q_EMIT volumeChanged();
}

bool VolumeObject::isMuted() const
{
    return m_muted;
}

void VolumeObject::setMuted(bool muted)
{
    if (m_muted == muted) {
        return;
    }
    m_muted = muted;
    Q_EMIT mutedChanged();
}

bool VolumeObject::hasVolume() const
{
    return m_hasVolume;
}

void VolumeObject::setHasVolume(bool hasVolume)
{
    if (m_hasVolume == hasVolume) {
        return;
    }
    m_hasVolume = hasVolume;
    Q_EMIT hasVolumeChanged();
}

bool VolumeObject::isVolumeWritable() const
{
    return m_volumeWritable;
}

void VolumeObject::setVolumeWritable(bool writable)
{
    if (m_volumeWritable == writable) {
        return;
    }
    m_volumeWritable = writable;
    Q_EMIT isVolumeWritableChanged();
}

QStringList VolumeObject::channels() const
{
    return m_channels;
}

void VolumeObject::setChannels(const QStringList &channels)
{
    if (m_channels == channels) {
        return;
    }
    m_channels = channels;
    Q_EMIT channelsChanged();
}

QStringList VolumeObject::rawChannels() const
{
    return m_rawChannels;
}

void VolumeObject::setRawChannels(const QStringList &rawChannels)
{
    if (m_rawChannels == rawChannels) {
        return;
    }
    m_rawChannels = rawChannels;
    Q_EMIT rawChannelsChanged();
}

QVector<qint64> VolumeObject::channelVolumes() const
{
    return m_channelVolumes;
}

void VolumeObject::setChannelVolumes(const QVector<qint64> &channelVolumes)
{
    if (m_channelVolumes == channelVolumes) {
        return;
    }
    m_channelVolumes = channelVolumes;
    Q_EMIT channelVolumesChanged();
}

void VolumeObject::setChannelVolume(int channel, qint64 volume)
{
    if (channel < 0) {
        return;
    }

    if (m_channelVolumes.size() <= channel) {
        m_channelVolumes.resize(channel + 1);
    }

    if (m_channelVolumes.at(channel) == volume) {
        return;
    }

    m_channelVolumes[channel] = volume;
    Q_EMIT channelVolumesChanged();
}

} // namespace QPipeWireAudio
