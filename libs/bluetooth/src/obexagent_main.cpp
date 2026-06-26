/*
    SPDX-FileCopyrightText: 2026 Nitrux Latinoamericana S.C.

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "obexagent.h"

#include <KLocalizedString>

#include <QCoreApplication>

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    app.setApplicationName(QStringLiteral("maui-bluetooth-obex-agent"));
    app.setApplicationVersion(QStringLiteral("1.0"));
    KLocalizedString::setApplicationDomain("mauikitsystem-bluetooth");

    ObexAgent agent;
    if (!agent.initialize()) {
        return 1;
    }

    return app.exec();
}
