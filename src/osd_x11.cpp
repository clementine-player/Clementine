// Libnotify headers need to go before Qt ones because they use "signals" as
// a variable name
#include <libnotify/notify.h>
#include <glib.h>

#include "osd.h"

#include <QCoreApplication>
#include <QtDebug>
#include <QTextDocument>

void OSD::Init() {
  notification_ = NULL;
  notify_init(QCoreApplication::applicationName().toUtf8().constData());
}

bool OSD::CanShowNativeMessages() const {
  return true;
}

void OSD::ShowMessageNative(const QString& summary, const QString& message,
                            const QString& icon) {
  if (summary.isNull())
    return;

  #define STR(x) (x.isNull() ? NULL : x.toUtf8().constData())

  if (notification_) {
    notify_notification_update(notification_,
        STR(summary), STR(Qt::escape(message)), STR(icon));
  } else {
    notification_ = notify_notification_new(
        STR(summary), STR(Qt::escape(message)), STR(icon), NULL);
  }

  #undef STR

  notify_notification_set_urgency(notification_, NOTIFY_URGENCY_LOW);
  notify_notification_set_timeout(notification_, timeout_);

  GError* error = NULL;
  notify_notification_show(notification_, &error);
  if (error) {
    qDebug() << "Error from notify_notification_show:" << error->message;
    g_error_free(error);
  }
}
