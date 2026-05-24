add_definitions(-DTRANSLATION_DOMAIN=\"mauikitsystem-network\")




add_library(MauiKitSystemNetwork SHARED)
generate_export_header(MauiKitSystemNetwork)

ecm_qt_declare_logging_category(MauiKitSystemNetwork HEADER mauikitsystemnetwork_logging.h IDENTIFIER MAUIKIT_SYSTEM_NETWORK_LOG CATEGORY_NAME org.mauikit.system.network DESCRIPTION "MauiKitSystem Network" EXPORT MAUIKITSYSTEMNETWORK)
target_sources(MauiKitSystemNetwork PRIVATE
    models/appletproxymodel.cpp
    models/creatableconnectionsmodel.cpp
    models/editorproxymodel.cpp
    models/kcmidentitymodel.cpp
    models/mobileproxymodel.cpp
    models/networkitemslist.cpp
    models/networkmodel.cpp
    models/networkmodelitem.cpp
    configurationproxy.cpp
    handler.cpp
    secretsbackend.cpp
    models/appletproxymodel.h
    models/creatableconnectionsmodel.h
    models/editorproxymodel.h
    models/kcmidentitymodel.h
    models/mobileproxymodel.h
    models/networkitemslist.h
    models/networkmodel.h
    models/networkmodelitem.h
    configurationproxy.h
    handler.h
    secretsbackend.h
    availabledevices.cpp
    connectionicon.cpp
    enabledconnections.cpp
    enums.cpp
    networkstatus.cpp
    wirelessstatus.cpp
    availabledevices.h
    connectionicon.h
    enabledconnections.h
    enums.h
    networkstatus.h
    wirelessstatus.h
    editor/configuration.cpp
    editor/configuration.h
    editor/uiutils.cpp
    editor/uiutils.h
)

target_include_directories(MauiKitSystemNetwork
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/editor
        ${CMAKE_CURRENT_SOURCE_DIR}/models
        ${CMAKE_CURRENT_BINARY_DIR})

target_link_libraries(MauiKitSystemNetwork
PUBLIC
    KF6::NetworkManagerQt
    PkgConfig::NETWORKMANAGER
    QCoro::Core
PRIVATE
    Qt6::Qml
    KF6::I18n
    KF6::Notifications
    KF6::KIOGui
    KF6::CoreAddons
    KF6::ConfigCore
    QCoro::DBus
)

target_link_libraries(MauiKitSystemNetwork PUBLIC KF6::ModemManagerQt)

if(MAUIKIT_SYSTEM_NETWORK_QTKEYCHAIN_TARGET)
    target_link_libraries(MauiKitSystemNetwork PRIVATE ${MAUIKIT_SYSTEM_NETWORK_QTKEYCHAIN_TARGET})
endif()

ecm_add_qml_module(MauiKitSystemNetwork URI org.mauikit.system.network GENERATE_PLUGIN_SOURCE)
ecm_finalize_qml_module(MauiKitSystemNetwork)

install(TARGETS MauiKitSystemNetwork ${KDE_INSTALL_TARGETS_DEFAULT_ARGS})
