# Copyright 2018-2020 Camilo Higuita <milo.h@aol.com>
# Copyright 2018-2020 Nitrux Latinoamericana S.C.
#
# SPDX-License-Identifier: GPL-3.0-or-later

cmake_minimum_required(VERSION 3.16)

project(MauiKitSystem VERSION ${MAUIKITSYSTEM_VERSION})

set(REQUIRED_QT_VERSION 6.9)
set(REQUIRED_KF_VERSION 6.13.0)

set(KDE_COMPILERSETTINGS_LEVEL "5.82")

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(CheckCXXCompilerFlag)
if(NOT CMAKE_CROSSCOMPILING)
    string(TOLOWER "${CMAKE_HOST_SYSTEM_PROCESSOR}" _host_processor)
    if(_host_processor MATCHES "^(x86_64|amd64)$")
        check_cxx_compiler_flag("-march=x86-64-v3" HAS_X86_64_V3)
        if(HAS_X86_64_V3)
            add_compile_options("$<$<COMPILE_LANGUAGE:CXX>:-march=x86-64-v3>")
        endif()
    endif()
endif()

find_package(ECM ${REQUIRED_KF_VERSION} NO_MODULE)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${ECM_MODULE_PATH})

include(KDEInstallDirs)
include(KDECMakeSettings)

include(GenerateExportHeader)
include(FeatureSummary)
include(CMakePackageConfigHelpers)

include(ECMSetupVersion)
include(ECMGenerateQmlTypes)
include(ECMFindQmlModule)
include(ECMQmlModule)

include(KDEPackageAppTemplates)
include(KDECompilerSettings)

##CMAKE PARTS
set(CMAKECONFIG_INSTALL_DIR "${KDE_INSTALL_CMAKEPACKAGEDIR}/MauiKitSystem")

configure_package_config_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/MauiKitSystemConfig.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/MauiKitSystemConfig.cmake"
    INSTALL_DESTINATION  ${CMAKECONFIG_INSTALL_DIR}
    PATH_VARS CMAKE_INSTALL_PREFIX)

install(FILES
    "${CMAKE_CURRENT_BINARY_DIR}/MauiKitSystemConfig.cmake"
    "${CMAKE_CURRENT_BINARY_DIR}/MauiKitSystemConfigVersion.cmake"
    DESTINATION "${CMAKECONFIG_INSTALL_DIR}"
    COMPONENT Devel)

ecm_setup_version(${PROJECT_VERSION}
    VARIABLE_PREFIX MauiKitSystem
    VERSION_HEADER "${CMAKE_CURRENT_BINARY_DIR}/mauikitsystem_version.h"
    PACKAGE_VERSION_FILE "${CMAKE_CURRENT_BINARY_DIR}/MauiKitSystemConfigVersion.cmake"
    SOVERSION ${PROJECT_VERSION_MAJOR})

add_subdirectory(audio)
add_subdirectory(notifications)
add_subdirectory(power)

feature_summary(WHAT ALL FATAL_ON_MISSING_REQUIRED_PACKAGES)
