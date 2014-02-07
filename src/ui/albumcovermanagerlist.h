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

#ifndef ALBUMCOVERMANAGERLIST_H
#define ALBUMCOVERMANAGERLIST_H

#include <QListWidget>

class AlbumCoverManager;

class AlbumCoverManagerList : public QListWidget {
  Q_OBJECT
 public:
  AlbumCoverManagerList(QWidget* parent = 0);

  void set_cover_manager(AlbumCoverManager* manager) { manager_ = manager; }

 protected:
  QMimeData* mimeData(const QList<QListWidgetItem*> items) const;
  void dropEvent(QDropEvent* event);

 private:
  AlbumCoverManager* manager_;
};

#endif  // ALBUMCOVERMANAGERLIST_H
