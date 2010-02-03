#include "osd.h"

#include <QtDebug>

void OSD::Init() {
}

bool OSD::CanShowNativeMessages() const {
  return true;
}

void OSD::ShowMessageNative(const QString& summary, const QString& message,
                            const QString& icon) {
  // This should use growl
  tray_icon_->showMessage(summary, message, QSystemTrayIcon::NoIcon, timeout_);
}
