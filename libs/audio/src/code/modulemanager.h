#pragma once

#include <QObject>

namespace QPipeWireAudio
{
class ModuleManager : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool settingsSupported READ settingsSupported NOTIFY serverUpdated)
    Q_PROPERTY(bool combineSinks READ combineSinks WRITE setCombineSinks NOTIFY combineSinksChanged)
    Q_PROPERTY(bool switchOnConnect READ switchOnConnect WRITE setSwitchOnConnect NOTIFY switchOnConnectChanged)
    Q_PROPERTY(bool configModuleLoaded READ configModuleLoaded NOTIFY loadedModulesChanged)
    Q_PROPERTY(QString configModuleName READ configModuleName CONSTANT)
    Q_PROPERTY(QStringList loadedModules READ loadedModules NOTIFY loadedModulesChanged)
public:
    explicit ModuleManager(QObject *parent = nullptr);
    ~ModuleManager() override;

    bool settingsSupported() const;
    bool combineSinks() const;
    void setCombineSinks(bool combineSinks);

    bool switchOnConnect() const;
    void setSwitchOnConnect(bool switchOnConnect);

    QStringList loadedModules() const;
    bool configModuleLoaded() const;
    QString configModuleName() const;

Q_SIGNALS:
    void combineSinksChanged();
    void switchOnConnectChanged();
    void loadedModulesChanged();
    void serverUpdated();

private:
    bool m_combineSinks = false;
    bool m_switchOnConnect = false;
};
}
