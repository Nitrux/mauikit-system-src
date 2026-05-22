#pragma once

#include "volumeobject.h"
#include <QtCore/qobjectdefs.h>

Q_MOC_INCLUDE("client.h")

namespace QPipeWireAudio
{
class Client;

class Stream : public VolumeObject
{
    Q_OBJECT
    Q_PROPERTY(QString name READ name WRITE setName NOTIFY nameChanged)
    Q_PROPERTY(QPipeWireAudio::Client *client READ client WRITE setClient NOTIFY clientChanged)
    Q_PROPERTY(bool virtualStream READ isVirtualStream WRITE setVirtualStream NOTIFY virtualStreamChanged)
    Q_PROPERTY(quint32 deviceIndex READ deviceIndex WRITE setDeviceIndex NOTIFY deviceIndexChanged)
    Q_PROPERTY(bool corked READ isCorked WRITE setCorked NOTIFY corkedChanged)
public:
    explicit Stream(QObject *parent = nullptr);
    ~Stream() override;

    QString name() const;
    void setName(const QString &name);

    Client *client() const;
    void setClient(Client *client);

    bool isVirtualStream() const;
    void setVirtualStream(bool virtualStream);

    quint32 deviceIndex() const;
    virtual void setDeviceIndex(quint32 deviceIndex);

    bool isCorked() const;
    void setCorked(bool corked);

Q_SIGNALS:
    void nameChanged();
    void clientChanged();
    void virtualStreamChanged();
    void deviceIndexChanged();
    void corkedChanged();

protected:
    quint32 m_deviceIndex = 0;

private:
    QString m_name;
    Client *m_client = nullptr;
    bool m_virtualStream = false;
    bool m_corked = false;
};
}
