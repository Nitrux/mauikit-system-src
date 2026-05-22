#include "audioobject.h"

namespace QPipeWireAudio
{
AudioObject::AudioObject(QObject *parent)
    : QObject(parent)
{
}

AudioObject::~AudioObject() = default;

quint32 AudioObject::index() const
{
    return m_index;
}

void AudioObject::setIndex(quint32 index)
{
    m_index = index;
}

QString AudioObject::iconName() const
{
    const QString icon = m_properties.value(QStringLiteral("icon-name")).toString();
    if (!icon.isEmpty()) {
        return icon;
    }
    return QStringLiteral("audio-card");
}

QVariantMap AudioObject::properties() const
{
    return m_properties;
}

void AudioObject::setProperties(const QVariantMap &properties)
{
    if (m_properties == properties) {
        return;
    }
    m_properties = properties;
    Q_EMIT propertiesChanged();
}

} // namespace QPipeWireAudio
