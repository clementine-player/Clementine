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

#include "playlistheader.h"

#include <QtDebug>
#include <QContextMenuEvent>
#include <QMenu>
#include <QSignalMapper>

PlaylistHeader::PlaylistHeader(Qt::Orientation orientation, QWidget* parent)
    : QHeaderView(orientation, parent),
      menu_(new QMenu(this)),
      show_menu_(new QMenu(this)),
      show_mapper_(new QSignalMapper(this))
{
  hide_action_ = menu_->addAction(tr("Hide..."), this, SLOT(HideCurrent()));
  QAction* show_action = menu_->addAction(tr("Show section"));
  show_action->setMenu(show_menu_);

  connect(show_mapper_, SIGNAL(mapped(int)), SLOT(ToggleVisible(int)));
}

void PlaylistHeader::contextMenuEvent(QContextMenuEvent* e) {
  menu_section_ = logicalIndexAt(e->pos());

  if (menu_section_ == -1 || (
        menu_section_ == logicalIndex(0) && logicalIndex(1) == -1))
    hide_action_->setVisible(false);
  else {
    hide_action_->setVisible(true);

    QString title(model()->headerData(menu_section_, Qt::Horizontal).toString());
    hide_action_->setText(tr("Hide %1").arg(title));
  }

  show_menu_->clear();
  for (int i=0 ; i<count() ; ++i) {
    AddColumnAction(i);
  }

  menu_->popup(e->globalPos());
}

void PlaylistHeader::AddColumnAction(int index) {
  QString title(model()->headerData(index, Qt::Horizontal).toString());

  QAction* action = show_menu_->addAction(title, show_mapper_, SLOT(map()));
  action->setCheckable(true);
  action->setChecked(!isSectionHidden(index));

  show_mapper_->setMapping(action, index);
}

void PlaylistHeader::HideCurrent() {
  if (menu_section_ == -1)
    return;

  setSectionHidden(menu_section_, true);
}

void PlaylistHeader::ToggleVisible(int section) {
  setSectionHidden(section, !isSectionHidden(section));

  static const int kMinSectionSize = 80;

  if (!isSectionHidden(section) && sectionSize(section) < kMinSectionSize)
    resizeSection(section, kMinSectionSize);
}
