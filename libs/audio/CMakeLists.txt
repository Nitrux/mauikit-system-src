# SPDX-FileCopyrightText: 2021 Camilo Higuita <milo.h@aol.com>
#
# SPDX-License-Identifier: BSD-2-Clause

cmake_minimum_required(VERSION 3.16)

include(FindPkgConfig)
include(ECMDeprecationSettings)

set(AUTOMOC_MOC_OPTIONS -Muri=org.mauikit.system.audio)

project(MauiKitSystemAudio LANGUAGES CXX VERSION ${MAUIKITSYSTEM_VERSION})

set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake/ ${CMAKE_MODULE_PATH} ${ECM_MODULE_PATH})

set(CMAKECONFIG_INSTALL_DIR "${KDE_INSTALL_CMAKEPACKAGEDIR}/MauiKitSystemAudio")

configure_package_config_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/MauiKitSystemAudioConfig.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/MauiKitSystemAudioConfig.cmake"
    INSTALL_DESTINATION ${CMAKECONFIG_INSTALL_DIR}
    PATH_VARS CMAKE_INSTALL_PREFIX)

install(FILES
    "${CMAKE_CURRENT_BINARY_DIR}/MauiKitSystemAudioConfigVersion.cmake"
    "${CMAKE_CURRENT_BINARY_DIR}/MauiKitSystemAudioConfig.cmake"
    DESTINATION "${CMAKECONFIG_INSTALL_DIR}"
    COMPONENT Devel)

install(EXPORT MauiKitSystemAudioTargets
    DESTINATION "${CMAKECONFIG_INSTALL_DIR}"
    FILE MauiKitSystemAudioTargets.cmake
    NAMESPACE MauiKitSystem::)

ecm_setup_version(${PROJECT_VERSION}
    VARIABLE_PREFIX MauiKitSystemAudio
    VERSION_HEADER "${CMAKE_CURRENT_BINARY_DIR}/mauikitsystemaudio_version.h"
    PACKAGE_VERSION_FILE "${CMAKE_CURRENT_BINARY_DIR}/MauiKitSystemAudioConfigVersion.cmake"
    SOVERSION ${PROJECT_VERSION_MAJOR})

install(FILES "${CMAKE_CURRENT_BINARY_DIR}/mauikitsystemaudio_version.h"
    DESTINATION ${KDE_INSTALL_INCLUDEDIR}/Maui/Core/Audio
    COMPONENT Devel)

find_package(Qt6 ${REQUIRED_QT_VERSION} COMPONENTS Core Quick Qml DBus Gui Widgets REQUIRED)
find_package(KF6 ${REQUIRED_KF_VERSION} REQUIRED COMPONENTS I18n Notifications)

pkg_check_modules(LIBPIPEWIRE libpipewire-0.3 REQUIRED IMPORTED_TARGET)

find_package(SoundThemeFreedesktop)
set_package_properties(SoundThemeFreedesktop PROPERTIES
    DESCRIPTION "The standard freedesktop sound theme"
    PURPOSE "Required for volume feedback sounds"
    URL "https://www.freedesktop.org/wiki/Specifications/sound-theme-spec/"
    TYPE RUNTIME)

ecm_set_disabled_deprecation_versions(QT 5.15.2
    KF 5.101)

add_subdirectory(src)

feature_summary(WHAT ALL INCLUDE_QUIET_PACKAGES FATAL_ON_MISSING_REQUIRED_PACKAGES)
