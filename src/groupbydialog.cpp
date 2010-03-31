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

#include "groupbydialog.h"

#include <QPushButton>

GroupByDialog::GroupByDialog(QWidget *parent)
  : QDialog(parent)
{
  ui_.setupUi(this);
  Reset();

  mapping_.insert(Mapping(Library::GroupBy_None, 0));
  mapping_.insert(Mapping(Library::GroupBy_Album, 1));
  mapping_.insert(Mapping(Library::GroupBy_Artist, 2));
  mapping_.insert(Mapping(Library::GroupBy_Composer, 3));
  mapping_.insert(Mapping(Library::GroupBy_Genre, 4));
  mapping_.insert(Mapping(Library::GroupBy_Year, 5));
  mapping_.insert(Mapping(Library::GroupBy_YearAlbum, 6));

  connect(ui_.button_box->button(QDialogButtonBox::Reset), SIGNAL(clicked()),
          SLOT(Reset()));
}

void GroupByDialog::Reset() {
  ui_.first->setCurrentIndex(2); // Artist
  ui_.second->setCurrentIndex(1); // Album
  ui_.third->setCurrentIndex(0); // None
}

void GroupByDialog::accept() {
  emit Accepted(Library::Grouping(
      mapping_.get<tag_index>().find(ui_.first->currentIndex())->group_by,
      mapping_.get<tag_index>().find(ui_.second->currentIndex())->group_by,
      mapping_.get<tag_index>().find(ui_.third->currentIndex())->group_by));
  QDialog::accept();
}

void GroupByDialog::LibraryGroupingChanged(const Library::Grouping& g) {
  ui_.first->setCurrentIndex(mapping_.get<tag_group_by>().find(g[0])->combo_box_index);
  ui_.second->setCurrentIndex(mapping_.get<tag_group_by>().find(g[1])->combo_box_index);
  ui_.third->setCurrentIndex(mapping_.get<tag_group_by>().find(g[2])->combo_box_index);
}
