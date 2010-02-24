#include "osd.h"

#include <QtDebug>

void OSD::Init() {
}

bool OSD::CanShowNativeMessages() const {
  return false;
}

void OSD::ShowMessageNative(const QString&, const QString&,
                            const QString&) {
  qWarning() << __PRETTY_FUNCTION__ << ": NOT IMPLEMENTED";
}

void OSD::ShowMessageNative(const QString& summary, const QString& message,
                            const QImage& image) {
  qWarning() << __PRETTY_FUNCTION__ << ": NOT IMPLEMENTED";
}
