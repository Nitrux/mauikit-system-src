#include "stream.h"

namespace QPipeWireAudio
{
Stream::Stream(QObject *parent)
    : VolumeObject(parent)
{
}

Stream::~Stream() = default;

QString Stream::name() const
{
    return m_name;
}

void Stream::setName(const QString &name)
{
    if (m_name == name) {
        return;
    }
    m_name = name;
    Q_EMIT nameChanged();
}

Client *Stream::client() const
{
    return m_client;
}

void Stream::setClient(Client *client)
{
    if (m_client == client) {
        return;
    }
    m_client = client;
    Q_EMIT clientChanged();
}

bool Stream::isVirtualStream() const
{
    return m_virtualStream;
}

void Stream::setVirtualStream(bool virtualStream)
{
    if (m_virtualStream == virtualStream) {
        return;
    }
    m_virtualStream = virtualStream;
    Q_EMIT virtualStreamChanged();
}

quint32 Stream::deviceIndex() const
{
    return m_deviceIndex;
}

void Stream::setDeviceIndex(quint32 deviceIndex)
{
    if (m_deviceIndex == deviceIndex) {
        return;
    }
    m_deviceIndex = deviceIndex;
    Q_EMIT deviceIndexChanged();
}

bool Stream::isCorked() const
{
    return m_corked;
}

void Stream::setCorked(bool corked)
{
    if (m_corked == corked) {
        return;
    }
    m_corked = corked;
    Q_EMIT corkedChanged();
}

} // namespace QPipeWireAudio
