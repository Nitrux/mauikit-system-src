/*
    SPDX-FileCopyrightText: 2014-2015 Harald Sitter <sitter@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "plugin.h"

#include <QQmlEngine>

#include "client.h"
#include "context.h"
#include "modulemanager.h"
#include "pipewirecontext.h"
#include "port.h"
#include "profile.h"
#include "audioengine.h"
#include "sink.h"
#include "source.h"
#include "volumemonitor.h"

#include "speakertest.h"
#include "volumefeedback.h"
#include "model/sortfiltermodel.h"

static QJSValue audio_singleton(QQmlEngine *engine, QJSEngine *scriptEngine)
{
    Q_UNUSED(engine)

    QJSValue object = scriptEngine->newObject();
    object.setProperty(QStringLiteral("NormalVolume"), (double)QPipeWireAudio::Context::NormalVolume);
    object.setProperty(QStringLiteral("MinimalVolume"), (double)QPipeWireAudio::Context::MinimalVolume);
    object.setProperty(QStringLiteral("MaximalVolume"), (double)QPipeWireAudio::Context::MaximalVolume);
    return object;
}

QUrl AudioPlugin::componentUrl(const QString &fileName) const
{
    return QUrl(resolveFileUrl(fileName));
}

void AudioPlugin::registerTypes(const char *uri)
{
    QPipeWireAudio::Context::setApplicationId(QString::fromLocal8Bit(uri));

    qmlRegisterType<SortFilterModel>(uri, 1, 0, "SortFilterModel");

    qmlRegisterType<QPipeWireAudio::CardModel>(uri, 1, 0, "CardModel");
    qmlRegisterType<QPipeWireAudio::SinkModel>(uri, 1, 0, "SinkModel");
    qmlRegisterType<QPipeWireAudio::SinkInputModel>(uri, 1, 0, "SinkInputModel");
    qmlRegisterType<QPipeWireAudio::SourceModel>(uri, 1, 0, "SourceModel");
    qmlRegisterType<QPipeWireAudio::ModuleManager>(uri, 1, 0, "ModuleManager");
    qmlRegisterType<QPipeWireAudio::SourceOutputModel>(uri, 1, 0, "SourceOutputModel");
    qmlRegisterType<QPipeWireAudio::StreamRestoreModel>(uri, 1, 0, "StreamRestoreModel");
    qmlRegisterType<QPipeWireAudio::ModuleModel>(uri, 1, 0, "ModuleModel");
    qmlRegisterType<QPipeWireAudio::VolumeMonitor>(uri, 1, 0, "VolumeMonitor");
    qmlRegisterUncreatableType<QPipeWireAudio::AudioObject>(uri, 1, 0, "AudioObject", QString());
    qmlRegisterUncreatableType<QPipeWireAudio::Profile>(uri, 1, 0, "Profile", QString());
    qmlRegisterUncreatableType<QPipeWireAudio::Port>(uri, 1, 0, "Port", QString());
    qmlRegisterType<VolumeFeedback>(uri, 1, 0, "VolumeFeedback");
    qmlRegisterType<SpeakerTest>(uri, 1, 0, "SpeakerTest");
    qmlRegisterSingletonType(uri, 1, 0, "Audio", audio_singleton);
    qmlRegisterType<PipeWireContext>(uri, 1, 0, "PipeWireContext");
    qmlRegisterAnonymousType<QPipeWireAudio::Client>(uri, 1);
    qmlRegisterAnonymousType<QPipeWireAudio::Sink>(uri, 1);
    qmlRegisterAnonymousType<QPipeWireAudio::Source>(uri, 1);
    qmlRegisterAnonymousType<QPipeWireAudio::VolumeObject>(uri, 1);

    qmlRegisterType(componentUrl(QStringLiteral("AudioObjectFilterModel.qml")), uri, 1, 0, "AudioObjectFilterModel");

}
