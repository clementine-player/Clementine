/* This file is part of Clementine.
   Copyright 2009-2010, David Sansome <davidsansome@gmail.com>
   Copyright 2011, Tyler Rhodes <tyler.s.rhodes@gmail.com>
   Copyright 2014, Krzysztof Sobiecki <sobkas@gmail.com>
   Copyright 2014, John Maguire <john.maguire@gmail.com>

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

#ifndef INTERNET_CORE_INTERNETVIEW_H_
#define INTERNET_CORE_INTERNETVIEW_H_

#include "widgets/autoexpandingtreeview.h"

class InternetView : public AutoExpandingTreeView {
  Q_OBJECT

 public:
  explicit InternetView(QWidget* parent = nullptr);

  // QWidget
  void contextMenuEvent(QContextMenuEvent* e);

  // QTreeView
  void setModel(QAbstractItemModel* model);
};

#endif  // INTERNET_CORE_INTERNETVIEW_H_
