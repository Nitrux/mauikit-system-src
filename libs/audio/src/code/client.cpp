#include "client.h"

namespace QPipeWireAudio
{
Client::Client(QObject *parent)
    : AudioObject(parent)
{
}

Client::~Client() = default;

QString Client::name() const
{
    return m_name;
}

void Client::setName(const QString &name)
{
    if (m_name == name) {
        return;
    }
    m_name = name;
    Q_EMIT nameChanged();
}

} // namespace QPipeWireAudio
