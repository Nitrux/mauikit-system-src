#include "bluetoothcontrolbackend.h"

#include <BluezQt/Adapter>
#include <BluezQt/Device>
#include <BluezQt/InitManagerJob>
#include <BluezQt/Manager>
#include <BluezQt/PendingCall>

#include <QCoreApplication>
#include <QSet>
#include <QLoggingCategory>

Q_LOGGING_CATEGORY(MAUI_BT_CONTROL, "org.mauikit.system.control.bluetooth")

namespace MauiKitSystem
{
BluetoothControlBackend::BluetoothControlBackend(QObject *parent)
    : QObject(parent)
{
}

BluetoothControlBackend *BluetoothControlBackend::instance()
{
    static BluetoothControlBackend *backend = new BluetoothControlBackend(QCoreApplication::instance());
    return backend;
}

void BluetoothControlBackend::ensureInitialized()
{
    if (!m_manager) {
        m_manager = new BluezQt::Manager(this);
    }

    if (m_manager->isInitialized()) {
        return;
    }

    BluezQt::InitManagerJob *job = m_manager->init();
    if (!job->exec()) {
        qCWarning(MAUI_BT_CONTROL) << "Failed to initialize BluezQt manager";
    }
}

bool BluetoothControlBackend::isAvailable()
{
    ensureInitialized();
    return m_manager && !m_manager->adapters().isEmpty();
}

bool BluetoothControlBackend::isEnabled()
{
    ensureInitialized();
    if (!m_manager) {
        return false;
    }

    const auto adapters = m_manager->adapters();
    for (const BluezQt::AdapterPtr &adapter : adapters) {
        if (adapter && adapter->isPowered()) {
            return true;
        }
    }

    return false;
}

int BluetoothControlBackend::connectedDeviceCount()
{
    ensureInitialized();
    if (!m_manager) {
        return 0;
    }

    QSet<QString> connectedDevices;
    const auto devices = m_manager->devices();
    for (const BluezQt::DevicePtr &device : devices) {
        if (!device || !device->isConnected()) {
            continue;
        }

        const QString key = device->address().isEmpty() ? device->ubi() : device->address();
        connectedDevices.insert(key);
    }

    return connectedDevices.size();
}

bool BluetoothControlBackend::setEnabled(bool enabled)
{
    ensureInitialized();
    if (!m_manager) {
        return false;
    }

    const auto adapters = m_manager->adapters();
    bool anyAdapter = false;
    bool allSucceeded = true;

    for (const BluezQt::AdapterPtr &adapter : adapters) {
        if (!adapter) {
            continue;
        }

        anyAdapter = true;
        BluezQt::PendingCall *call = adapter->setPowered(enabled);
        if (!call) {
            allSucceeded = false;
            continue;
        }

        call->waitForFinished();
        if (call->error() != BluezQt::PendingCall::NoError) {
            qCWarning(MAUI_BT_CONTROL) << "Failed to set adapter powered state:" << call->errorText();
            allSucceeded = false;
        }
    }

    return anyAdapter && allSucceeded;
}
}
