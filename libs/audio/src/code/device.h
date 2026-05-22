#pragma once

#include "port.h"
#include "volumeobject.h"

namespace QPipeWireAudio
{
class Device : public VolumeObject
{
    Q_OBJECT
    Q_PROPERTY(State state READ state WRITE setState NOTIFY stateChanged)
    Q_PROPERTY(QString name READ name WRITE setName NOTIFY nameChanged)
    Q_PROPERTY(QString description READ description WRITE setDescription NOTIFY descriptionChanged)
    Q_PROPERTY(QString formFactor READ formFactor WRITE setFormFactor NOTIFY formFactorChanged)
    Q_PROPERTY(quint32 cardIndex READ cardIndex WRITE setCardIndex NOTIFY cardIndexChanged)
    Q_PROPERTY(QList<QObject *> ports READ ports NOTIFY portsChanged)
    Q_PROPERTY(quint32 activePortIndex READ activePortIndex WRITE setActivePortIndex NOTIFY activePortIndexChanged)
    Q_PROPERTY(bool default READ isDefault WRITE setDefault NOTIFY defaultChanged)
    Q_PROPERTY(bool virtualDevice READ isVirtualDevice WRITE setVirtualDevice NOTIFY virtualDeviceChanged)
public:
    enum State {
        InvalidState = 0,
        RunningState,
        IdleState,
        SuspendedState,
        UnknownState,
    };
    Q_ENUM(State)

    explicit Device(QObject *parent = nullptr);
    ~Device() override;

    State state() const;
    void setState(State state);

    QString name() const;
    void setName(const QString &name);

    QString description() const;
    void setDescription(const QString &description);

    QString formFactor() const;
    void setFormFactor(const QString &formFactor);

    quint32 cardIndex() const;
    void setCardIndex(quint32 cardIndex);

    QList<QObject *> ports() const;
    void setPorts(const QStringList &portDescriptions);

    quint32 activePortIndex() const;
    virtual void setActivePortIndex(quint32 portIndex);

    virtual bool isDefault() const;
    virtual void setDefault(bool enable);

    bool isVirtualDevice() const;
    void setVirtualDevice(bool virtualDevice);

    Q_INVOKABLE virtual void switchStreams();

Q_SIGNALS:
    void stateChanged();
    void nameChanged();
    void descriptionChanged();
    void formFactorChanged();
    void cardIndexChanged();
    void portsChanged();
    void activePortIndexChanged();
    void defaultChanged();
    void virtualDeviceChanged();

protected:
    QString m_name;
    QString m_description;
    QString m_formFactor;
    quint32 m_cardIndex = 0;
    QList<QObject *> m_ports;
    quint32 m_activePortIndex = 0;
    State m_state = UnknownState;
    bool m_isDefault = false;
    bool m_virtualDevice = false;
};
}
