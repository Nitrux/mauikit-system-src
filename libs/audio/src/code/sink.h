#pragma once

#include "device.h"

namespace QPipeWireAudio
{
class Sink : public Device
{
    Q_OBJECT
public:
    explicit Sink(QObject *parent = nullptr);
    ~Sink() override;

    void setVolume(qint64 volume) override;
    void setMuted(bool muted) override;
    void setActivePortIndex(quint32 portIndex) override;
    void setChannelVolume(int channel, qint64 volume) override;
    void setChannelVolumes(const QVector<qint64> &channelVolumes) override;

    bool isDefault() const override;
    void setDefault(bool enable) override;

    void switchStreams() override;

    quint32 monitorIndex() const;

Q_SIGNALS:
    void monitorIndexChanged();

private:
    quint32 m_monitorIndex = 0;
};
}
