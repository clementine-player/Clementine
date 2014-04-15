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

#ifndef SONGINFOTEXTVIEW_H
#define SONGINFOTEXTVIEW_H

#include <QTextBrowser>

class SongInfoTextView : public QTextBrowser {
  Q_OBJECT

 public:
  SongInfoTextView(QWidget* parent = nullptr);

  static const qreal kDefaultFontSize;
  static const char* kSettingsGroup;

  static qreal FontSize();
  static QFont Font();

  QSize sizeHint() const;

 public slots:
  void ReloadSettings();
  void SetHtml(const QString& html);

signals:
  void ShowSettingsDialog();

 protected:
  void resizeEvent(QResizeEvent* e);
  void wheelEvent(QWheelEvent* e);
  void contextMenuEvent(QContextMenuEvent* e);
  QVariant loadResource(int type, const QUrl& name);

 private:
  int last_width_;
  bool recursion_filter_;
};

#endif  // SONGINFOTEXTVIEW_H
