#include "server.h"

namespace QPipeWireAudio
{
Server::Server(QObject *parent)
    : QObject(parent)
{
}

Sink *Server::defaultSink() const
{
    return m_defaultSink;
}

void Server::setDefaultSink(Sink *sink)
{
    if (m_defaultSink == sink) {
        return;
    }
    m_defaultSink = sink;
    Q_EMIT defaultSinkChanged(m_defaultSink);
    Q_EMIT updated();
}

Source *Server::defaultSource() const
{
    return m_defaultSource;
}

void Server::setDefaultSource(Source *source)
{
    if (m_defaultSource == source) {
        return;
    }
    m_defaultSource = source;
    Q_EMIT defaultSourceChanged(m_defaultSource);
    Q_EMIT updated();
}

void Server::reset()
{
    m_defaultSink = nullptr;
    m_defaultSource = nullptr;
    Q_EMIT updated();
}

bool Server::isPipeWire() const
{
    return true;
}

} // namespace QPipeWireAudio
