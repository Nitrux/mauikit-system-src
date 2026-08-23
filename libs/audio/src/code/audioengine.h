#pragma once

#include <QAbstractListModel>
#include <QtCore/qobjectdefs.h>

Q_MOC_INCLUDE("sink.h")
Q_MOC_INCLUDE("source.h")

namespace QPipeWireAudio
{
class AudioObject;
class Context;
class Sink;
class Source;
class SinkInput;
class SourceOutput;

/** @brief Base list model exposing PipeWire audio objects through AudioObjectRole. */
class AbstractModel : public QAbstractListModel
{
    Q_OBJECT
public:
    enum ItemRole {
        AudioObjectRole = Qt::UserRole + 1,
    };
    Q_PROPERTY(int count READ rowCount NOTIFY countChanged)

    Q_ENUM(ItemRole)

    ~AbstractModel() override;
    QHash<int, QByteArray> roleNames() const final;
    int rowCount(const QModelIndex &parent = QModelIndex()) const final;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    bool setData(const QModelIndex &index, const QVariant &value, int role) final;

    Q_INVOKABLE int role(const QByteArray &roleName) const;

Q_SIGNALS:
    void countChanged();

protected:
    explicit AbstractModel(QObject *parent = nullptr);
    void initRoleNames(const QMetaObject &qobjectMetaObject);
    Context *context() const;

    void replaceItems(const QList<AudioObject *> &items);
    QList<AudioObject *> items() const;

private:
    QList<AudioObject *> m_items;
    QHash<int, QByteArray> m_roles;
    QHash<int, int> m_objectProperties;
};

/** @brief Lists the audio cards reported by PipeWire. */
class CardModel : public AbstractModel
{
    Q_OBJECT
public:
    explicit CardModel(QObject *parent = nullptr);
};

/** @brief Lists audio output devices and identifies the default and preferred sink. */
class SinkModel : public AbstractModel
{
    Q_OBJECT
    Q_PROPERTY(QPipeWireAudio::Sink *defaultSink READ defaultSink NOTIFY defaultSinkChanged)
    Q_PROPERTY(QPipeWireAudio::Sink *preferredSink READ preferredSink NOTIFY preferredSinkChanged)
public:
    enum ItemRole {
        SortByDefaultRole = AudioObjectRole + 1,
    };
    Q_ENUM(ItemRole)

    explicit SinkModel(QObject *parent = nullptr);
    Sink *defaultSink() const;
    Sink *preferredSink() const;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

Q_SIGNALS:
    void defaultSinkChanged();
    void preferredSinkChanged();

private:
    void rebuild();
    Sink *m_preferredSink = nullptr;
};

/** @brief Lists application playback streams connected to audio sinks. */
class SinkInputModel : public AbstractModel
{
    Q_OBJECT
public:
    explicit SinkInputModel(QObject *parent = nullptr);

private:
    void rebuild();
};

/** @brief Lists audio input devices and identifies the default source. */
class SourceModel : public AbstractModel
{
    Q_OBJECT
    Q_PROPERTY(QPipeWireAudio::Source *defaultSource READ defaultSource NOTIFY defaultSourceChanged)
public:
    enum ItemRole {
        SortByDefaultRole = AudioObjectRole + 1,
    };
    Q_ENUM(ItemRole)

    explicit SourceModel(QObject *parent = nullptr);
    Source *defaultSource() const;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

Q_SIGNALS:
    void defaultSourceChanged();

private:
    void rebuild();
};

/** @brief Lists recording streams connected to audio sources. */
class SourceOutputModel : public AbstractModel
{
    Q_OBJECT
public:
    explicit SourceOutputModel(QObject *parent = nullptr);

private:
    void rebuild();
};

/** @brief Lists persisted per-stream audio settings. */
class StreamRestoreModel : public AbstractModel
{
    Q_OBJECT
public:
    explicit StreamRestoreModel(QObject *parent = nullptr);
};

/** @brief Lists the audio modules reported by PipeWire. */
class ModuleModel : public AbstractModel
{
    Q_OBJECT
public:
    explicit ModuleModel(QObject *parent = nullptr);
};

} // QPipeWireAudio
