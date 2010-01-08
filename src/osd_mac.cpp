#include "osd.h"

#include <QtDebug>

void OSD::Init() {
}

void OSD::ShowMessage(const QString& summary, const QString& message,
                      const QString& icon) {
  tray_icon_->showMessage(summary, message, QSystemTrayIcon::NoIcon, timeout_);
}
