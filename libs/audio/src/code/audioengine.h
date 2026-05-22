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

class CardModel : public AbstractModel
{
    Q_OBJECT
public:
    explicit CardModel(QObject *parent = nullptr);
};

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

class SinkInputModel : public AbstractModel
{
    Q_OBJECT
public:
    explicit SinkInputModel(QObject *parent = nullptr);

private:
    void rebuild();
};

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

class SourceOutputModel : public AbstractModel
{
    Q_OBJECT
public:
    explicit SourceOutputModel(QObject *parent = nullptr);

private:
    void rebuild();
};

class StreamRestoreModel : public AbstractModel
{
    Q_OBJECT
public:
    explicit StreamRestoreModel(QObject *parent = nullptr);
};

class ModuleModel : public AbstractModel
{
    Q_OBJECT
public:
    explicit ModuleModel(QObject *parent = nullptr);
};

} // QPipeWireAudio
