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

#include <QSystemTrayIcon>

class SystemTrayIcon : public QSystemTrayIcon {
  Q_OBJECT

 public:
  SystemTrayIcon(QObject* parent = 0);
  virtual ~SystemTrayIcon();

  virtual bool event(QEvent* event);

 public slots:
  void SetProgress(int percentage);
  void SetPaused();
  void SetPlaying();
  void SetStopped();

 signals:
  void WheelEvent(int delta);

 private:
  void Update();

  QPixmap icon_;
  QPixmap grey_icon_;
  QPixmap playing_icon_;
  QPixmap paused_icon_;

  int percentage_;
  QPixmap current_state_icon_;
};

#endif // SYSTEMTRAYICON_H
