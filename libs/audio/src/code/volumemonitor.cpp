#include "volumemonitor.h"

#include "context.h"
#include "volumeobject.h"

namespace QPipeWireAudio
{
VolumeMonitor::VolumeMonitor(QObject *parent)
    : QObject(parent)
{
    Context::instance()->ref();
}

VolumeMonitor::~VolumeMonitor()
{
    setTarget(nullptr);
    Context::instance()->unref();
}

bool VolumeMonitor::isAvailable() const
{
    return m_target != nullptr;
}

VolumeObject *VolumeMonitor::target() const
{
    return m_target;
}

void VolumeMonitor::setTarget(VolumeObject *target)
{
    if (m_target == target) {
        return;
    }

    if (m_target) {
        disconnect(m_target, nullptr, this, nullptr);
    }

    m_target = target;

    if (m_target) {
        connect(m_target, &VolumeObject::volumeChanged, this, &VolumeMonitor::syncFromTarget);
        connect(m_target, &VolumeObject::mutedChanged, this, &VolumeMonitor::syncFromTarget);
        syncFromTarget();
    } else {
        m_volume = 0.0;
        Q_EMIT volumeChanged();
    }

    Q_EMIT availableChanged();
    Q_EMIT targetChanged();
}

void VolumeMonitor::syncFromTarget()
{
    if (!m_target || m_target->isMuted()) {
        if (!qFuzzyCompare(1.0 + m_volume, 1.0)) {
            m_volume = 0.0;
            Q_EMIT volumeChanged();
        }
        return;
    }

    const qreal normalized = qBound<qreal>(0.0, m_target->volume() / static_cast<qreal>(Context::NormalVolume), 1.0);
    if (qFuzzyCompare(1.0 + m_volume, 1.0 + normalized)) {
        return;
    }

    m_volume = normalized;
    Q_EMIT volumeChanged();
}

} // namespace QPipeWireAudio
