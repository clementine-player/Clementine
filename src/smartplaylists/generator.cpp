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

#include "generator.h"
#include "querygenerator.h"
#include "core/logging.h"
#include "internet/jamendo/jamendodynamicplaylist.h"

#include <QSettings>

namespace smart_playlists {

const int Generator::kDefaultLimit = 20;
const int Generator::kDefaultDynamicHistory = 5;
const int Generator::kDefaultDynamicFuture = 15;

Generator::Generator() : QObject(nullptr), backend_(nullptr) {}

GeneratorPtr Generator::Create(const QString& type) {
  if (type == "Query")
    return GeneratorPtr(new QueryGenerator);
  else if (type == "Jamendo")
    return GeneratorPtr(new JamendoDynamicPlaylist);

  qLog(Warning) << "Invalid playlist generator type:" << type;
  return GeneratorPtr();
}

}  // namespace
