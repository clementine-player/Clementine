/* This file is part of Clementine.
   Copyright 2010, David Sansome <me@davidsansome.com>
   Copyright 2021, Lukas Prediger <lumip@lumip.de>

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

#ifndef FILENAMEFORMATWIDGET_H
#define FILENAMEFORMATWIDGET_H

#include <QWidget>
#include <memory>

#include "core/organiseformat.h"
#include "ui_filenameformatwidget.h"

class FileNameFormatWidget : public QWidget {
  Q_OBJECT
 public:
  static const char* kDefaultFormat;
  static const char* kSettingsGroup;

 signals:
  void OptionChanged();
  void FormatStringChanged();

 private slots:
  void InsertTag(const QString& tag);

 public:
  FileNameFormatWidget(QWidget* parent);
  void Reset();
  void StoreSettings();

  bool ignore_the() const;
  bool replace_spaces() const;
  bool restrict_to_ascii() const;
  bool overwrite_existing() const;
  bool mark_as_listened() const;
  OrganiseFormat format() const;

 private:
  void LoadSettings();

  std::unique_ptr<Ui_FileNameFormatWidget> ui_;
};

#endif  // FILENAMEFORMATWIDGET_H
