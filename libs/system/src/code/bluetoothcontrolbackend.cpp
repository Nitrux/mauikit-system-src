#include "bluetoothcontrolbackend.h"

#include <BluezQt/Adapter>
#include <BluezQt/Device>
#include <BluezQt/InitManagerJob>
#include <BluezQt/Manager>
#include <BluezQt/PendingCall>

#include <QCoreApplication>
#include <QLoggingCategory>
#include <QMetaObject>
#include <QMutex>
#include <QMutexLocker>
#include <QSet>
#include <QThread>

#include <type_traits>
#include <utility>

Q_LOGGING_CATEGORY(MAUI_BT_CONTROL, "org.mauikit.system.control.bluetooth")

namespace MauiKitSystem
{
namespace
{
template<typename Func>
auto invokeOnBackendThread(BluetoothControlBackend *backend, Func &&func)
    -> std::invoke_result_t<Func>
{
    using ReturnType = std::invoke_result_t<Func>;

    if (QThread::currentThread() == backend->thread())
    {
        if constexpr (std::is_void_v<ReturnType>)
        {
            func();
            return;
        }
        else
        {
            return func();
        }
    }

    if constexpr (std::is_void_v<ReturnType>)
    {
        QMetaObject::invokeMethod(backend, std::forward<Func>(func), Qt::BlockingQueuedConnection);
    }
    else
    {
        ReturnType result{};
        QMetaObject::invokeMethod(backend, [&, func = std::forward<Func>(func)]() mutable {
            result = func();
        }, Qt::BlockingQueuedConnection);
        return result;
    }
}
}

BluetoothControlBackend::BluetoothControlBackend(QObject *parent)
    : QObject(parent)
{
}

BluetoothControlBackend *BluetoothControlBackend::instance()
{
    static QMutex mutex;
    static BluetoothControlBackend *backend = nullptr;

    QMutexLocker locker(&mutex);
    if (backend)
        return backend;

    QCoreApplication *app = QCoreApplication::instance();
    if (!app)
    {
        backend = new BluetoothControlBackend();
        return backend;
    }

    if (QThread::currentThread() == app->thread())
    {
        backend = new BluetoothControlBackend(app);
    }
    else
    {
        QMetaObject::invokeMethod(app, [&]() {
            backend = new BluetoothControlBackend(app);
        }, Qt::BlockingQueuedConnection);
    }

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
    return invokeOnBackendThread(this, [this]() {
        ensureInitialized();
        return m_manager && !m_manager->adapters().isEmpty();
    });
}

bool BluetoothControlBackend::isEnabled()
{
    return invokeOnBackendThread(this, [this]() {
        ensureInitialized();
        if (!m_manager) {
            return false;
        }

        const auto adapters = m_manager->adapters();
        for (const BluezQt::AdapterPtr &adapter : adapters) {
            if (!adapter) {
                continue;
            }

            bool powered = false;
            if (QThread::currentThread() == adapter->thread()) {
                powered = adapter->isPowered();
            } else {
                QMetaObject::invokeMethod(adapter.data(), [adapter, &powered]() {
                    powered = adapter->isPowered();
                }, Qt::BlockingQueuedConnection);
            }

            if (powered) {
                return true;
            }
        }

        return false;
    });
}

int BluetoothControlBackend::connectedDeviceCount()
{
    return invokeOnBackendThread(this, [this]() {
        ensureInitialized();
        if (!m_manager) {
            return 0;
        }

        QSet<QString> connectedDevices;
        const auto devices = m_manager->devices();
        for (const BluezQt::DevicePtr &device : devices) {
            if (!device) {
                continue;
            }

            bool connected = false;
            if (QThread::currentThread() == device->thread()) {
                connected = device->isConnected();
            } else {
                QMetaObject::invokeMethod(device.data(), [device, &connected]() {
                    connected = device->isConnected();
                }, Qt::BlockingQueuedConnection);
            }

            if (!connected) {
                continue;
            }

            QString key;
            if (QThread::currentThread() == device->thread()) {
                const QString address = device->address();
                key = address.isEmpty() ? device->ubi() : address;
            } else {
                QMetaObject::invokeMethod(device.data(), [device, &key]() {
                    const QString address = device->address();
                    key = address.isEmpty() ? device->ubi() : address;
                }, Qt::BlockingQueuedConnection);
            }

            connectedDevices.insert(key);
        }

        return static_cast<int>(connectedDevices.size());
    });
}

bool BluetoothControlBackend::setEnabled(bool enabled)
{
    return invokeOnBackendThread(this, [this, enabled]() {
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
            bool adapterSucceeded = false;
            if (QThread::currentThread() == adapter->thread()) {
                BluezQt::PendingCall *call = adapter->setPowered(enabled);
                if (call) {
                    call->waitForFinished();
                    if (call->error() != BluezQt::PendingCall::NoError) {
                        qCWarning(MAUI_BT_CONTROL) << "Failed to set adapter powered state:" << call->errorText();
                    } else {
                        adapterSucceeded = true;
                    }
                }
            } else {
                QMetaObject::invokeMethod(adapter.data(), [adapter, enabled, &adapterSucceeded]() {
                    BluezQt::PendingCall *call = adapter->setPowered(enabled);
                    if (!call) {
                        return;
                    }

                    call->waitForFinished();
                    if (call->error() != BluezQt::PendingCall::NoError) {
                        qCWarning(MAUI_BT_CONTROL) << "Failed to set adapter powered state:" << call->errorText();
                        return;
                    }

                    adapterSucceeded = true;
                }, Qt::BlockingQueuedConnection);
            }

            if (!adapterSucceeded) {
                allSucceeded = false;
            }
        }

        return anyAdapter && allSucceeded;
    });
}
}
