#pragma once

#include <QObject>

namespace BluezQt
{
class Manager;
}

namespace MauiKitSystem
{
class BluetoothControlBackend final : public QObject
{
    Q_OBJECT

public:
    static BluetoothControlBackend *instance();

    bool isAvailable();
    bool isEnabled();
    int connectedDeviceCount();
    bool setEnabled(bool enabled);

private:
    explicit BluetoothControlBackend(QObject *parent = nullptr);

    void ensureInitialized();

    BluezQt::Manager *m_manager = nullptr;
};
}
