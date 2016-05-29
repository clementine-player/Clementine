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

#ifndef QTSYSTEMTRAYICON_H
#define QTSYSTEMTRAYICON_H

#include "mainwindow.h"
#include "systemtrayicon.h"

#include <QSystemTrayIcon>

class QtSystemTrayIcon : public SystemTrayIcon {
  Q_OBJECT

 public:
  QtSystemTrayIcon(QObject* parent = nullptr);
  ~QtSystemTrayIcon();

  void SetupMenu(QAction* previous, QAction* play, QAction* stop,
                 QAction* stop_after, QAction* next, QAction* mute,
                 QAction* love, QAction* quit);
  bool IsVisible() const;
  void SetVisible(bool visible);

  void ShowPopup(const QString& summary, const QString& message, int timeout);

  void SetNowPlaying(const Song& song, const QString& image_path);
  void ClearNowPlaying();

 protected:
  // SystemTrayIcon
  void UpdateIcon();
  void SetPaused();
  void SetPlaying(bool enable_play_pause = false, bool enable_love = false);
  void SetStopped();
  void LastFMButtonVisibilityChanged(bool value);
  void LastFMButtonLoveStateChanged(bool value);
  void MuteButtonStateChanged(bool value);

  // QObject
  bool eventFilter(QObject*, QEvent*);

 private slots:
  void Clicked(QSystemTrayIcon::ActivationReason);

 private:
  QSystemTrayIcon* tray_;
  QMenu* menu_;
  QAction* action_play_pause_;
  QAction* action_stop_;
  QAction* action_stop_after_this_track_;
  QAction* action_mute_;
  QAction* action_love_;

  QString pattern_;

  QPixmap orange_icon_;
  QPixmap grey_icon_;
};

#endif  // QTSYSTEMTRAYICON_H
