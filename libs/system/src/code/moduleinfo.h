// SPDX-FileCopyrightText: 2026 Nitrux Latinoamericana S.C.
// SPDX-License-Identifier: BSD-2-Clause

#pragma once

#include <KAboutData>
#include <QString>

#include "control_export.h"

namespace MauiKitSystem
{
CONTROL_EXPORT QString versionString();
CONTROL_EXPORT QString buildVersion();
CONTROL_EXPORT KAboutComponent aboutData();
}
