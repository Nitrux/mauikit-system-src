# SPDX-FileCopyrightText: 2026 Nitrux Latinoamericana S.C.
#
# SPDX-License-Identifier: BSD-2-Clause

cmake_minimum_required(VERSION 3.16)

project(MauiKitSystemControl LANGUAGES CXX VERSION ${MAUIKITSYSTEM_VERSION})

find_package(Qt6 ${REQUIRED_QT_VERSION} COMPONENTS Core DBus REQUIRED)
find_package(KF6 ${REQUIRED_KF_VERSION} REQUIRED COMPONENTS BluezQt)

include(GenerateExportHeader)

set(CMAKECONFIG_INSTALL_DIR "${KDE_INSTALL_CMAKEPACKAGEDIR}/MauiKitSystemControl")

configure_package_config_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/MauiKitSystemControlConfig.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/MauiKitSystemControlConfig.cmake"
    INSTALL_DESTINATION ${CMAKECONFIG_INSTALL_DIR}
    PATH_VARS CMAKE_INSTALL_PREFIX)

install(FILES
    "${CMAKE_CURRENT_BINARY_DIR}/MauiKitSystemControlConfig.cmake"
    "${CMAKE_CURRENT_BINARY_DIR}/MauiKitSystemControlConfigVersion.cmake"
    DESTINATION "${CMAKECONFIG_INSTALL_DIR}"
    COMPONENT Devel)

ecm_setup_version(${PROJECT_VERSION}
    VARIABLE_PREFIX MauiKitSystemControl
    VERSION_HEADER "${CMAKE_CURRENT_BINARY_DIR}/mauikitsystemcontrol_version.h"
    PACKAGE_VERSION_FILE "${CMAKE_CURRENT_BINARY_DIR}/MauiKitSystemControlConfigVersion.cmake"
    SOVERSION ${PROJECT_VERSION_MAJOR})

add_library(${PROJECT_NAME} SHARED)
add_library(MauiKitSystem::Control ALIAS ${PROJECT_NAME})

target_sources(${PROJECT_NAME}
    PRIVATE
    src/code/bluetoothcontrolbackend.cpp
    src/code/mauikit_system_control.cpp)

target_include_directories(${PROJECT_NAME}
    PUBLIC
    "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src/code>"
    "$<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}>"
    "$<INSTALL_INTERFACE:${KDE_INSTALL_INCLUDEDIR}/Maui/Core/System>")

target_link_libraries(${PROJECT_NAME}
    PRIVATE
    Qt6::Core
    Qt6::DBus
    KF6::BluezQt)

generate_export_header(${PROJECT_NAME} BASE_NAME Control)
set_target_properties(${PROJECT_NAME} PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION ${PROJECT_VERSION_MAJOR}
    EXPORT_NAME Control)

install(TARGETS ${PROJECT_NAME} EXPORT MauiKitSystemControlTargets ${KDE_INSTALL_TARGETS_DEFAULT_ARGS})
install(EXPORT MauiKitSystemControlTargets
    DESTINATION "${CMAKECONFIG_INSTALL_DIR}"
    FILE MauiKitSystemControlTargets.cmake
    NAMESPACE MauiKitSystem::)
install(FILES
    src/code/mauikit_system_control.h
    "${CMAKE_CURRENT_BINARY_DIR}/control_export.h"
    "${CMAKE_CURRENT_BINARY_DIR}/mauikitsystemcontrol_version.h"
    DESTINATION ${KDE_INSTALL_INCLUDEDIR}/Maui/Core/System
    COMPONENT Devel)
