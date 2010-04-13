/* This file is part of Clementine.

   Clementine is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   Clementine is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with Clementine.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "osd.h"
#include "osdpretty.h"

#include <QCoreApplication>
#include <QtDebug>
#include <QSettings>

const char* OSD::kSettingsGroup = "OSD";

OSD::OSD(QSystemTrayIcon* tray_icon, QObject* parent)
  : QObject(parent),
    tray_icon_(tray_icon),
    timeout_msec_(5000),
    behaviour_(Native),
    show_on_volume_change_(false),
    show_art_(true),
    force_show_next_(false),
    pretty_popup_(new OSDPretty)
{
  ReloadSettings();
  Init();
}

OSD::~OSD() {
  delete pretty_popup_;
}

void OSD::ReloadSettings() {
  QSettings s;
  s.beginGroup(kSettingsGroup);
  behaviour_ = OSD::Behaviour(s.value("Behaviour", Native).toInt());
  timeout_msec_ = s.value("Timeout", 5000).toInt();
  show_on_volume_change_ = s.value("ShowOnVolumeChange", false).toBool();
  show_art_ = s.value("ShowArt", true).toBool();

  if (!SupportsNativeNotifications() && behaviour_ == Native)
    behaviour_ = Pretty;
  if (!SupportsTrayPopups() && behaviour_ == TrayPopup)
    behaviour_ = Disabled;

  pretty_popup_->set_popup_duration(timeout_msec_);
  pretty_popup_->ReloadSettings();
}

void OSD::SongChanged(const Song &song) {
  QString summary(song.PrettyTitle());
  if (!song.artist().isEmpty())
    summary = QString("%1 - %2").arg(song.artist(), summary);

  QStringList message_parts;
  if (!song.album().isEmpty())
    message_parts << song.album();
  if (song.disc() > 0)
    message_parts << tr("disc %1").arg(song.disc());
  if (song.track() > 0)
    message_parts << tr("track %1").arg(song.track());

  ShowMessage(summary, message_parts.join(", "), "notification-audio-play",
              show_art_ ? song.GetBestImage() : QImage());
}

void OSD::Paused() {
  ShowMessage(QCoreApplication::applicationName(), tr("Paused"));
}

void OSD::Stopped() {
  ShowMessage(QCoreApplication::applicationName(), tr("Playlist finished"));
}

void OSD::VolumeChanged(int value) {
  if (!show_on_volume_change_)
    return;

  ShowMessage(QCoreApplication::applicationName(), tr("Volume %1%").arg(value));
}

void OSD::ShowMessage(const QString& summary,
                      const QString& message,
                      const QString& icon,
                      const QImage& image) {
  switch (behaviour_) {
    case Native:
      if (image.isNull()) {
        ShowMessageNative(summary, message, icon, QImage());
      } else {
        ShowMessageNative(summary, message, QString(), image);
      }
      break;

    case TrayPopup:
      tray_icon_->showMessage(summary, message, QSystemTrayIcon::NoIcon, timeout_msec_);
      break;

    case Disabled:
      if (!force_show_next_)
        break;
      force_show_next_ = false;
      // fallthrough
    case Pretty:
      pretty_popup_->SetMessage(summary, message, image);
      pretty_popup_->show();
      break;

    default:
      break;
  }
}

#ifndef Q_WS_X11
void OSD::CallFinished(QDBusPendingCallWatcher*) {}
#endif
