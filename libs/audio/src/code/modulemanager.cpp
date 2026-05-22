#include "modulemanager.h"

namespace QPipeWireAudio
{
ModuleManager::ModuleManager(QObject *parent)
    : QObject(parent)
{
}

ModuleManager::~ModuleManager() = default;

bool ModuleManager::settingsSupported() const
{
    return false;
}

bool ModuleManager::combineSinks() const
{
    return m_combineSinks;
}

void ModuleManager::setCombineSinks(bool combineSinks)
{
    if (m_combineSinks == combineSinks) {
        return;
    }
    m_combineSinks = combineSinks;
    Q_EMIT combineSinksChanged();
}

bool ModuleManager::switchOnConnect() const
{
    return m_switchOnConnect;
}

void ModuleManager::setSwitchOnConnect(bool switchOnConnect)
{
    if (m_switchOnConnect == switchOnConnect) {
        return;
    }
    m_switchOnConnect = switchOnConnect;
    Q_EMIT switchOnConnectChanged();
}

QStringList ModuleManager::loadedModules() const
{
    return {};
}

bool ModuleManager::configModuleLoaded() const
{
    return false;
}

QString ModuleManager::configModuleName() const
{
    return QStringLiteral("wireplumber");
}

} // namespace QPipeWireAudio
