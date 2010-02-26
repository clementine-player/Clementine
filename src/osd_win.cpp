#include "osd.h"

#include <QtDebug>

void OSD::Init() {
}

bool OSD::SupportsNativeNotifications() {
  return false;
}

bool OSD::SupportsTrayPopups() {
  return true;
}

void OSD::ShowMessageNative(const QString&, const QString&,
                            const QString&) {
  qWarning() << __PRETTY_FUNCTION__ << ": NOT IMPLEMENTED";
}

void OSD::ShowMessageNative(const QString& summary, const QString& message,
                            const QImage& image) {
  qWarning() << __PRETTY_FUNCTION__ << ": NOT IMPLEMENTED";
}
