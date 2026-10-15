/*
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 */

#include "notifications_adaptor.h"
#include "notificationserver.h"

NotificationsAdaptor::NotificationsAdaptor(QObject *parent)
    : QDBusAbstractAdaptor(parent)
{
    setAutoRelaySignals(true);
}

NotificationsAdaptor::~NotificationsAdaptor() = default;

void NotificationsAdaptor::CloseNotification(uint id)
{
    static_cast<NotificationServer *>(parent())->CloseNotification(id);
}

QStringList NotificationsAdaptor::GetCapabilities()
{
    return static_cast<NotificationServer *>(parent())->GetCapabilities();
}

QString NotificationsAdaptor::GetServerInformation(QString &vendor, QString &version, QString &spec_version)
{
    return static_cast<NotificationServer *>(parent())->GetServerInformation(vendor, version, spec_version);
}

uint NotificationsAdaptor::Notify(const QString &app_name,
                                  uint replaces_id,
                                  const QString &app_icon,
                                  const QString &summary,
                                  const QString &body,
                                  const QStringList &actions,
                                  const QVariantMap &hints,
                                  int timeout)
{
    return static_cast<NotificationServer *>(parent())->Notify(app_name, replaces_id, app_icon, summary, body, actions, hints, timeout);
}
