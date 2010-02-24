// Libnotify headers need to go before Qt ones because they use "signals" as
// a variable name
#include <libnotify/notify.h>
#include <gdk-pixbuf/gdk-pixbuf.h>
#include <glib.h>

#include "osd.h"

#include <QCoreApplication>
#include <QtDebug>
#include <QTextDocument>

void OSD::Init() {
  notification_ = NULL;
  pixbuf_ = NULL;
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

  notification_ = notify_notification_new(
      STR(summary), STR(Qt::escape(message)), STR(icon), NULL);

  #undef STR

  notify_notification_set_urgency(notification_, NOTIFY_URGENCY_LOW);
  notify_notification_set_timeout(notification_, timeout_);

  if (pixbuf_) {
    notify_notification_set_icon_from_pixbuf(notification_, pixbuf_);
  }

  GError* error = NULL;
  notify_notification_show(notification_, &error);
  if (error) {
    qDebug() << "Error from notify_notification_show:" << error->message;
    g_error_free(error);
  }

  pixbuf_ = NULL;
}

void OSD::ShowMessageNative(const QString& summary, const QString& message,
                            const QImage& image) {
  QImage happy_gdk_image = image.convertToFormat(QImage::Format_RGB888);
  pixbuf_ = gdk_pixbuf_new_from_data(
      happy_gdk_image.bits(),
      GDK_COLORSPACE_RGB,
      false,  // has_alpha
      8,      // bits_per_sample
      happy_gdk_image.width(),
      happy_gdk_image.height(),
      happy_gdk_image.bytesPerLine(),
      NULL, NULL);

  ShowMessageNative(summary, message, QString());
}
