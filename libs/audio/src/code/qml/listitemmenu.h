#pragma once

#include <QObject>
#include <QPointer>
#include <QQmlParserStatus>

class QAbstractItemModel;
class QMenu;
class QQuickItem;

namespace QPipeWireAudio
{
class CardModel;
class AudioObject;
}

class ListItemMenu : public QObject, public QQmlParserStatus
{
    Q_OBJECT
    Q_INTERFACES(QQmlParserStatus)

    Q_PROPERTY(ItemType itemType READ itemType WRITE setItemType NOTIFY itemTypeChanged)

    Q_PROPERTY(QPipeWireAudio::AudioObject *audioObject READ audioObject WRITE setAudioObject NOTIFY audioObjectChanged)

    Q_PROPERTY(QAbstractItemModel *sourceModel READ sourceModel WRITE setSourceModel NOTIFY sourceModelChanged)

    Q_PROPERTY(QPipeWireAudio::CardModel *cardModel READ cardModel WRITE setCardModel NOTIFY cardModelChanged)

    Q_PROPERTY(bool visible READ isVisible NOTIFY visibleChanged)

    Q_PROPERTY(bool hasContent READ hasContent NOTIFY hasContentChanged)

    Q_PROPERTY(QQuickItem *visualParent READ visualParent WRITE setVisualParent NOTIFY visualParentChanged)

public:
    explicit ListItemMenu(QObject *parent = nullptr);
    ~ListItemMenu() override;

    enum ItemType {
        None,
        Sink,
        SinkInput,
        Source,
        SourceOutput,
    };
    Q_ENUM(ItemType)

    ItemType itemType() const;
    void setItemType(ItemType itemType);
    Q_SIGNAL void itemTypeChanged();

    QPipeWireAudio::AudioObject *audioObject() const;
    void setAudioObject(QPipeWireAudio::AudioObject *audioObject);
    Q_SIGNAL void audioObjectChanged();

    QAbstractItemModel *sourceModel() const;
    void setSourceModel(QAbstractItemModel *sourceModel);
    Q_SIGNAL void sourceModelChanged();

    QPipeWireAudio::CardModel *cardModel() const;
    void setCardModel(QPipeWireAudio::CardModel *cardModel);
    Q_SIGNAL void cardModelChanged();

    bool isVisible() const;
    Q_SIGNAL void visibleChanged();

    bool hasContent() const;
    Q_SIGNAL void hasContentChanged();

    QQuickItem *visualParent() const;
    void setVisualParent(QQuickItem *visualParent);
    Q_SIGNAL void visualParentChanged();

    void classBegin() override;
    void componentComplete() override;

    Q_INVOKABLE void open(int x, int y);
    Q_INVOKABLE void openRelative();

private:
    void setVisible(bool visible);

    void update();
    bool checkHasContent();
    QMenu *createMenu();

    bool m_complete = false;
    bool m_visible = false;
    bool m_hasContent = false;
    QPointer<QQuickItem> m_visualParent;

    ItemType m_itemType = None;
    QPointer<QPipeWireAudio::AudioObject> m_audioObject;
    QPointer<QAbstractItemModel> m_sourceModel;
    QPointer<QPipeWireAudio::CardModel> m_cardModel;
};
