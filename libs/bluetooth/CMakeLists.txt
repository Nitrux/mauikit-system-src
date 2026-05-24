# SPDX-FileCopyrightText: 2021 Camilo Higuita <milo.h@aol.com>
#
# SPDX-License-Identifier: BSD-2-Clause

cmake_minimum_required(VERSION 3.16)

include(ECMDeprecationSettings)

project(MauiKitSystemBluetooth LANGUAGES CXX VERSION ${MAUIKITSYSTEM_VERSION})

find_package(Qt6 ${REQUIRED_QT_VERSION} COMPONENTS Core Qml DBus REQUIRED)

find_package(KF6 ${REQUIRED_KF_VERSION} REQUIRED COMPONENTS
    I18n
    BluezQt
    Notifications)

ecm_set_disabled_deprecation_versions(QT 5.15.2 KF 5.101)

add_subdirectory(src)
