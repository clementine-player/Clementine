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

#ifndef TRANSCODEROPTIONSERROR_H
#define TRANSCODEROPTIONSERROR_H

#include "transcoderoptionsinterface.h"

class Ui_TranscoderOptionsError;

class TranscoderOptionsError : public TranscoderOptionsInterface {
 public:
  TranscoderOptionsError(const QString& mime_type, const QString& element,
                         QWidget* parent = nullptr);
  ~TranscoderOptionsError();

  void Load(){};
  void Save(){};

 private:
  Ui_TranscoderOptionsError* ui_;
};

#endif  // TRANSCODEROPTIONSERROR_H
