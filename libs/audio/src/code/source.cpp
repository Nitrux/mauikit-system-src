#include "source.h"

#include "context.h"

namespace QPipeWireAudio
{
Source::Source(QObject *parent)
    : Device(parent)
{
}

void Source::setVolume(qint64 volume)
{
    if (Context::instance()->setNodeVolume(index(), volume)) {
        VolumeObject::setVolume(volume);
    }
}

void Source::setMuted(bool muted)
{
    if (Context::instance()->setNodeMuted(index(), muted)) {
        VolumeObject::setMuted(muted);
    }
}

void Source::setActivePortIndex(quint32 portIndex)
{
    Device::setActivePortIndex(portIndex);
}

void Source::setChannelVolume(int channel, qint64 volume)
{
    Q_UNUSED(channel)
    setVolume(volume);
}

void Source::setChannelVolumes(const QVector<qint64> &volumes)
{
    if (!volumes.isEmpty()) {
        setVolume(volumes.first());
    }
    VolumeObject::setChannelVolumes(volumes);
}

bool Source::isDefault() const
{
    return Device::isDefault();
}

void Source::setDefault(bool enable)
{
    if (!enable) {
        return;
    }

    if (Context::instance()->setDefaultNode(index())) {
        Device::setDefault(true);
    }
}

void Source::switchStreams()
{
}

} // namespace QPipeWireAudio
