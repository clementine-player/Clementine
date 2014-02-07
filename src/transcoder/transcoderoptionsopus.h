/* This file is part of Clementine.
   Copyright 2013, Martin Brodbeck <martin@brodbeck-online.de>

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

#ifndef TRANSCODEROPTIONSOPUS_H
#define TRANSCODEROPTIONSOPUS_H

#include "transcoderoptionsinterface.h"

class Ui_TranscoderOptionsOpus;

class TranscoderOptionsOpus : public TranscoderOptionsInterface {
 public:
  TranscoderOptionsOpus(QWidget* parent = 0);
  ~TranscoderOptionsOpus();

  void Load();
  void Save();

 private:
  static const char* kSettingsGroup;

  Ui_TranscoderOptionsOpus* ui_;
};

#endif  // TRANSCODEROPTIONSOPUS_H
