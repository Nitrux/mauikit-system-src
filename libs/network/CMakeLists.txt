# SPDX-FileCopyrightText: 2021 Camilo Higuita <milo.h@aol.com>
#
# SPDX-License-Identifier: BSD-2-Clause

cmake_minimum_required(VERSION 3.16)

include(FindPkgConfig)
include(ECMDeprecationSettings)
include(ECMQtDeclareLoggingCategory)

project(MauiKitSystemNetwork LANGUAGES CXX VERSION ${MAUIKITSYSTEM_VERSION})

set(CMAKECONFIG_INSTALL_DIR "${KDE_INSTALL_CMAKEPACKAGEDIR}/MauiKitSystemNetwork")

configure_package_config_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/MauiKitSystemNetworkConfig.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/MauiKitSystemNetworkConfig.cmake"
    INSTALL_DESTINATION ${CMAKECONFIG_INSTALL_DIR}
    PATH_VARS CMAKE_INSTALL_PREFIX)

install(FILES
    "${CMAKE_CURRENT_BINARY_DIR}/MauiKitSystemNetworkConfig.cmake"
    "${CMAKE_CURRENT_BINARY_DIR}/MauiKitSystemNetworkConfigVersion.cmake"
    DESTINATION "${CMAKECONFIG_INSTALL_DIR}"
    COMPONENT Devel)

install(EXPORT MauiKitSystemNetworkTargets
    DESTINATION "${CMAKECONFIG_INSTALL_DIR}"
    FILE MauiKitSystemNetworkTargets.cmake
    NAMESPACE MauiKitSystem::)

ecm_setup_version(${PROJECT_VERSION}
    VARIABLE_PREFIX MauiKitSystemNetwork
    VERSION_HEADER "${CMAKE_CURRENT_BINARY_DIR}/mauikitsystemnetwork_version.h"
    PACKAGE_VERSION_FILE "${CMAKE_CURRENT_BINARY_DIR}/MauiKitSystemNetworkConfigVersion.cmake"
    SOVERSION ${PROJECT_VERSION_MAJOR})

install(FILES "${CMAKE_CURRENT_BINARY_DIR}/mauikitsystemnetwork_version.h"
    DESTINATION ${KDE_INSTALL_INCLUDEDIR}/Maui/Core/Network
    COMPONENT Devel)


set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake/ ${CMAKE_MODULE_PATH} ${ECM_MODULE_PATH})

option(MAUIKIT_SYSTEM_NETWORK_USE_KEYCHAIN "Prefer keychain-backed secret storage when a Secret Service is available" ON)

find_package(Qt6 ${REQUIRED_QT_VERSION} COMPONENTS Core Qml Quick DBus Gui REQUIRED)
find_package(Qt6Keychain QUIET)

find_package(KF6 ${REQUIRED_KF_VERSION} REQUIRED COMPONENTS
    I18n
    Notifications
    KIO
    NetworkManagerQt
    ModemManagerQt
    CoreAddons
    Config)

find_package(QCoro6 REQUIRED COMPONENTS Core DBus)

pkg_check_modules(NETWORKMANAGER libnm REQUIRED IMPORTED_TARGET)

ecm_set_disabled_deprecation_versions(QT 5.15.2 KF 5.101)

if(MAUIKIT_SYSTEM_NETWORK_USE_KEYCHAIN)
    add_compile_definitions(MAUIKIT_SYSTEM_NETWORK_USE_KEYCHAIN=1)
else()
    add_compile_definitions(MAUIKIT_SYSTEM_NETWORK_USE_KEYCHAIN=0)
endif()

if(TARGET Qt6Keychain::Qt6Keychain)
    set(MAUIKIT_SYSTEM_NETWORK_QTKEYCHAIN_TARGET Qt6Keychain::Qt6Keychain)
    add_compile_definitions(MAUIKIT_SYSTEM_NETWORK_HAS_QTKEYCHAIN=1)
else()
    set(MAUIKIT_SYSTEM_NETWORK_QTKEYCHAIN_TARGET "")
    add_compile_definitions(MAUIKIT_SYSTEM_NETWORK_HAS_QTKEYCHAIN=0)
endif()

add_subdirectory(src)
