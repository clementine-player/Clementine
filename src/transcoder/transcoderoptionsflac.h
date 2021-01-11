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

#ifndef TRANSCODEROPTIONSFLAC_H
#define TRANSCODEROPTIONSFLAC_H

#include "transcoderoptionsinterface.h"

class Ui_TranscoderOptionsFlac;

class TranscoderOptionsFlac : public TranscoderOptionsInterface {
  Q_OBJECT

 public:
  TranscoderOptionsFlac(QWidget* parent = nullptr);
  ~TranscoderOptionsFlac();

  void Load();
  void Save();

 private slots:
  void ValueChanged(int value);

 private:
  static bool IsInStreamingSubset(int level);

  static const char* kSettingsGroup;

  Ui_TranscoderOptionsFlac* ui_;
};

#endif  // TRANSCODEROPTIONSFLAC_H
