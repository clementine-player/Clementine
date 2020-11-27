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

#ifndef ADDSTREAMDIALOG_H
#define ADDSTREAMDIALOG_H

#include <QDialog>
#include <QUrl>

class SavedRadio;
class Ui_AddStreamDialog;

class AddStreamDialog : public QDialog {
  Q_OBJECT

 public:
  AddStreamDialog(QWidget* parent = nullptr);
  ~AddStreamDialog();

  void set_url(const QUrl& url);
  void set_name(const QString& name);
  void set_url_logo(const QUrl& url);
  void set_save_visible(bool visible);
  void set_add_on_accept(SavedRadio* saved_radio) {
    saved_radio_ = saved_radio;
  }

  QUrl url() const;
  QString name() const;
  QUrl url_logo() const;

  void accept();

 protected:
  void showEvent(QShowEvent*);

 private slots:
  void TextChanged(const QString& text);

 private:
  static const char* kSettingsGroup;

  Ui_AddStreamDialog* ui_;

  SavedRadio* saved_radio_;
};

#endif  // ADDSTREAMDIALOG_H
