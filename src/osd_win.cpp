#include "osd.h"

#include <QtDebug>

void OSD::Init() {
}

bool OSD::CanShowNativeMessages() const {
  return false;
}

void OSD::ShowMessageNative(const QString&, const QString&,
                            const QString&) {
}

