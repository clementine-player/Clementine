// Libnotify headers need to go before Qt ones because they use "signals" as
// a variable name
#include <libnotify/notify.h>
#include <glib.h>

#include "osd.h"

#include <QtDebug>

void OSD::Init() {
  notify_init("Tangerine");
}

void OSD::ShowMessage(const QString& summary, const QString& message,
                      const QString& icon) {
  if (summary.isNull())
    return;

  NotifyNotification* notification = notify_notification_new(
      summary.toUtf8().constData(),
      message.isNull() ? NULL : message.toUtf8().constData(),
      icon.isNull() ? NULL : icon.toUtf8().constData(), NULL);

  notify_notification_set_urgency(notification, NOTIFY_URGENCY_LOW);
  notify_notification_set_timeout(notification, 5000);

  GError* error = NULL;
  notify_notification_show(notification, &error);
  if (error) {
    qDebug() << "Error from notify_notification_show:" << error->message;
    g_error_free(error);
  }

  g_object_unref(G_OBJECT(notification));
}
