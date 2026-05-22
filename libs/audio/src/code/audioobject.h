#pragma once

#include <QObject>
#include <QVariantMap>

namespace QPipeWireAudio
{
class AudioObject : public QObject
{
    Q_OBJECT
    Q_PROPERTY(quint32 index READ index CONSTANT)
    Q_PROPERTY(QString iconName READ iconName CONSTANT)
    Q_PROPERTY(QVariantMap properties READ properties NOTIFY propertiesChanged)
public:
    explicit AudioObject(QObject *parent = nullptr);
    ~AudioObject() override;

    quint32 index() const;
    void setIndex(quint32 index);

    QString iconName() const;
    QVariantMap properties() const;
    void setProperties(const QVariantMap &properties);

Q_SIGNALS:
    void propertiesChanged();

protected:
    quint32 m_index = 0;
    QVariantMap m_properties;
};
}
