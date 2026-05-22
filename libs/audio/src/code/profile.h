#pragma once

#include <QObject>

namespace QPipeWireAudio
{
class Profile : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString name READ name WRITE setName NOTIFY nameChanged)
    Q_PROPERTY(QString description READ description WRITE setDescription NOTIFY descriptionChanged)
    Q_PROPERTY(quint32 priority READ priority WRITE setPriority NOTIFY priorityChanged)
    Q_PROPERTY(Availability availability READ availability WRITE setAvailability NOTIFY availabilityChanged)
public:
    enum Availability {
        Unknown,
        Available,
        Unavailable,
    };
    Q_ENUM(Availability)

    explicit Profile(QObject *parent = nullptr);
    ~Profile() override;

    QString name() const;
    void setName(const QString &name);

    QString description() const;
    void setDescription(const QString &description);

    quint32 priority() const;
    void setPriority(quint32 priority);

    Availability availability() const;
    void setAvailability(Availability availability);

Q_SIGNALS:
    void nameChanged();
    void descriptionChanged();
    void priorityChanged();
    void availabilityChanged();

private:
    QString m_name;
    QString m_description;
    quint32 m_priority = 0;
    Availability m_availability = Unknown;
};
}
