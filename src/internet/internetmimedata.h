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

#ifndef INTERNETMIMEDATA_H
#define INTERNETMIMEDATA_H

#include "core/mimedata.h"

#include <QMimeData>
#include <QModelIndexList>

class InternetModel;

class InternetMimeData : public MimeData {
  Q_OBJECT

 public:
  InternetMimeData(const InternetModel* _model) : model(_model) {}

  const InternetModel* model;
  QModelIndexList indexes;
};

#endif  // INTERNETMIMEDATA_H
