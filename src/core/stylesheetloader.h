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

#ifndef STYLESHEETLOADER_H
#define STYLESHEETLOADER_H

#include <QString>
#include <QPalette>
#include <QWidget>
#include <QMap>

class StyleSheetLoader : public QObject {
 public:
  StyleSheetLoader(QObject* parent = 0);

  // Sets the given stylesheet on the given widget.
  // If the stylesheet contains strings like %palette-[role], these get replaced
  // with actual palette colours.
  // The stylesheet is reloaded when the widget's palette changes.
  void SetStyleSheet(QWidget* widget, const QString& filename);

 protected:
  bool eventFilter(QObject* obj, QEvent* event);

 private:
  void UpdateStyleSheet(QWidget* widget);
  void ReplaceColor(QString* css, const QString& name, const QPalette& palette,
                    QPalette::ColorRole role) const;

 private:
  QMap<QWidget*, QString> filenames_;
};

#endif  // STYLESHEETLOADER_H
