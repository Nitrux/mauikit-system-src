#include "streamrestore.h"

namespace QPipeWireAudio
{
StreamRestore::StreamRestore(QObject *parent)
    : AudioObject(parent)
{
}

QString StreamRestore::name() const
{
    return m_name;
}

QString StreamRestore::device() const
{
    return m_device;
}

void StreamRestore::setDevice(const QString &device)
{
    if (m_device == device) {
        return;
    }
    m_device = device;
    Q_EMIT deviceChanged();
}

qint64 StreamRestore::volume() const
{
    return m_volume;
}

void StreamRestore::setVolume(qint64 volume)
{
    if (m_volume == volume) {
        return;
    }
    m_volume = volume;
    Q_EMIT volumeChanged();
}

bool StreamRestore::isMuted() const
{
    return m_muted;
}

void StreamRestore::setMuted(bool muted)
{
    if (m_muted == muted) {
        return;
    }
    m_muted = muted;
    Q_EMIT mutedChanged();
}

bool StreamRestore::hasVolume() const
{
    return true;
}

bool StreamRestore::isVolumeWritable() const
{
    return true;
}

QStringList StreamRestore::channels() const
{
    return m_channels;
}

QList<qreal> StreamRestore::channelVolumes() const
{
    return m_channelVolumes;
}

quint32 StreamRestore::deviceIndex() const
{
    return m_deviceIndex;
}

void StreamRestore::setDeviceIndex(quint32 deviceIndex)
{
    if (m_deviceIndex == deviceIndex) {
        return;
    }
    m_deviceIndex = deviceIndex;
    Q_EMIT deviceIndexChanged();
}

void StreamRestore::setChannelVolume(int channel, qint64 volume)
{
    if (channel < 0) {
        return;
    }

    while (m_channelVolumes.size() <= channel) {
        m_channelVolumes << 0.0;
    }

    const qreal normalized = qBound<qreal>(0.0, volume / 100.0, 1.5);
    if (qFuzzyCompare(1.0 + m_channelVolumes.at(channel), 1.0 + normalized)) {
        return;
    }

    m_channelVolumes[channel] = normalized;
    Q_EMIT channelVolumesChanged();
}

} // namespace QPipeWireAudio
