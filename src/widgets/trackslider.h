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

#ifndef TRACKSLIDER_H
#define TRACKSLIDER_H

#include <QWidget>

class QLabel;

class Application;
class MoodbarProxyStyle;
class Ui_TrackSlider;

class TrackSlider : public QWidget {
  Q_OBJECT

 public:
  TrackSlider(QWidget* parent = 0);
  ~TrackSlider();

  void SetApplication(Application* app);

  // QWidget
  QSize sizeHint() const;

  // QObject
  bool event(QEvent*);

  MoodbarProxyStyle* moodbar_style() const { return moodbar_style_; }

  static const char* kSettingsGroup;

 public slots:
  void SetValue(int elapsed, int total);
  void SetStopped();
  void SetCanSeek(bool can_seek);
  void Seek(int gap);

signals:
  void ValueChanged(int value);

 private slots:
  void ValueMaybeChanged(int value);
  void ToggleTimeDisplay();

 private:
  void UpdateTimes(int elapsed);
  void UpdateLabelWidth();
  void UpdateLabelWidth(QLabel* label, const QString& text);

 private:
  Ui_TrackSlider* ui_;

  MoodbarProxyStyle* moodbar_style_;

  bool setting_value_;
  bool show_remaining_time_;
  int slider_maximum_value_;  // we cache it to avoid unnecessary updates
};

#endif  // TRACKSLIDER_H
