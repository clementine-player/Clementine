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

#ifndef SMARTPLAYLISTSEARCHPREVIEW_H
#define SMARTPLAYLISTSEARCHPREVIEW_H

#include "smartplaylistsearch.h"

#include <QWidget>

#include <boost/scoped_ptr.hpp>

class LibraryBackend;
class QueryPlaylistGenerator;
class Ui_SmartPlaylistSearchPreview;

class SmartPlaylistSearchPreview : public QWidget {
  Q_OBJECT

public:
  SmartPlaylistSearchPreview(QWidget *parent = 0);
  ~SmartPlaylistSearchPreview();

  void set_library(LibraryBackend* backend);

  void Update(const SmartPlaylistSearch& search);

private:
  void RunSearch(const SmartPlaylistSearch& search);

private slots:
  void SearchFinished();

private:
  Ui_SmartPlaylistSearchPreview* ui_;
  QList<SmartPlaylistSearchTerm::Field> fields_;

  LibraryBackend* backend_;

  SmartPlaylistSearch pending_search_;
  boost::scoped_ptr<QueryPlaylistGenerator> generator_;
};

#endif // SMARTPLAYLISTSEARCHPREVIEW_H
