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

#include "groupbydialog.h"
#include "ui_groupbydialog.h"

#include <QPushButton>

// boost::multi_index still relies on these being in the global namespace.
using std::placeholders::_1;
using std::placeholders::_2;

#include <boost/multi_index_container.hpp>
#include <boost/multi_index/member.hpp>
#include <boost/multi_index/ordered_index.hpp>

using boost::multi_index_container;
using boost::multi_index::indexed_by;
using boost::multi_index::ordered_unique;
using boost::multi_index::tag;
using boost::multi_index::member;

namespace {

struct Mapping {
  Mapping(LibraryModel::GroupBy g, int i) : group_by(g), combo_box_index(i) {}

  LibraryModel::GroupBy group_by;
  int combo_box_index;
};

struct tag_index {};
struct tag_group_by {};

}  // namespace

class GroupByDialogPrivate {
 private:
  typedef multi_index_container<
      Mapping,
      indexed_by<
          ordered_unique<tag<tag_index>,
                         member<Mapping, int, &Mapping::combo_box_index> >,
          ordered_unique<tag<tag_group_by>,
                         member<Mapping, LibraryModel::GroupBy,
                                &Mapping::group_by> > > > MappingContainer;

 public:
  MappingContainer mapping_;
};

GroupByDialog::GroupByDialog(QWidget* parent)
    : QDialog(parent), ui_(new Ui_GroupByDialog), p_(new GroupByDialogPrivate) {
  ui_->setupUi(this);
  Reset();

  p_->mapping_.insert(Mapping(LibraryModel::GroupBy_None, 0));
  p_->mapping_.insert(Mapping(LibraryModel::GroupBy_Album, 1));
  p_->mapping_.insert(Mapping(LibraryModel::GroupBy_Artist, 2));
  p_->mapping_.insert(Mapping(LibraryModel::GroupBy_AlbumArtist, 3));
  p_->mapping_.insert(Mapping(LibraryModel::GroupBy_Composer, 4));
  p_->mapping_.insert(Mapping(LibraryModel::GroupBy_FileType, 5));
  p_->mapping_.insert(Mapping(LibraryModel::GroupBy_Genre, 6));
  p_->mapping_.insert(Mapping(LibraryModel::GroupBy_Year, 7));
  p_->mapping_.insert(Mapping(LibraryModel::GroupBy_YearAlbum, 8));
  p_->mapping_.insert(Mapping(LibraryModel::GroupBy_Bitrate, 9));
  // p_->mapping_.insert(Mapping(LibraryModel::GroupBy_Performer, 10));
  p_->mapping_.insert(Mapping(LibraryModel::GroupBy_Disc, 10));
  p_->mapping_.insert(Mapping(LibraryModel::GroupBy_Grouping, 11));

  connect(ui_->button_box->button(QDialogButtonBox::Reset), SIGNAL(clicked()),
          SLOT(Reset()));

  resize(sizeHint());
}

GroupByDialog::~GroupByDialog() {}

void GroupByDialog::Reset() {
  ui_->first->setCurrentIndex(2);   // Artist
  ui_->second->setCurrentIndex(1);  // Album
  ui_->third->setCurrentIndex(0);   // None
}

void GroupByDialog::accept() {
  emit Accepted(LibraryModel::Grouping(
      p_->mapping_.get<tag_index>().find(ui_->first->currentIndex())->group_by,
      p_->mapping_.get<tag_index>().find(ui_->second->currentIndex())->group_by,
      p_->mapping_.get<tag_index>()
          .find(ui_->third->currentIndex())
          ->group_by));
  QDialog::accept();
}

void GroupByDialog::LibraryGroupingChanged(const LibraryModel::Grouping& g) {
  ui_->first->setCurrentIndex(
      p_->mapping_.get<tag_group_by>().find(g[0])->combo_box_index);
  ui_->second->setCurrentIndex(
      p_->mapping_.get<tag_group_by>().find(g[1])->combo_box_index);
  ui_->third->setCurrentIndex(
      p_->mapping_.get<tag_group_by>().find(g[2])->combo_box_index);
}
