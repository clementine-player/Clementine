/* This file is part of Clementine.
   Copyright 2013, David Sansome <me@davidsansome.com>

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

#include "playlistlistview.h"

#include <QPainter>

PlaylistListView::PlaylistListView(QWidget* parent)
    : AutoExpandingTreeView(parent) {}

void PlaylistListView::paintEvent(QPaintEvent* event) {
  if (model()->rowCount() <= 0) {
    QPainter p(viewport());
    QRect rect(viewport()->rect());

    p.setPen(palette().color(QPalette::Disabled, QPalette::Text));

    QFont bold_font;
    bold_font.setBold(true);
    p.setFont(bold_font);

    p.drawText(rect, Qt::AlignHCenter | Qt::TextWordWrap,
               tr("\n\n"
                  "You can favorite playlists by clicking the star icon next "
                  "to a playlist name\n\n"
                  "Favorited playlists will be saved here"));
  } else {
    AutoExpandingTreeView::paintEvent(event);
  }
}
