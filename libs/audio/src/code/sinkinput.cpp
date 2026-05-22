#include "sinkinput.h"

#include "context.h"

namespace QPipeWireAudio
{
SinkInput::SinkInput(QObject *parent)
    : Stream(parent)
{
}

void SinkInput::setVolume(qint64 volume)
{
    if (Context::instance()->setNodeVolume(index(), volume)) {
        VolumeObject::setVolume(volume);
    }
}

void SinkInput::setMuted(bool muted)
{
    if (Context::instance()->setNodeMuted(index(), muted)) {
        VolumeObject::setMuted(muted);
    }
}

void SinkInput::setChannelVolume(int channel, qint64 volume)
{
    Q_UNUSED(channel)
    setVolume(volume);
}

void SinkInput::setChannelVolumes(const QVector<qint64> &channelVolumes)
{
    if (!channelVolumes.isEmpty()) {
        setVolume(channelVolumes.first());
    }
    VolumeObject::setChannelVolumes(channelVolumes);
}

void SinkInput::setDeviceIndex(quint32 deviceIndex)
{
    Stream::setDeviceIndex(deviceIndex);
}

} // namespace QPipeWireAudio
