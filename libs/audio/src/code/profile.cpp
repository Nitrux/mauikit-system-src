#include "profile.h"

namespace QPipeWireAudio
{
Profile::Profile(QObject *parent)
    : QObject(parent)
{
}

Profile::~Profile() = default;

QString Profile::name() const
{
    return m_name;
}

void Profile::setName(const QString &name)
{
    if (m_name == name) {
        return;
    }
    m_name = name;
    Q_EMIT nameChanged();
}

QString Profile::description() const
{
    return m_description;
}

void Profile::setDescription(const QString &description)
{
    if (m_description == description) {
        return;
    }
    m_description = description;
    Q_EMIT descriptionChanged();
}

quint32 Profile::priority() const
{
    return m_priority;
}

void Profile::setPriority(quint32 priority)
{
    if (m_priority == priority) {
        return;
    }
    m_priority = priority;
    Q_EMIT priorityChanged();
}

Profile::Availability Profile::availability() const
{
    return m_availability;
}

void Profile::setAvailability(Availability availability)
{
    if (m_availability == availability) {
        return;
    }
    m_availability = availability;
    Q_EMIT availabilityChanged();
}

} // namespace QPipeWireAudio
