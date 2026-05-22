# SPDX-FileCopyrightText: 2021 Camilo Higuita <milo.h@aol.com>
#
# SPDX-License-Identifier: BSD-2-Clause

cmake_minimum_required(VERSION 3.16)

set(AUTOMOC_MOC_OPTIONS -Muri=org.mauikit.system.notifications)

project(MauiKitSystemNotifications LANGUAGES CXX VERSION ${MAUIKITSYSTEM_VERSION})

set(CMAKECONFIG_INSTALL_DIR "${KDE_INSTALL_CMAKEPACKAGEDIR}/MauiKitSystemNotifications")

configure_package_config_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/MauiKitSystemNotificationsConfig.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/MauiKitSystemNotificationsConfig.cmake"
    INSTALL_DESTINATION ${CMAKECONFIG_INSTALL_DIR}
    PATH_VARS CMAKE_INSTALL_PREFIX)

install(FILES
    "${CMAKE_CURRENT_BINARY_DIR}/MauiKitSystemNotificationsConfigVersion.cmake"
    "${CMAKE_CURRENT_BINARY_DIR}/MauiKitSystemNotificationsConfig.cmake"
    DESTINATION "${CMAKECONFIG_INSTALL_DIR}"
    COMPONENT Devel)

install(EXPORT MauiKitSystemNotificationsTargets
    DESTINATION "${CMAKECONFIG_INSTALL_DIR}"
    FILE MauiKitSystemNotificationsTargets.cmake
    NAMESPACE MauiKitSystem::)

ecm_setup_version(${PROJECT_VERSION}
    VARIABLE_PREFIX MauiKitSystemNotifications
    VERSION_HEADER "${CMAKE_CURRENT_BINARY_DIR}/mauikitsystemnotifications_version.h"
    PACKAGE_VERSION_FILE "${CMAKE_CURRENT_BINARY_DIR}/MauiKitSystemNotificationsConfigVersion.cmake"
    SOVERSION ${PROJECT_VERSION_MAJOR})

install(FILES "${CMAKE_CURRENT_BINARY_DIR}/mauikitsystemnotifications_version.h"
    DESTINATION ${KDE_INSTALL_INCLUDEDIR}/Maui/Core/Notifications
    COMPONENT Devel)

find_package(Qt6 ${REQUIRED_QT_VERSION} COMPONENTS Core Quick DBus Gui REQUIRED)
find_package(KF6 ${REQUIRED_KF_VERSION} REQUIRED COMPONENTS I18n)

add_subdirectory(src)

feature_summary(WHAT ALL INCLUDE_QUIET_PACKAGES FATAL_ON_MISSING_REQUIRED_PACKAGES)
