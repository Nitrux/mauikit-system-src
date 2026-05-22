/*
    SPDX-FileCopyrightText: 2014-2015 Harald Sitter <sitter@kde.org>
    SPDX-FileCopyrightText: 2021 Nicolas Fella

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "speakertest.h"

#include <KNotification>

QPipeWireAudio::Sink *SpeakerTest::sink() const
{
    return m_sink;
}

void SpeakerTest::setSink(QPipeWireAudio::Sink *sink)
{
    if (m_sink != sink) {
        m_sink = sink;
        Q_EMIT sinkChanged();
    }
}

void SpeakerTest::testChannel(const QString &name)
{
    Q_UNUSED(name)
    KNotification::event(QStringLiteral("SpeakerTest"),
                         QString(),
                         QString(),
                         QStringLiteral("audio-speakers"));
}
