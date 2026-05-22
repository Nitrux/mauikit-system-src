#include "sourceoutput.h"

#include "context.h"

namespace QPipeWireAudio
{
SourceOutput::SourceOutput(QObject *parent)
    : Stream(parent)
{
}

void SourceOutput::setVolume(qint64 volume)
{
    if (Context::instance()->setNodeVolume(index(), volume)) {
        VolumeObject::setVolume(volume);
    }
}

void SourceOutput::setMuted(bool muted)
{
    if (Context::instance()->setNodeMuted(index(), muted)) {
        VolumeObject::setMuted(muted);
    }
}

void SourceOutput::setChannelVolume(int channel, qint64 volume)
{
    Q_UNUSED(channel)
    setVolume(volume);
}

void SourceOutput::setChannelVolumes(const QVector<qint64> &channelVolumes)
{
    if (!channelVolumes.isEmpty()) {
        setVolume(channelVolumes.first());
    }
    VolumeObject::setChannelVolumes(channelVolumes);
}

void SourceOutput::setDeviceIndex(quint32 deviceIndex)
{
    Stream::setDeviceIndex(deviceIndex);
}

} // namespace QPipeWireAudio
