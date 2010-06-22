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

#ifndef SYSTEMTRAYICON_H
#define SYSTEMTRAYICON_H

#include <QObject>
#include <QPixmap>

class QAction;

class SystemTrayIcon : public QObject {
  Q_OBJECT

 public:
  SystemTrayIcon(QObject* parent = 0);

  // Called once to create the icon's context menu
  virtual void SetupMenu(QAction* previous, QAction* play, QAction* stop,
                         QAction* stop_after, QAction* next, QAction* love,
                         QAction* ban, QAction* quit) = 0;

  virtual bool IsVisible() const { return true; }
  virtual void SetVisible(bool visible) {}

  // Called by the OSD
  virtual void ShowPopup(const QString& summary, const QString& message,
                         int timeout) {}

 public slots:
  void SetProgress(int percentage);
  void SetPaused();
  void SetPlaying();
  void SetStopped();

 signals:
  void ChangeVolume(int delta);
  void ShowHide();
  void PlayPause();

 protected:
  virtual void UpdateIcon() = 0;
  QPixmap CreateIcon(const QPixmap& icon, const QPixmap& grey_icon);

  int song_progress() const { return percentage_; }
  QPixmap current_state_icon() const { return current_state_icon_; }

 private:
  int percentage_;
  QPixmap playing_icon_;
  QPixmap paused_icon_;
  QPixmap current_state_icon_;
};

#endif // SYSTEMTRAYICON_H
