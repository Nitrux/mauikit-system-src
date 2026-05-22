#pragma once

#include "audioobject.h"

namespace QPipeWireAudio
{
class Client : public AudioObject
{
    Q_OBJECT
    Q_PROPERTY(QString name READ name WRITE setName NOTIFY nameChanged)
public:
    explicit Client(QObject *parent = nullptr);
    ~Client() override;

    QString name() const;
    void setName(const QString &name);

Q_SIGNALS:
    void nameChanged();

private:
    QString m_name;
};
}
