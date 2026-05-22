#include "pipewirecontext.h"

#include <QDBusConnection>
#include <QDBusConnectionInterface>
#include <QDir>
#include <QStandardPaths>

#include <pipewire/pipewire.h>

PipeWireContext::PipeWireContext(QObject *parent)
    : QObject(parent)
{
    pw_init(nullptr, nullptr);
}

PipeWireContext::~PipeWireContext()
{
    pw_deinit();
}

bool PipeWireContext::pipeWireAvailable() const
{
    const QString runtimeDir = QStandardPaths::writableLocation(QStandardPaths::RuntimeLocation);
    if (!runtimeDir.isEmpty() && QDir(runtimeDir).exists(QStringLiteral("pipewire-0"))) {
        return true;
    }

    const auto *iface = QDBusConnection::sessionBus().interface();
    return iface && iface->isServiceRegistered(QStringLiteral("org.pipewire1"));
}

bool PipeWireContext::wirePlumberAvailable() const
{
    const auto *iface = QDBusConnection::sessionBus().interface();
    if (!iface) {
        return false;
    }

    return iface->isServiceRegistered(QStringLiteral("org.wireplumber"))
        || iface->isServiceRegistered(QStringLiteral("org.wireplumber1"));
}

QString PipeWireContext::stack() const
{
    if (pipeWireAvailable() && wirePlumberAvailable()) {
        return QStringLiteral("pipewire+wireplumber");
    }

    if (pipeWireAvailable()) {
        return QStringLiteral("pipewire");
    }

    return QStringLiteral("legacy");
}
