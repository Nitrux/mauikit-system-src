#include "sink.h"

#include "context.h"

namespace QPipeWireAudio
{
Sink::Sink(QObject *parent)
    : Device(parent)
{
}

Sink::~Sink() = default;

void Sink::setVolume(qint64 volume)
{
    if (Context::instance()->setNodeVolume(index(), volume)) {
        VolumeObject::setVolume(volume);
    }
}

void Sink::setMuted(bool muted)
{
    if (Context::instance()->setNodeMuted(index(), muted)) {
        VolumeObject::setMuted(muted);
    }
}

void Sink::setActivePortIndex(quint32 portIndex)
{
    Device::setActivePortIndex(portIndex);
}

void Sink::setChannelVolume(int channel, qint64 volume)
{
    Q_UNUSED(channel)
    setVolume(volume);
}

void Sink::setChannelVolumes(const QVector<qint64> &channelVolumes)
{
    if (!channelVolumes.isEmpty()) {
        setVolume(channelVolumes.first());
    }
    VolumeObject::setChannelVolumes(channelVolumes);
}

bool Sink::isDefault() const
{
    return Device::isDefault();
}

void Sink::setDefault(bool enable)
{
    if (!enable) {
        return;
    }

    if (Context::instance()->setDefaultNode(index())) {
        Device::setDefault(true);
    }
}

void Sink::switchStreams()
{
}

quint32 Sink::monitorIndex() const
{
    return m_monitorIndex;
}

} // namespace QPipeWireAudio
