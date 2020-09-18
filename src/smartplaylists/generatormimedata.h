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

#ifndef GENERATORMIMEDATA_H
#define GENERATORMIMEDATA_H

#include <QMimeData>

#include "core/mimedata.h"
#include "generator_fwd.h"

namespace smart_playlists {

class GeneratorMimeData : public MimeData {
  Q_OBJECT

 public:
  GeneratorMimeData(GeneratorPtr generator) : generator_(generator) {}

  GeneratorPtr generator_;
};

}  // namespace smart_playlists

#endif  // GENERATORMIMEDATA_H
