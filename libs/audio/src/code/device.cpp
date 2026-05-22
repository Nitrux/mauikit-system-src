#include "device.h"

namespace QPipeWireAudio
{
Device::Device(QObject *parent)
    : VolumeObject(parent)
{
}

Device::~Device() = default;

Device::State Device::state() const
{
    return m_state;
}

void Device::setState(State state)
{
    if (m_state == state) {
        return;
    }
    m_state = state;
    Q_EMIT stateChanged();
}

QString Device::name() const
{
    return m_name;
}

void Device::setName(const QString &name)
{
    if (m_name == name) {
        return;
    }
    m_name = name;
    Q_EMIT nameChanged();
}

QString Device::description() const
{
    return m_description;
}

void Device::setDescription(const QString &description)
{
    if (m_description == description) {
        return;
    }
    m_description = description;
    Q_EMIT descriptionChanged();
}

QString Device::formFactor() const
{
    return m_formFactor;
}

void Device::setFormFactor(const QString &formFactor)
{
    if (m_formFactor == formFactor) {
        return;
    }
    m_formFactor = formFactor;
    Q_EMIT formFactorChanged();
}

quint32 Device::cardIndex() const
{
    return m_cardIndex;
}

void Device::setCardIndex(quint32 cardIndex)
{
    if (m_cardIndex == cardIndex) {
        return;
    }
    m_cardIndex = cardIndex;
    Q_EMIT cardIndexChanged();
}

QList<QObject *> Device::ports() const
{
    return m_ports;
}

void Device::setPorts(const QStringList &portDescriptions)
{
    bool changed = false;

    while (m_ports.size() > portDescriptions.size()) {
        delete m_ports.takeLast();
        changed = true;
    }

    for (int i = 0; i < portDescriptions.size(); ++i) {
        Port *port = nullptr;
        if (i < m_ports.size()) {
            port = static_cast<Port *>(m_ports.at(i));
        } else {
            port = new Port(this);
            m_ports.append(port);
            changed = true;
        }

        const QString desc = portDescriptions.at(i);
        if (port->description() != desc) {
            port->setDescription(desc);
            port->setName(desc);
            port->setAvailability(Profile::Available);
            changed = true;
        }
    }

    if (changed) {
        Q_EMIT portsChanged();
    }
}

quint32 Device::activePortIndex() const
{
    return m_activePortIndex;
}

void Device::setActivePortIndex(quint32 portIndex)
{
    if (m_activePortIndex == portIndex) {
        return;
    }
    m_activePortIndex = portIndex;
    Q_EMIT activePortIndexChanged();
}

bool Device::isDefault() const
{
    return m_isDefault;
}

void Device::setDefault(bool enable)
{
    if (m_isDefault == enable) {
        return;
    }
    m_isDefault = enable;
    Q_EMIT defaultChanged();
}

bool Device::isVirtualDevice() const
{
    return m_virtualDevice;
}

void Device::setVirtualDevice(bool virtualDevice)
{
    if (m_virtualDevice == virtualDevice) {
        return;
    }
    m_virtualDevice = virtualDevice;
    Q_EMIT virtualDeviceChanged();
}

void Device::switchStreams()
{
}

} // namespace QPipeWireAudio
