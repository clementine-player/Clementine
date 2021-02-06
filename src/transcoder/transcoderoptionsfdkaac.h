/* This file is part of Clementine.
   Copyright 2021, Jim Broadus <jbroadus@gmail.com>

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

#ifndef TRANSCODEROPTIONSFDKAAC_H
#define TRANSCODEROPTIONSFDKAAC_H

#include "transcoderoptionsinterface.h"

class Ui_TranscoderOptionsFDKAAC;

class TranscoderOptionsFDKAAC : public TranscoderOptionsInterface {
  Q_OBJECT

 public:
  TranscoderOptionsFDKAAC(QWidget* parent = nullptr);
  ~TranscoderOptionsFDKAAC();

  void Load();
  void Save();

 private slots:
  void TargetBitrateToggle(bool checked);

 private:
  static const char* kSettingsGroup;

  Ui_TranscoderOptionsFDKAAC* ui_;
};

#endif  // TRANSCODEROPTIONSFDKAAC_H
