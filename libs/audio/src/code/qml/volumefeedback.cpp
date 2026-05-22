/*
    SPDX-FileCopyrightText: 2008 Helio Chissini de Castro <helio@kde.org>
    SPDX-FileCopyrightText: 2016 David Rosca <nowrep@gmail.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "volumefeedback.h"

#include <KNotification>

VolumeFeedback::VolumeFeedback(QObject *parent)
    : QObject(parent)
{
}

VolumeFeedback::~VolumeFeedback()
{
}

bool VolumeFeedback::isValid() const
{
    return true;
}

void VolumeFeedback::play(quint32 sinkIndex)
{
    Q_UNUSED(sinkIndex)
    KNotification::event(QStringLiteral("VolumeChanged"),
                         QString(),
                         QString(),
                         QStringLiteral("audio-volume-high"));
}
