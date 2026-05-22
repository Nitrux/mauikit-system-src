#pragma once

#include "stream.h"

namespace QPipeWireAudio
{
class SourceOutput : public Stream
{
    Q_OBJECT
public:
    explicit SourceOutput(QObject *parent = nullptr);

    void setVolume(qint64 volume) override;
    void setMuted(bool muted) override;
    void setChannelVolume(int channel, qint64 volume) override;
    void setChannelVolumes(const QVector<qint64> &channelVolumes) override;
    void setDeviceIndex(quint32 deviceIndex) override;
};
}
