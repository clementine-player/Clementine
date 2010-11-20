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

#ifndef METATYPES_ENV_H
#define METATYPES_ENV_H

#include <gtest/gtest.h>

#include <QMetaType>
#include <QModelIndex>

#include "core/song.h"
#include "core/songloader.h"
#include "library/directory.h"

class MetatypesEnvironment : public ::testing::Environment {
public:
  void SetUp() {
    qRegisterMetaType<Directory>("Directory");
    qRegisterMetaType<DirectoryList>("DirectoryList");
    qRegisterMetaType<Subdirectory>("Subdirectory");
    qRegisterMetaType<SubdirectoryList>("SubdirectoryList");
    qRegisterMetaType<SongList>("SongList");
    qRegisterMetaType<QModelIndex>("QModelIndex");
    qRegisterMetaType<SongLoader::Result>("SongLoader::Result");
  }
};

#endif // RESOURCES_ENV_H
