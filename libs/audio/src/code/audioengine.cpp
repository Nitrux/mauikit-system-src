#include "audioengine.h"

#include "audioobject.h"
#include "card.h"
#include "context.h"
#include "debug.h"
#include "sink.h"
#include "sinkinput.h"
#include "source.h"
#include "sourceoutput.h"
#include "streamrestore.h"

#include <QMetaEnum>
#include <QMetaProperty>

namespace QPipeWireAudio
{
AbstractModel::AbstractModel(QObject *parent)
    : QAbstractListModel(parent)
{
    Context::instance()->ref();
}

AbstractModel::~AbstractModel()
{
    qDeleteAll(m_items);
    Context::instance()->unref();
}

QHash<int, QByteArray> AbstractModel::roleNames() const
{
    return m_roles;
}

int AbstractModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid()) {
        return 0;
    }
    return m_items.count();
}

QVariant AbstractModel::data(const QModelIndex &index, int role) const
{
    if (!hasIndex(index.row(), index.column())) {
        return {};
    }

    AudioObject *item = m_items.at(index.row());
    if (!item) {
        return {};
    }

    if (role == AudioObjectRole) {
        return QVariant::fromValue(static_cast<QObject *>(item));
    }

    if (role == Qt::DisplayRole) {
        return item->properties().value(QStringLiteral("name")).toString();
    }

    const int propertyIndex = m_objectProperties.value(role, -1);
    if (propertyIndex < 0) {
        return {};
    }

    return item->metaObject()->property(propertyIndex).read(item);
}

bool AbstractModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (!hasIndex(index.row(), index.column())) {
        return false;
    }

    const int propertyIndex = m_objectProperties.value(role, -1);
    if (propertyIndex < 0) {
        return false;
    }

    AudioObject *item = m_items.at(index.row());
    if (!item) {
        return false;
    }

    const QMetaProperty property = item->metaObject()->property(propertyIndex);
    return property.write(item, value);
}

int AbstractModel::role(const QByteArray &roleName) const
{
    return m_roles.key(roleName, -1);
}

Context *AbstractModel::context() const
{
    return Context::instance();
}

void AbstractModel::initRoleNames(const QMetaObject &qobjectMetaObject)
{
    m_roles.clear();
    m_objectProperties.clear();

    m_roles[AudioObjectRole] = QByteArrayLiteral("AudioObject");

    QMetaEnum enumerator;
    for (int i = 0; i < metaObject()->enumeratorCount(); ++i) {
        if (metaObject()->enumerator(i).name() == QByteArrayLiteral("ItemRole")) {
            enumerator = metaObject()->enumerator(i);
            break;
        }
    }

    for (int i = 0; i < enumerator.keyCount(); ++i) {
        QByteArray key(enumerator.key(i));
        if (key.endsWith(QByteArrayLiteral("Role"))) {
            key.chop(4);
        }
        m_roles[enumerator.value(i)] = key;
    }

    int maxEnumValue = -1;
    for (auto it = m_roles.constBegin(); it != m_roles.constEnd(); ++it) {
        maxEnumValue = qMax(maxEnumValue, it.key());
    }

    for (int i = 0; i < qobjectMetaObject.propertyCount(); ++i) {
        const QMetaProperty property = qobjectMetaObject.property(i);
        QString roleName = QString::fromLatin1(property.name());
        if (roleName.isEmpty()) {
            continue;
        }
        roleName[0] = roleName[0].toUpper();
        const int roleId = ++maxEnumValue;
        m_roles[roleId] = roleName.toLatin1();
        m_objectProperties[roleId] = i;
    }
}

void AbstractModel::replaceItems(const QList<AudioObject *> &items)
{
    beginResetModel();
    qDeleteAll(m_items);
    m_items = items;
    endResetModel();
    Q_EMIT countChanged();
}

QList<AudioObject *> AbstractModel::items() const
{
    return m_items;
}

CardModel::CardModel(QObject *parent)
    : AbstractModel(parent)
{
    initRoleNames(Card::staticMetaObject);
    replaceItems({});
}

static void applyCommonNodeSnapshot(VolumeObject *obj, const NodeSnapshot &snap)
{
    obj->setIndex(snap.index);
    obj->setProperties({
        {QStringLiteral("name"), snap.name},
        {QStringLiteral("description"), snap.description},
    });
    obj->VolumeObject::setVolume(snap.volume);
    obj->VolumeObject::setMuted(snap.muted);
    obj->setHasVolume(snap.hasVolume);
    obj->setVolumeWritable(snap.volumeWritable);
    obj->setChannels(snap.channels.isEmpty() ? QStringList{QStringLiteral("FL"), QStringLiteral("FR")} : snap.channels);
    obj->setRawChannels(obj->channels());
    QVector<qint64> channelVolumes;
    channelVolumes << snap.volume << snap.volume;
    obj->setChannelVolumes(channelVolumes);
}

SinkModel::SinkModel(QObject *parent)
    : AbstractModel(parent)
{
    initRoleNames(Sink::staticMetaObject);

    connect(context(), &Context::updated, this, &SinkModel::rebuild);
    context()->refresh();
    rebuild();
}

void SinkModel::rebuild()
{
    QList<AudioObject *> newItems;

    for (const NodeSnapshot &snap : context()->sinks()) {
        auto *sink = new Sink(this);
        applyCommonNodeSnapshot(sink, snap);
        sink->setName(snap.name);
        sink->setDescription(snap.description);
        sink->setState(Device::RunningState);
        sink->Device::setDefault(snap.isDefault);
        sink->setVirtualDevice(snap.virtualDevice);
        sink->setPorts({snap.description});
        sink->setActivePortIndex(0);
        newItems << sink;
    }

    replaceItems(newItems);

    Sink *newDefault = defaultSink();
    if (m_preferredSink != newDefault) {
        m_preferredSink = newDefault;
        Q_EMIT preferredSinkChanged();
    }

    Q_EMIT defaultSinkChanged();
}

Sink *SinkModel::defaultSink() const
{
    for (AudioObject *obj : items()) {
        auto *sink = qobject_cast<Sink *>(obj);
        if (sink && sink->isDefault()) {
            return sink;
        }
    }

    return items().isEmpty() ? nullptr : qobject_cast<Sink *>(items().first());
}

Sink *SinkModel::preferredSink() const
{
    return m_preferredSink;
}

QVariant SinkModel::data(const QModelIndex &index, int role) const
{
    if (role == SortByDefaultRole) {
        const QString idx = AbstractModel::data(index, AbstractModel::role(QByteArrayLiteral("Index"))).toString();
        const QString def = AbstractModel::data(index, AbstractModel::role(QByteArrayLiteral("Default"))).toString();
        return def + idx;
    }
    return AbstractModel::data(index, role);
}

SourceModel::SourceModel(QObject *parent)
    : AbstractModel(parent)
{
    initRoleNames(Source::staticMetaObject);

    connect(context(), &Context::updated, this, &SourceModel::rebuild);
    context()->refresh();
    rebuild();
}

void SourceModel::rebuild()
{
    QList<AudioObject *> newItems;

    for (const NodeSnapshot &snap : context()->sources()) {
        auto *source = new Source(this);
        applyCommonNodeSnapshot(source, snap);
        source->setName(snap.name);
        source->setDescription(snap.description);
        source->setState(Device::RunningState);
        source->Device::setDefault(snap.isDefault);
        source->setVirtualDevice(snap.virtualDevice);
        source->setPorts({snap.description});
        source->setActivePortIndex(0);
        newItems << source;
    }

    replaceItems(newItems);
    Q_EMIT defaultSourceChanged();
}

Source *SourceModel::defaultSource() const
{
    for (AudioObject *obj : items()) {
        auto *source = qobject_cast<Source *>(obj);
        if (source && source->isDefault()) {
            return source;
        }
    }

    return items().isEmpty() ? nullptr : qobject_cast<Source *>(items().first());
}

QVariant SourceModel::data(const QModelIndex &index, int role) const
{
    if (role == SortByDefaultRole) {
        const QString idx = AbstractModel::data(index, AbstractModel::role(QByteArrayLiteral("Index"))).toString();
        const QString def = AbstractModel::data(index, AbstractModel::role(QByteArrayLiteral("Default"))).toString();
        return def + idx;
    }
    return AbstractModel::data(index, role);
}

SinkInputModel::SinkInputModel(QObject *parent)
    : AbstractModel(parent)
{
    initRoleNames(SinkInput::staticMetaObject);
    connect(context(), &Context::updated, this, &SinkInputModel::rebuild);
    context()->refresh();
    rebuild();
}

void SinkInputModel::rebuild()
{
    QList<AudioObject *> newItems;

    for (const NodeSnapshot &snap : context()->sinkInputs()) {
        auto *stream = new SinkInput(this);
        applyCommonNodeSnapshot(stream, snap);
        stream->setName(snap.description);
        stream->setDeviceIndex(snap.deviceIndex);
        stream->setVirtualStream(false);
        newItems << stream;
    }

    replaceItems(newItems);
}

SourceOutputModel::SourceOutputModel(QObject *parent)
    : AbstractModel(parent)
{
    initRoleNames(SourceOutput::staticMetaObject);
    connect(context(), &Context::updated, this, &SourceOutputModel::rebuild);
    context()->refresh();
    rebuild();
}

void SourceOutputModel::rebuild()
{
    QList<AudioObject *> newItems;

    for (const NodeSnapshot &snap : context()->sourceOutputs()) {
        auto *stream = new SourceOutput(this);
        applyCommonNodeSnapshot(stream, snap);
        stream->setName(snap.description);
        stream->setDeviceIndex(snap.deviceIndex);
        stream->setVirtualStream(false);
        newItems << stream;
    }

    replaceItems(newItems);
}

StreamRestoreModel::StreamRestoreModel(QObject *parent)
    : AbstractModel(parent)
{
    initRoleNames(StreamRestore::staticMetaObject);
    replaceItems({});
}

ModuleModel::ModuleModel(QObject *parent)
    : AbstractModel(parent)
{
    initRoleNames(AudioObject::staticMetaObject);
    replaceItems({});
}

} // namespace QPipeWireAudio
