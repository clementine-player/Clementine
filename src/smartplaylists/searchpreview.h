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

#ifndef SMARTPLAYLISTSEARCHPREVIEW_H
#define SMARTPLAYLISTSEARCHPREVIEW_H

#include <QFuture>
#include <QWidget>

#include "search.h"
#include "smartplaylists/generator_fwd.h"

class Application;
class LibraryBackend;
class Playlist;
class Ui_SmartPlaylistSearchPreview;

namespace smart_playlists {

class SearchPreview : public QWidget {
  Q_OBJECT

 public:
  SearchPreview(QWidget* parent = nullptr);
  ~SearchPreview();

  void set_application(Application* app);
  void set_library(LibraryBackend* backend);

  void Update(const Search& search);

 protected:
  void showEvent(QShowEvent*);

 private:
  void RunSearch(const Search& search);

 private slots:
  void SearchFinished(QFuture<PlaylistItemList> future);

 private:
  Ui_SmartPlaylistSearchPreview* ui_;
  QList<SearchTerm::Field> fields_;

  LibraryBackend* backend_;
  Playlist* model_;

  Search pending_search_;
  Search last_search_;
  GeneratorPtr generator_;
};

}  // namespace smart_playlists

#endif  // SMARTPLAYLISTSEARCHPREVIEW_H
