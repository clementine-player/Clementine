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

#ifndef ARTISTINFOVIEW_H
#define ARTISTINFOVIEW_H

#include "collapsibleinfopane.h"
#include "songinfobase.h"
#include "songinfofetcher.h"
#include "widgets/prettyimageview.h"

class PrettyImageView;

class QScrollArea;
class QTimeLine;
class QVBoxLayout;

class ArtistInfoView : public SongInfoBase {
  Q_OBJECT

public:
  ArtistInfoView(QWidget* parent = 0);
  ~ArtistInfoView();

protected:
  virtual void InfoResultReady (int id, const CollapsibleInfoPane::Data& data);
  bool NeedsUpdate(const Song& old_metadata, const Song& new_metadata) const;
  
  PrettyImageView* image_view;

protected slots:
  void ResultReady(int id, const SongInfoFetcher::Result& result);
};

#endif // ARTISTINFOVIEW_H

