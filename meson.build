project('mauikit-system', 'cpp', version: '4.0.5', meson_version: '>=1.12.0', default_options: ['cpp_std=c++20', 'buildtype=release'])

cpp_compiler = meson.get_compiler('cpp')
if cpp_compiler.get_id() == 'gcc' and cpp_compiler.has_argument('-Wno-sfinae-incomplete')
    add_project_arguments('-Wno-sfinae-incomplete', language: 'cpp')
endif
qt6 = import('qt6')
qt = dependency('qt6', version: '>=6.9', modules: ['Core', 'Gui', 'DBus', 'Qml', 'Quick'])
qt_moc_args = ['-I' + qt.get_variable('includedir')]
kf6_core = declare_dependency(include_directories: include_directories(get_option('prefix') / 'include' / 'KF6' / 'KCoreAddons'), dependencies: [cpp_compiler.find_library('KF6CoreAddons')])
kf6_i18n = declare_dependency(include_directories: include_directories(get_option('prefix') / 'include' / 'KF6' / 'KI18n'), dependencies: [cpp_compiler.find_library('KF6I18n')])
kf6_notifications = declare_dependency(include_directories: include_directories(get_option('prefix') / 'include' / 'KF6' / 'KNotifications'), dependencies: [cpp_compiler.find_library('KF6Notifications')])
kf6_bluez = declare_dependency(include_directories: include_directories(get_option('prefix') / 'include' / 'KF6' / 'BluezQt'), dependencies: [cpp_compiler.find_library('KF6BluezQt')])
kf6_network = declare_dependency(include_directories: include_directories(get_option('prefix') / 'include' / 'KF6' / 'NetworkManagerQt'), dependencies: [cpp_compiler.find_library('KF6NetworkManagerQt')])
kf6_modem = declare_dependency(include_directories: include_directories(get_option('prefix') / 'include' / 'KF6' / 'ModemManagerQt', get_option('prefix') / 'include' / 'ModemManager'), dependencies: [cpp_compiler.find_library('KF6ModemManagerQt')])
kf6_config = declare_dependency(include_directories: include_directories(get_option('prefix') / 'include' / 'KF6' / 'KConfig', get_option('prefix') / 'include' / 'KF6' / 'KConfigCore', get_option('prefix') / 'include' / 'KF6' / 'KConfigGui'), dependencies: [cpp_compiler.find_library('KF6ConfigCore')])
kf6_kio = declare_dependency(include_directories: include_directories(get_option('prefix') / 'include' / 'KF6' / 'KIO', get_option('prefix') / 'include' / 'KF6' / 'KIOCore', get_option('prefix') / 'include' / 'KF6' / 'KIOGui', get_option('prefix') / 'include' / 'KF6' / 'KIOWidgets', get_option('prefix') / 'include' / 'KF6' / 'KJobWidgets', get_option('prefix') / 'include' / 'KF6' / 'KService', get_option('prefix') / 'include' / 'KF6' / 'KWidgetsAddons', get_option('prefix') / 'include' / 'KF6' / 'KWindowSystem'), dependencies: [cpp_compiler.find_library('KF6KIOCore'), cpp_compiler.find_library('KF6KIOGui'), cpp_compiler.find_library('KF6KIOWidgets')])
kf6_solid = declare_dependency(include_directories: include_directories(get_option('prefix') / 'include' / 'KF6' / 'Solid'), dependencies: [cpp_compiler.find_library('KF6Solid')])
kf6_idle = declare_dependency(include_directories: include_directories(get_option('prefix') / 'include' / 'KF6' / 'KIdleTime'), dependencies: [cpp_compiler.find_library('KF6IdleTime')])
kf6_service = declare_dependency(include_directories: include_directories(get_option('prefix') / 'include' / 'KF6' / 'KService'), dependencies: [cpp_compiler.find_library('KF6Service')])
pipewire = dependency('libpipewire-0.3')
networkmanager = dependency('libnm')
qcoro_include_prefix = get_option('prefix') / 'include' / 'qcoro6'
qcoro_core = declare_dependency(include_directories: include_directories(qcoro_include_prefix, qcoro_include_prefix / 'QCoro', qcoro_include_prefix / 'qcoro'), dependencies: [cpp_compiler.find_library('QCoro6Core')])
qcoro_dbus = declare_dependency(include_directories: include_directories(qcoro_include_prefix, qcoro_include_prefix / 'QCoro', qcoro_include_prefix / 'qcoro'), dependencies: [cpp_compiler.find_library('QCoro6DBus')])
cpp_args = host_machine.cpu_family() == 'x86_64' ? ['-march=x86-64-v3'] : []

control_moc = qt6.preprocess(moc_headers: ['libs/system/src/code/bluetoothcontrolbackend.h'])
control_export = configure_file(
    input: 'meson-content.in',
    output: 'control_export.h',
    configuration: {'CONTENT': ('#pragma once\n#define CONTROL_EXPORT __attribute__((visibility("default")))\n')}
)
control = shared_library('MauiKitSystemControl', files('libs/system/src/code/bluetoothcontrolbackend.cpp', 'libs/system/src/code/mauikit_system_control.cpp') + control_moc + control_export, dependencies: [qt, kf6_bluez], include_directories: include_directories('libs/system/src/code'), cpp_args: cpp_args, version: '4.0.5', soversion: '4', install: true)
install_headers(files('libs/system/src/code/mauikit_system_control.h') + [control_export], subdir: 'Maui/Core/System')

audio_cpp_all = run_command('find', 'libs/audio/src/code', '-type', 'f', '-name', '*.cpp', check: true).stdout().strip().split('\n')
audio_cpp = []
foreach source : audio_cpp_all
    if not source.endswith('qml/plugin.cpp')
        audio_cpp += [source]
    endif
endforeach
audio_moc_headers_all = run_command('find', 'libs/audio/src/code', '-type', 'f', '-name', '*.h', check: true).stdout().strip().split('\n')
audio_moc_headers = []
foreach source : audio_moc_headers_all
    if not source.endswith('qml/plugin.h')
        audio_moc_headers += [source]
    endif
endforeach
audio_moc = qt6.preprocess(moc_headers: audio_moc_headers)
audio_export = configure_file(
    input: 'meson-content.in',
    output: 'audio_export.h',
    configuration: {'CONTENT': ('#pragma once\n#define AUDIO_EXPORT __attribute__((visibility("default")))\n')}
)
audio = shared_library('MauiKitSystemAudio', audio_cpp + audio_moc + audio_export, dependencies: [qt, kf6_i18n, kf6_notifications, pipewire], include_directories: include_directories('libs/audio/src', 'libs/audio/src/code'), cpp_args: cpp_args, version: '4.0.5', soversion: '4', install: true)
audio_plugin_moc = qt6.preprocess(moc_headers: ['libs/audio/src/code/qml/plugin.h'])
audio_plugin = shared_module('MauiKitSystemAudioplugin', ['libs/audio/src/code/qml/plugin.cpp'] + audio_plugin_moc, link_with: audio, dependencies: [qt, kf6_i18n, kf6_notifications, pipewire], include_directories: include_directories('libs/audio/src', 'libs/audio/src/code'), cpp_args: cpp_args, install: true, install_dir: get_option('libdir') / 'qt6/qml/org/mauikit/system/audio')
audio_qml_dir = get_option('libdir') / 'qt6/qml/org/mauikit/system/audio'
install_data(configure_file(
    input: 'meson-content.in',
    output: 'audio_qmldir',
    configuration: {'CONTENT': ('module org.mauikit.system.audio 1.0\\nplugin MauiKitSystemAudioplugin\\n')}
), install_dir: audio_qml_dir, rename: 'qmldir')
install_subdir('libs/audio/src/controls', install_dir: audio_qml_dir, strip_directory: true)
install_headers(files('libs/audio/src/code/card.h'), subdir: 'Maui/Core/Audio', preserve_path: true)

notification_cpp = run_command('find', 'libs/notifications/src/code', '-type', 'f', '-name', '*.cpp', check: true).stdout().strip().split('\n')
notification_lib_cpp = []
foreach source : notification_cpp
    if not source.endswith('notifications_plugin.cpp') and not source.endswith('notifications_adaptor.cpp')
        notification_lib_cpp += [source]
    endif
endforeach
notification_moc_headers_all = run_command('find', 'libs/notifications/src/code', '-type', 'f', '-name', '*.h', check: true).stdout().strip().split('\n')
notification_moc_headers = []
foreach source : notification_moc_headers_all
    if not source.endswith('notifications_plugin.h')
        notification_moc_headers += [source]
    endif
endforeach
notification_moc = qt6.preprocess(moc_extra_arguments: qt_moc_args, moc_headers: notification_moc_headers)
qdbusxml2cpp = find_program('qdbusxml2cpp', dirs: ['/usr/lib/qt6/bin'])
notifications_adaptor = custom_target(
    'notifications-dbus-adaptor',
    input: 'libs/notifications/src/code/org.freedesktop.Notifications.xml',
    output: ['notifications_adaptor.cpp', 'notifications_adaptor.h'],
    command: [qdbusxml2cpp, '-a', 'notifications_adaptor', '@INPUT@']
)
notifications_adaptor_moc = qt6.preprocess(moc_extra_arguments: qt_moc_args, moc_headers: [notifications_adaptor[1]])
notifications_adaptor_source = files('libs/notifications/src/code/notifications_adaptor.cpp')
notification_export = configure_file(
    input: 'meson-content.in',
    output: 'notifications_export.h',
    configuration: {'CONTENT': ('#pragma once\n#define NOTIFICATIONS_EXPORT __attribute__((visibility("default")))\n')}
)
notifications = shared_library('MauiKitSystemNotifications', notification_lib_cpp + [notifications_adaptor_source, notifications_adaptor[1]] + notification_moc + notifications_adaptor_moc + notification_export, dependencies: [qt, kf6_i18n], include_directories: include_directories('libs/notifications/src', 'libs/notifications/src/code'), cpp_args: cpp_args, version: '4.0.5', soversion: '4', install: true)
notifications_plugin_moc = qt6.preprocess(moc_headers: ['libs/notifications/src/code/notifications_plugin.h'])
notifications_plugin = shared_module('MauiKitSystemNotificationsplugin', ['libs/notifications/src/code/notifications_plugin.cpp'] + notifications_plugin_moc, link_with: notifications, dependencies: [qt, kf6_i18n], include_directories: include_directories('libs/notifications/src', 'libs/notifications/src/code'), cpp_args: cpp_args, install: true, install_dir: get_option('libdir') / 'qt6/qml/org/mauikit/system/notifications')
notifications_qml_dir = get_option('libdir') / 'qt6/qml/org/mauikit/system/notifications'
install_data(configure_file(
    input: 'meson-content.in',
    output: 'notifications_qmldir',
    configuration: {'CONTENT': ('module org.mauikit.system.notifications 1.0\\nplugin MauiKitSystemNotificationsplugin\\n')}
), install_dir: notifications_qml_dir, rename: 'qmldir')
install_headers(files('libs/notifications/src/code/notifications.h') + [notification_export], subdir: 'Maui/Core/Notifications')

power_sources = files('libs/power/src/code/powermanagementengine.cpp', 'libs/power/src/code/powermanagementjob.cpp', 'libs/power/src/code/brightnesscontrol.cpp', 'libs/power/src/code/batteryinfo.cpp', 'libs/power/src/code/powerprofile.cpp', 'libs/power/src/code/inhibitioncontrol.cpp')
power_export = configure_file(
    input: 'meson-content.in',
    output: 'power_export.h',
    configuration: {'CONTENT': ('''#pragma once
#define POWER_EXPORT __attribute__((visibility("default")))
''')}
)
power_moc = qt6.preprocess(moc_headers: ['libs/power/src/code/powermanagementengine.h', 'libs/power/src/code/powermanagementjob.h', 'libs/power/src/code/brightnesscontrol.h', 'libs/power/src/code/batteryinfo.h', 'libs/power/src/code/powerprofile.h', 'libs/power/src/code/inhibitioncontrol.h'])
power = shared_library('MauiKitSystemPower', power_sources + power_moc + power_export, dependencies: [qt, kf6_solid, kf6_idle, kf6_core, kf6_i18n, kf6_service], include_directories: include_directories('libs/power/src', 'libs/power/src/code'), cpp_args: cpp_args, version: '4.0.5', soversion: '4', install: true)
power_plugin_moc = qt6.preprocess(moc_headers: ['libs/power/src/code/powerplugin.h'])
power_plugin = shared_module('MauiKitSystemPowerplugin', ['libs/power/src/code/powerplugin.cpp'] + power_plugin_moc, link_with: power, dependencies: [qt, kf6_solid], include_directories: include_directories('libs/power/src', 'libs/power/src/code'), cpp_args: cpp_args, install: true, install_dir: get_option('libdir') / 'qt6/qml/org/mauikit/system/power')
power_qml_dir = get_option('libdir') / 'qt6/qml/org/mauikit/system/power'
install_data(configure_file(
    input: 'meson-content.in',
    output: 'power_qmldir',
    configuration: {'CONTENT': ('''module org.mauikit.system.power
plugin MauiKitSystemPowerplugin
''')}
), install_dir: power_qml_dir, rename: 'qmldir')
install_headers(files('libs/power/src/code/powermanagementengine.h') + [power_export], subdir: 'Maui/Core/Power')

network_cpp = run_command('find', 'libs/network/src/code/libs', '-type', 'f', '-name', '*.cpp', check: true).stdout().strip().split('\n')
network_moc_headers = run_command('find', 'libs/network/src/code/libs', '-type', 'f', '-name', '*.h', check: true).stdout().strip().split('\n')
network_moc = qt6.preprocess(moc_headers: network_moc_headers)
network_export = configure_file(
    input: 'meson-content.in',
    output: 'mauikitsystemnetwork_export.h',
    configuration: {'CONTENT': ('#pragma once\n#define MAUIKITSYSTEMNETWORK_EXPORT __attribute__((visibility("default")))\n')}
)
network_logging_header = configure_file(
    input: 'meson-content.in',
    output: 'mauikitsystemnetwork_logging.h',
    configuration: {'CONTENT': ('''#pragma once
#include <QLoggingCategory>
Q_DECLARE_LOGGING_CATEGORY(MAUIKIT_SYSTEM_NETWORK_LOG)
''')}
)
network_logging_source = configure_file(
    input: 'meson-content.in',
    output: 'mauikitsystemnetwork_logging.cpp',
    configuration: {'CONTENT': ('''#include "mauikitsystemnetwork_logging.h"
Q_LOGGING_CATEGORY(MAUIKIT_SYSTEM_NETWORK_LOG, "org.mauikit.system.network")
''')}
)
network = shared_library('MauiKitSystemNetwork', network_cpp + network_moc + [network_logging_source, network_logging_header, network_export], dependencies: [qt, kf6_network, kf6_modem, kf6_kio, kf6_core, kf6_i18n, kf6_notifications, kf6_config, networkmanager, qcoro_core, qcoro_dbus], include_directories: include_directories('libs/network/src/code/libs', 'libs/network/src/code/libs/models', 'libs/network/src/code/libs/editor'), cpp_args: cpp_args + ['-DMAUIKIT_SYSTEM_NETWORK_USE_KEYCHAIN=1'], version: '4.0.5', soversion: '4', install: true)
network_plugin_source = configure_file(
    input: 'meson-content.in',
    output: 'network_plugin.cpp',
    configuration: {'CONTENT': ('''#include <QQmlExtensionPlugin>
#include <QQmlEngine>
#include <qqml.h>
#include "availabledevices.h"
#include "connectionicon.h"
#include "enabledconnections.h"
#include "enums.h"
#include "handler.h"
#include "networkstatus.h"
#include "wirelessstatus.h"
#include "configurationproxy.h"
#include "models/appletproxymodel.h"
#include "models/creatableconnectionsmodel.h"
#include "models/editorproxymodel.h"
#include "models/kcmidentitymodel.h"
#include "models/mobileproxymodel.h"
#include "models/networkmodel.h"

class MauiKitSystemNetworkPlugin : public QQmlExtensionPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.qt-project.Qt.QQmlExtensionInterface")
public:
    void registerTypes(const char *uri) override
    {
        qmlRegisterType<AvailableDevices>(uri, 1, 0, "AvailableDevices");
        qmlRegisterType<ConnectionIcon>(uri, 1, 0, "ConnectionIcon");
        qmlRegisterType<EnabledConnections>(uri, 1, 0, "EnabledConnections");
        qmlRegisterUncreatableType<Enums>(uri, 1, 0, "Enums", "Enums cannot be created directly");
        qmlRegisterType<Handler>(uri, 1, 0, "Handler");
        qmlRegisterType<NetworkStatus>(uri, 1, 0, "NetworkStatus");
        qmlRegisterType<WirelessStatus>(uri, 1, 0, "WirelessStatus");
        qmlRegisterSingletonType<ConfigurationProxy>(uri, 1, 0, "Configuration", [](QQmlEngine *engine, QJSEngine *) {
            return new ConfigurationProxy(engine);
        });
        qmlRegisterType<AppletProxyModel>(uri, 1, 0, "AppletProxyModel");
        qmlRegisterType<CreatableConnectionsModel>(uri, 1, 0, "CreatableConnectionsModel");
        qmlRegisterType<EditorProxyModel>(uri, 1, 0, "EditorProxyModel");
        qmlRegisterType<KcmIdentityModel>(uri, 1, 0, "KcmIdentityModel");
        qmlRegisterType<MobileProxyModel>(uri, 1, 0, "MobileProxyModel");
        qmlRegisterType<NetworkModel>(uri, 1, 0, "NetworkModel");
    }
};
''')}
)
network_plugin_moc = qt6.preprocess(moc_sources: [network_plugin_source])
network_plugin = shared_module('MauiKitSystemNetworkplugin', [network_plugin_source] + network_plugin_moc, link_with: network, dependencies: [qt, kf6_network, kf6_modem, kf6_kio, kf6_core, kf6_i18n, kf6_notifications, kf6_config, networkmanager, qcoro_core, qcoro_dbus], include_directories: include_directories('libs/network/src/code/libs', 'libs/network/src/code/libs/models', 'libs/network/src/code/libs/editor'), cpp_args: cpp_args, install: true, install_dir: get_option('libdir') / 'qt6/qml/org/mauikit/system/network')
network_qml_dir = get_option('libdir') / 'qt6/qml/org/mauikit/system/network'
install_data(configure_file(
    input: 'meson-content.in',
    output: 'network_qmldir',
    configuration: {'CONTENT': ('module org.mauikit.system.network 1.0\\nplugin MauiKitSystemNetworkplugin\\n')}
), install_dir: network_qml_dir, rename: 'qmldir')
install_headers(files('libs/network/src/code/libs/handler.h') + [network_export], subdir: 'Maui/Core/Network')

bluetooth_cpp = files('libs/bluetooth/src/devicesproxymodel.cpp', 'libs/bluetooth/src/launchapp.cpp', 'libs/bluetooth/src/utils.cpp')
bluetooth_moc = qt6.preprocess(moc_headers: ['libs/bluetooth/src/devicesproxymodel.h', 'libs/bluetooth/src/launchapp.h', 'libs/bluetooth/src/utils.h'])
bluetooth = shared_library('MauiKitSystemBluetooth', bluetooth_cpp + bluetooth_moc, dependencies: [qt, kf6_bluez, kf6_i18n, kf6_notifications], include_directories: include_directories('libs/bluetooth/src'), cpp_args: cpp_args, version: '4.0.5', soversion: '4', install: true)
obex_moc = qt6.preprocess(moc_headers: ['libs/bluetooth/src/obexagent.h'])
obex = executable('maui-bluetooth-obex-agent', files('libs/bluetooth/src/obexagent.cpp', 'libs/bluetooth/src/obexagent_main.cpp') + obex_moc, dependencies: [qt, kf6_bluez, kf6_i18n], cpp_args: cpp_args, install: true)

pkg = import('pkgconfig')
pkg.generate(control, name: 'MauiKit System Control', filebase: 'MauiKitSystemControl', version: meson.project_version(), description: 'MauiKit system control library')
pkg.generate(network, name: 'MauiKit System Network', filebase: 'MauiKitSystemNetwork', version: meson.project_version(), description: 'MauiKit system network library')

system_control_targets = configure_file(
    input: 'meson-content.in',
    output: 'MauiKitSystemControlTargets.cmake',
    configuration: {'CONTENT': (('''get_filename_component(_MAUIKIT_PREFIX "${CMAKE_CURRENT_LIST_DIR}/../../.." ABSOLUTE)
if(NOT TARGET MauiKitSystemControl)
  add_library(MauiKitSystemControl SHARED IMPORTED)
  set_target_properties(MauiKitSystemControl PROPERTIES
    IMPORTED_LOCATION "${_MAUIKIT_PREFIX}/@LIBDIR@/libMauiKitSystemControl.so.4.0.5"
    INTERFACE_INCLUDE_DIRECTORIES "${_MAUIKIT_PREFIX}/include/Maui/Core/System")
endif()
if(NOT TARGET MauiKitSystem::Control)
  add_library(MauiKitSystem::Control ALIAS MauiKitSystemControl)
endif()
''')).replace('@LIBDIR@', (get_option('libdir')))}
)
system_control_config = configure_file(
    input: 'meson-content.in',
    output: 'MauiKitSystemControlConfig.cmake',
    configuration: {'CONTENT': ('''include(CMakeFindDependencyMacro)
find_dependency(Qt6 6.9 COMPONENTS Core Gui DBus)
include("${CMAKE_CURRENT_LIST_DIR}/MauiKitSystemControlTargets.cmake")
set(MauiKitSystemControl_FOUND TRUE)
''')}
)
system_control_version = configure_file(
    input: 'meson-content.in',
    output: 'MauiKitSystemControlConfigVersion.cmake',
    configuration: {'CONTENT': (('''set(PACKAGE_VERSION "@VERSION@")
if(PACKAGE_FIND_VERSION VERSION_LESS_EQUAL PACKAGE_VERSION)
  set(PACKAGE_VERSION_COMPATIBLE TRUE)
endif()
if(PACKAGE_FIND_VERSION VERSION_EQUAL PACKAGE_VERSION)
  set(PACKAGE_VERSION_EXACT TRUE)
endif()
''')).replace('@VERSION@', (meson.project_version()))}
)
install_data([system_control_targets, system_control_config, system_control_version], install_dir: get_option('libdir') / 'cmake' / 'MauiKitSystemControl')

system_network_targets = configure_file(
    input: 'meson-content.in',
    output: 'MauiKitSystemNetworkTargets.cmake',
    configuration: {'CONTENT': (('''get_filename_component(_MAUIKIT_PREFIX "${CMAKE_CURRENT_LIST_DIR}/../../.." ABSOLUTE)
if(NOT TARGET MauiKitSystemNetwork)
  add_library(MauiKitSystemNetwork SHARED IMPORTED)
  set_target_properties(MauiKitSystemNetwork PROPERTIES
    IMPORTED_LOCATION "${_MAUIKIT_PREFIX}/@LIBDIR@/libMauiKitSystemNetwork.so.4.0.5"
    INTERFACE_INCLUDE_DIRECTORIES "${_MAUIKIT_PREFIX}/include/Maui/Core/Network")
endif()
if(NOT TARGET MauiKitSystem::Network)
  add_library(MauiKitSystem::Network ALIAS MauiKitSystemNetwork)
endif()
''')).replace('@LIBDIR@', (get_option('libdir')))}
)
system_network_config = configure_file(
    input: 'meson-content.in',
    output: 'MauiKitSystemNetworkConfig.cmake',
    configuration: {'CONTENT': ('''include(CMakeFindDependencyMacro)
find_dependency(Qt6 6.9 COMPONENTS Core Gui DBus)
include("${CMAKE_CURRENT_LIST_DIR}/MauiKitSystemNetworkTargets.cmake")
set(MauiKitSystemNetwork_FOUND TRUE)
''')}
)
system_network_version = configure_file(
    input: 'meson-content.in',
    output: 'MauiKitSystemNetworkConfigVersion.cmake',
    configuration: {'CONTENT': (('''set(PACKAGE_VERSION "@VERSION@")
if(PACKAGE_FIND_VERSION VERSION_LESS_EQUAL PACKAGE_VERSION)
  set(PACKAGE_VERSION_COMPATIBLE TRUE)
endif()
if(PACKAGE_FIND_VERSION VERSION_EQUAL PACKAGE_VERSION)
  set(PACKAGE_VERSION_EXACT TRUE)
endif()
''')).replace('@VERSION@', (meson.project_version()))}
)
install_data([system_network_targets, system_network_config, system_network_version], install_dir: get_option('libdir') / 'cmake' / 'MauiKitSystemNetwork')
