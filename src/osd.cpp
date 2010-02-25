#include "osd.h"

#include <QCoreApplication>
#include <QtDebug>
#include <QSettings>

const char* OSD::kSettingsGroup = "OSD";

OSD::OSD(QSystemTrayIcon* tray_icon, QObject* parent)
  : QObject(parent),
    tray_icon_(tray_icon),
    timeout_(5000),
    behaviour_(Native)
{
  ReloadSettings();
  Init();
}

void OSD::ReloadSettings() {
  QSettings s;
  s.beginGroup(kSettingsGroup);
  behaviour_ = OSD::Behaviour(s.value("Behaviour", Native).toInt());
  timeout_ = s.value("Timeout", 5000).toInt();

  if (!CanShowNativeMessages() && behaviour_ == Native)
    behaviour_ = TrayPopup;
}

void OSD::SongChanged(const Song &song) {
  qDebug() << __PRETTY_FUNCTION__;
  QString summary(song.PrettyTitle());
  if (!song.artist().isNull())
    summary = QString("%1 - %2").arg(song.artist(), summary);

  QStringList message_parts;
  if (!song.album().isEmpty())
    message_parts << song.album();
  if (song.disc() > 0)
    message_parts << QString("disc %1").arg(song.disc());
  if (song.track() > 0)
    message_parts << QString("track %1").arg(song.track());

  ShowMessage(summary, message_parts.join(", "), "notification-audio-play", song.image());
}

void OSD::Paused() {
  ShowMessage(QCoreApplication::applicationName(), tr("Paused"));
}

void OSD::Stopped() {
  ShowMessage(QCoreApplication::applicationName(), tr("Playlist finished"));
}

void OSD::VolumeChanged(int value) {
  ShowMessage(QCoreApplication::applicationName(), tr("Volume %1%").arg(value));
}

void OSD::ShowMessage(const QString& summary,
                      const QString& message,
                      const QString& icon,
                      const QImage& image) {
  switch (behaviour_) {
    case Native:
      if (image.isNull()) {
        ShowMessageNative(summary, message, icon);
      } else {
        ShowMessageNative(summary, message, image);
      }
      break;

    case TrayPopup:
      tray_icon_->showMessage(summary, message, QSystemTrayIcon::NoIcon, timeout_);
      break;

    case Disabled:
    default:
      break;
  }
}
