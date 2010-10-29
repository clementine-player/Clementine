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

#ifndef PLAYLISTGENERATOR_H
#define PLAYLISTGENERATOR_H

#include "playlist/playlistitem.h"

#include <boost/enable_shared_from_this.hpp>
#include <boost/shared_ptr.hpp>

class LibraryBackend;

class QSettings;

class PlaylistGenerator : public QObject, public boost::enable_shared_from_this<PlaylistGenerator> {
  Q_OBJECT

public:
  PlaylistGenerator();
  virtual ~PlaylistGenerator() {}

  static const int kDefaultLimit;
  static boost::shared_ptr<PlaylistGenerator> Create(const QString& type);

  void set_library(LibraryBackend* backend) { backend_ = backend; }

  QString name() const { return name_; }

  virtual void Load(const QSettings& s) {}
  virtual PlaylistItemList Generate() = 0;

signals:
  void Error(const QString& message);

protected:
  LibraryBackend* backend_;

  QString name_;
};

#include "playlistgenerator_fwd.h"

#endif // PLAYLISTGENERATOR_H
