#include "card.h"

namespace QPipeWireAudio
{
Card::Card(QObject *parent)
    : AudioObject(parent)
{
}

QString Card::name() const
{
    return m_name;
}

void Card::setName(const QString &name)
{
    if (m_name == name) {
        return;
    }
    m_name = name;
    Q_EMIT nameChanged();
}

QList<QObject *> Card::profiles() const
{
    return {};
}

QList<QObject *> Card::ports() const
{
    return {};
}

quint32 Card::activeProfileIndex() const
{
    return m_activeProfileIndex;
}

void Card::setActiveProfileIndex(quint32 profileIndex)
{
    if (m_activeProfileIndex == profileIndex) {
        return;
    }
    m_activeProfileIndex = profileIndex;
    Q_EMIT activeProfileIndexChanged();
}

} // namespace QPipeWireAudio
