#pragma once

#include <QObject>
#include <QtCore/qobjectdefs.h>

Q_MOC_INCLUDE("volumeobject.h")

namespace QPipeWireAudio
{
class VolumeObject;

class VolumeMonitor : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QPipeWireAudio::VolumeObject *target READ target WRITE setTarget NOTIFY targetChanged)
    Q_PROPERTY(qreal volume MEMBER m_volume NOTIFY volumeChanged)
    Q_PROPERTY(bool available READ isAvailable NOTIFY availableChanged)

public:
    explicit VolumeMonitor(QObject *parent = nullptr);
    ~VolumeMonitor() override;

    bool isAvailable() const;

    VolumeObject *target() const;
    void setTarget(VolumeObject *target);

Q_SIGNALS:
    void volumeChanged();
    void targetChanged();
    void availableChanged();

private:
    void syncFromTarget();

    VolumeObject *m_target = nullptr;
    qreal m_volume = 0.0;
};
}
