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

#include "songinfotextview.h"
#include "taglyricsinfoprovider.h"
#include "core/logging.h"

void TagLyricsInfoProvider::FetchInfo(int id, const Song& metadata) {
  QString lyrics;
  lyrics = metadata.lyrics();

  if (!lyrics.isEmpty()) {
    CollapsibleInfoPane::Data data;
    data.id_ = "tag/lyrics";
    data.title_ = tr("Lyrics from the tag");
    data.type_ = CollapsibleInfoPane::Data::Type_Lyrics;

    SongInfoTextView* editor = new SongInfoTextView;
    editor->setPlainText(lyrics);
    data.contents_ = editor;

    emit InfoReady(id, data);
  }
  emit Finished(id);
}
