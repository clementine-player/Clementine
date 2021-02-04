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

#ifndef TRANSCODEROPTIONSDIALOG_H
#define TRANSCODEROPTIONSDIALOG_H

#include <QDialog>

#include "transcoderoptionsinterface.h"

class Ui_TranscoderOptionsDialog;
struct TranscoderPreset;

class TranscoderOptionsDialog : public QDialog {
  Q_OBJECT

 public:
  TranscoderOptionsDialog(const TranscoderPreset& preset,
                          QWidget* parent = nullptr);
  ~TranscoderOptionsDialog();

  void accept();

  void set_settings_postfix(const QString& settings_postfix);

  static TranscoderOptionsInterface* MakeOptionsPage(const QString& mime_type,
                                                     QWidget* parent = nullptr);

 protected:
  void showEvent(QShowEvent* e);

 private:
  Ui_TranscoderOptionsDialog* ui_;
  TranscoderOptionsInterface* options_;
};

#endif  // TRANSCODEROPTIONSDIALOG_H
