#pragma once

#include "audioobject.h"

namespace QPipeWireAudio
{
class Card : public AudioObject
{
    Q_OBJECT
    Q_PROPERTY(QString name READ name WRITE setName NOTIFY nameChanged)
    Q_PROPERTY(QList<QObject *> profiles READ profiles NOTIFY profilesChanged)
    Q_PROPERTY(quint32 activeProfileIndex READ activeProfileIndex WRITE setActiveProfileIndex NOTIFY activeProfileIndexChanged)
    Q_PROPERTY(QList<QObject *> ports READ ports NOTIFY portsChanged)
public:
    explicit Card(QObject *parent = nullptr);

    QString name() const;
    void setName(const QString &name);

    QList<QObject *> profiles() const;
    QList<QObject *> ports() const;

    quint32 activeProfileIndex() const;
    void setActiveProfileIndex(quint32 profileIndex);

Q_SIGNALS:
    void nameChanged();
    void profilesChanged();
    void activeProfileIndexChanged();
    void portsChanged();

private:
    QString m_name;
    quint32 m_activeProfileIndex = 0;
};
}
