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
            if (adapter && adapter->isPowered()) {
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
            if (!device || !device->isConnected()) {
                continue;
            }

            const QString key = device->address().isEmpty() ? device->ubi() : device->address();
            connectedDevices.insert(key);
        }

        return connectedDevices.size();
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
    });
}
}
