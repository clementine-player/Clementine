/* This file is part of Clementine.
   Copyright 2010, David Sansome <me@davidsansome.com>

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

#include "iconloader.h"
#include "qtsystemtrayicon.h"

#include "core/song.h"

#include <QCoreApplication>
#include <QTextDocument>
#include <QStringBuilder>
#include <QMenu>
#include <QFile>
#include <QSystemTrayIcon>
#include <QWheelEvent>

QtSystemTrayIcon::QtSystemTrayIcon(QObject* parent)
    : SystemTrayIcon(parent),
      tray_(new QSystemTrayIcon(this)),
      menu_(new QMenu),
      action_play_pause_(nullptr),
      action_stop_(nullptr),
      action_stop_after_this_track_(nullptr),
      action_mute_(nullptr),
      action_love_(nullptr) {
  QIcon theme_icon = IconLoader::Load("clementine-panel", IconLoader::Base);
  QIcon theme_icon_grey =
      IconLoader::Load("clementine-panel-grey", IconLoader::Base);

  if (theme_icon.isNull() || theme_icon_grey.isNull()) {
    // Load the default icon
    QIcon icon(":icon.png");
    orange_icon_ = icon.pixmap(48, QIcon::Normal);
    grey_icon_ = icon.pixmap(48, QIcon::Disabled);
  } else {
    // Use the icons from the theme
    orange_icon_ = theme_icon.pixmap(48);
    grey_icon_ = theme_icon_grey.pixmap(48);
  }

  tray_->setIcon(orange_icon_);
  tray_->installEventFilter(this);
  ClearNowPlaying();

  QFile pattern_file(":/now_playing_tooltip.txt");
  pattern_file.open(QIODevice::ReadOnly);
  pattern_ = QString::fromLatin1(pattern_file.readAll());

  connect(tray_, SIGNAL(activated(QSystemTrayIcon::ActivationReason)),
          SLOT(Clicked(QSystemTrayIcon::ActivationReason)));
}

QtSystemTrayIcon::~QtSystemTrayIcon() { delete menu_; }

bool QtSystemTrayIcon::eventFilter(QObject* object, QEvent* event) {
  if (QObject::eventFilter(object, event)) return true;

  if (object != tray_) return false;

  if (event->type() == QEvent::Wheel) {
    QWheelEvent* e = static_cast<QWheelEvent*>(event);
    if (e->modifiers() == Qt::ShiftModifier) {
      if (e->delta() > 0) {
        emit SeekForward();
      } else {
        emit SeekBackward();
      }
    } else if (e->modifiers() == Qt::ControlModifier) {
      if (e->delta() < 0) {
        emit NextTrack();
      } else {
        emit PreviousTrack();
      }
    } else {
      QSettings s;
      s.beginGroup(MainWindow::kSettingsGroup);
      bool prev_next_track = s.value("scrolltrayicon").toBool();
      if (prev_next_track) {
        if (e->delta() < 0) {
          emit NextTrack();
        } else {
          emit PreviousTrack();
        }
      } else {
        emit ChangeVolume(e->delta());
      }
    }
    return true;
  }

  return false;
}

void QtSystemTrayIcon::SetupMenu(QAction* previous, QAction* play,
                                 QAction* stop, QAction* stop_after,
                                 QAction* next, QAction* mute, QAction* love,
                                 QAction* quit) {
  // Creating new actions and connecting them to old ones. This allows us to
  // use old actions without displaying shortcuts that can not be used when
  // Clementine's window is hidden
  menu_->addAction(previous->icon(), previous->text(), previous,
                   SLOT(trigger()));
  action_play_pause_ =
      menu_->addAction(play->icon(), play->text(), play, SLOT(trigger()));
  action_stop_ =
      menu_->addAction(stop->icon(), stop->text(), stop, SLOT(trigger()));
  action_stop_after_this_track_ = menu_->addAction(
      stop_after->icon(), stop_after->text(), stop_after, SLOT(trigger()));
  menu_->addAction(next->icon(), next->text(), next, SLOT(trigger()));

  menu_->addSeparator();
  action_mute_ =
      menu_->addAction(mute->icon(), mute->text(), mute, SLOT(trigger()));
  action_mute_->setCheckable(true);
  action_mute_->setChecked(mute->isChecked());

  menu_->addSeparator();
#ifdef HAVE_LIBLASTFM
  action_love_ =
      menu_->addAction(love->icon(), love->text(), love, SLOT(trigger()));
  action_love_->setVisible(love->isVisible());
  action_love_->setEnabled(love->isEnabled());
#endif

  menu_->addSeparator();
  menu_->addAction(quit->icon(), quit->text(), quit, SLOT(trigger()));

  tray_->setContextMenu(menu_);
}

void QtSystemTrayIcon::Clicked(QSystemTrayIcon::ActivationReason reason) {
  switch (reason) {
    case QSystemTrayIcon::DoubleClick:
    case QSystemTrayIcon::Trigger:
      emit ShowHide();
      break;

    case QSystemTrayIcon::MiddleClick:
      emit PlayPause();
      break;

    default:
      break;
  }
}

void QtSystemTrayIcon::ShowPopup(const QString& summary, const QString& message,
                                 int timeout) {
  tray_->showMessage(summary, message, QSystemTrayIcon::NoIcon, timeout);
}

void QtSystemTrayIcon::UpdateIcon() {
  tray_->setIcon(CreateIcon(orange_icon_, grey_icon_));
}

void QtSystemTrayIcon::SetPaused() {
  SystemTrayIcon::SetPaused();

  action_stop_->setEnabled(true);
  action_stop_after_this_track_->setEnabled(true);
  action_play_pause_->setIcon(
      IconLoader::Load("media-playback-start", IconLoader::Base));
  action_play_pause_->setText(tr("Play"));

  action_play_pause_->setEnabled(true);
}

void QtSystemTrayIcon::SetPlaying(bool enable_play_pause, bool enable_love) {
  SystemTrayIcon::SetPlaying();

  action_stop_->setEnabled(true);
  action_stop_after_this_track_->setEnabled(true);
  action_play_pause_->setIcon(
      IconLoader::Load("media-playback-pause", IconLoader::Base));
  action_play_pause_->setText(tr("Pause"));
  action_play_pause_->setEnabled(enable_play_pause);
#ifdef HAVE_LIBLASTFM
  action_love_->setEnabled(enable_love);
#endif
}

void QtSystemTrayIcon::SetStopped() {
  SystemTrayIcon::SetStopped();

  action_stop_->setEnabled(false);
  action_stop_after_this_track_->setEnabled(false);
  action_play_pause_->setIcon(
      IconLoader::Load("media-playback-start", IconLoader::Base));
  action_play_pause_->setText(tr("Play"));

  action_play_pause_->setEnabled(true);

#ifdef HAVE_LIBLASTFM
  action_love_->setEnabled(false);
#endif
}

void QtSystemTrayIcon::LastFMButtonVisibilityChanged(bool value) {
#ifdef HAVE_LIBLASTFM
  action_love_->setVisible(value);
#endif
}

void QtSystemTrayIcon::LastFMButtonLoveStateChanged(bool value) {
#ifdef HAVE_LIBLASTFM
  action_love_->setEnabled(value);
#endif
}

void QtSystemTrayIcon::MuteButtonStateChanged(bool value) {
  if (action_mute_) action_mute_->setChecked(value);
}

bool QtSystemTrayIcon::IsVisible() const { return tray_->isVisible(); }

void QtSystemTrayIcon::SetVisible(bool visible) { tray_->setVisible(visible); }

void QtSystemTrayIcon::SetNowPlaying(const Song& song,
                                     const QString& image_path) {
#ifdef Q_OS_WIN
  // Windows doesn't support HTML in tooltips, so just show something basic
  tray_->setToolTip(song.PrettyTitleWithArtist());
  return;
#endif

  int columns = image_path == nullptr ? 1 : 2;

  QString clone = pattern_;

  clone.replace("%columns", QString::number(columns));
  clone.replace("%appName", QCoreApplication::applicationName());

  clone.replace("%titleKey", tr("Title") % ":");
  clone.replace("%titleValue", song.PrettyTitle().toHtmlEscaped());
  clone.replace("%artistKey", tr("Artist") % ":");
  clone.replace("%artistValue", song.artist().toHtmlEscaped());
  clone.replace("%albumKey", tr("Album") % ":");
  clone.replace("%albumValue", song.album().toHtmlEscaped());

  clone.replace("%lengthKey", tr("Length") % ":");
  clone.replace("%lengthValue", song.PrettyLength().toHtmlEscaped());

  if (columns == 2) {
    QString final_path =
        image_path.startsWith("file://") ? image_path.mid(7) : image_path;
    clone.replace("%image",
                  "    <td>"
                  "      <img src=\"" %
                      final_path %
                      "\" />"
                      "    </td>");
  } else {
    clone.replace("%image", "");
  }

  // TODO: we should also repaint this
  tray_->setToolTip(clone);
}

void QtSystemTrayIcon::ClearNowPlaying() {
  tray_->setToolTip(QCoreApplication::applicationName());
}
