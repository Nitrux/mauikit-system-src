#pragma once

#include "audioobject.h"

namespace QPipeWireAudio
{
class Module : public AudioObject
{
    Q_OBJECT
    Q_PROPERTY(QString name READ name WRITE setName NOTIFY nameChanged)
    Q_PROPERTY(QString argument READ argument WRITE setArgument NOTIFY argumentChanged)

public:
    explicit Module(QObject *parent = nullptr);

    QString name() const;
    void setName(const QString &name);

    QString argument() const;
    void setArgument(const QString &argument);

Q_SIGNALS:
    void nameChanged();
    void argumentChanged();

private:
    QString m_name;
    QString m_argument;
};
}
