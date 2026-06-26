add_definitions(-DTRANSLATION_DOMAIN=\"mauikitsystem-bluetooth\")

add_library(MauiKitSystemBluetooth SHARED)
generate_export_header(MauiKitSystemBluetooth)

target_sources(MauiKitSystemBluetooth PRIVATE
    devicesproxymodel.cpp
    devicesproxymodel.h
    launchapp.cpp
    launchapp.h
    utils.cpp
    utils.h
)

target_include_directories(MauiKitSystemBluetooth
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_CURRENT_BINARY_DIR})

target_link_libraries(MauiKitSystemBluetooth
    PRIVATE
        Qt6::Core
        Qt6::DBus
        Qt6::Qml
        KF6::BluezQt
        KF6::I18n
        KF6::Notifications)

ecm_add_qml_module(MauiKitSystemBluetooth URI org.mauikit.system.bluetooth GENERATE_PLUGIN_SOURCE)
ecm_finalize_qml_module(MauiKitSystemBluetooth)

add_executable(maui-bluetooth-obex-agent
    obexagent.cpp
    obexagent.h
    obexagent_main.cpp
)

target_link_libraries(maui-bluetooth-obex-agent
    PRIVATE
        Qt6::Core
        Qt6::DBus
        KF6::BluezQt
        KF6::I18n
)

install(TARGETS MauiKitSystemBluetooth ${KDE_INSTALL_TARGETS_DEFAULT_ARGS})
install(TARGETS maui-bluetooth-obex-agent ${KDE_INSTALL_TARGETS_DEFAULT_ARGS})
