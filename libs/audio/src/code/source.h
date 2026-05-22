#pragma once

#include "device.h"

namespace QPipeWireAudio
{
class Source : public Device
{
    Q_OBJECT
public:
    explicit Source(QObject *parent = nullptr);

    void setVolume(qint64 volume) override;
    void setMuted(bool muted) override;
    void setActivePortIndex(quint32 portIndex) override;
    void setChannelVolume(int channel, qint64 volume) override;
    void setChannelVolumes(const QVector<qint64> &volumes) override;

    bool isDefault() const override;
    void setDefault(bool enable) override;

    void switchStreams() override;
};
}
