#pragma once

#include <QMap>
#include <QObject>
#include <QSet>

namespace QPipeWireAudio
{
class MapBaseQObject : public QObject
{
    Q_OBJECT

public:
    virtual int count() const = 0;
    virtual QObject *objectAt(int index) const = 0;
    virtual int indexOfObject(QObject *object) const = 0;

Q_SIGNALS:
    void aboutToBeAdded(int index);
    void added(int index);
    void aboutToBeRemoved(int index);
    void removed(int index);
};

template<typename Type, typename BackendInfo = void>
class MapBase : public MapBaseQObject
{
public:
    ~MapBase() override = default;

    const QMap<quint32, Type *> &data() const
    {
        return m_data;
    }

    int count() const override
    {
        return m_data.count();
    }

    int indexOfObject(QObject *object) const override
    {
        int index = 0;
        QMapIterator<quint32, Type *> it(m_data);
        while (it.hasNext()) {
            it.next();
            if (it.value() == object) {
                return index;
            }
            ++index;
        }
        return -1;
    }

    QObject *objectAt(int index) const override
    {
        return (m_data.constBegin() + index).value();
    }

    void reset()
    {
        while (!m_data.isEmpty()) {
            removeEntry(m_data.lastKey());
        }
        m_pendingRemovals.clear();
    }

    void insert(Type *object)
    {
        Q_ASSERT(!m_data.contains(object->index()));

        int modelIndex = 0;
        for (auto it = m_data.constBegin(); it != m_data.constEnd(); ++it) {
            if (object->index() < it.key()) {
                break;
            }
            ++modelIndex;
        }

        Q_EMIT aboutToBeAdded(modelIndex);
        m_data.insert(object->index(), object);
        Q_EMIT added(modelIndex);
    }

    template<typename InfoType>
    void updateEntry(const InfoType *, QObject *)
    {
    }

    void removeEntry(quint32 index)
    {
        if (!m_data.contains(index)) {
            m_pendingRemovals.insert(index);
            return;
        }

        const int modelIndex = m_data.keys().indexOf(index);
        Q_EMIT aboutToBeRemoved(modelIndex);
        delete m_data.take(index);
        Q_EMIT removed(modelIndex);
    }

protected:
    QMap<quint32, Type *> m_data;
    QSet<quint32> m_pendingRemovals;
};

} // namespace QPipeWireAudio
