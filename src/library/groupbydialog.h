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

#ifndef GROUPBYDIALOG_H
#define GROUPBYDIALOG_H

#include <QDialog>

#include <memory>

using std::placeholders::_1;
using std::placeholders::_2;

#include <boost/multi_index_container.hpp>
#include <boost/multi_index/member.hpp>
#include <boost/multi_index/ordered_index.hpp>

#include "librarymodel.h"

class Ui_GroupByDialog;

using boost::multi_index_container;
using boost::multi_index::indexed_by;
using boost::multi_index::ordered_unique;
using boost::multi_index::tag;
using boost::multi_index::member;

class GroupByDialog : public QDialog {
  Q_OBJECT

 public:
  GroupByDialog(QWidget *parent = 0);
  ~GroupByDialog();

 public slots:
  void LibraryGroupingChanged(const LibraryModel::Grouping& g);
  void accept();

 signals:
  void Accepted(const LibraryModel::Grouping& g);

 private slots:
  void Reset();

 private:
  struct Mapping {
    Mapping(LibraryModel::GroupBy g, int i) : group_by(g), combo_box_index(i) {}

    LibraryModel::GroupBy group_by;
    int combo_box_index;
  };

  struct tag_index {};
  struct tag_group_by {};
  typedef multi_index_container<
    Mapping,
    indexed_by<
      ordered_unique<tag<tag_index>,
        member<Mapping, int, &Mapping::combo_box_index> >,
      ordered_unique<tag<tag_group_by>,
        member<Mapping, LibraryModel::GroupBy, &Mapping::group_by> >
    >
  > MappingContainer;

  MappingContainer mapping_;
  Ui_GroupByDialog* ui_;
};

#endif // GROUPBYDIALOG_H
