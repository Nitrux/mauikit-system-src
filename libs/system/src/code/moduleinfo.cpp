// SPDX-FileCopyrightText: 2026 Nitrux Latinoamericana S.C.
// SPDX-License-Identifier: BSD-2-Clause

#include "moduleinfo.h"

#include "mauikitsystemcontrol_version.h"

QString MauiKitSystem::versionString()
{
    return QStringLiteral(MauiKitSystemControl_VERSION_STRING);
}

QString MauiKitSystem::buildVersion()
{
    if (QStringLiteral(GIT_COMMIT_HASH) == QStringLiteral("n/a"))
        return QStringLiteral("n/a");

    return QStringLiteral(GIT_BRANCH) + QStringLiteral("/") + QStringLiteral(GIT_COMMIT_HASH);
}

KAboutComponent MauiKitSystem::aboutData()
{
    return KAboutComponent(QStringLiteral("MauiKit System"),
                           QStringLiteral("System integration libraries."),
                           QStringLiteral(MauiKitSystemControl_VERSION_STRING),
                           QStringLiteral("https://mauikit.org"));
}
