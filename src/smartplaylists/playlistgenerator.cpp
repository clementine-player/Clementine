/* This file is part of Clementine.

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

#include "queryplaylistgenerator.h"
#include "playlistgenerator.h"

#include <QSettings>

const int PlaylistGenerator::kDefaultLimit = 20;

PlaylistGenerator::PlaylistGenerator()
  : QObject(NULL),
    backend_(NULL)
{
}

PlaylistGeneratorPtr PlaylistGenerator::Create(const QString& type) {
  if (type == "Query")
    return PlaylistGeneratorPtr(new QueryPlaylistGenerator);

  qWarning() << "Invalid playlist generator type:" << type;
  return PlaylistGeneratorPtr();
}

