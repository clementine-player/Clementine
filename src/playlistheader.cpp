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
  hide_action_ = menu_->addAction("Hide...", this, SLOT(HideCurrent()));
  QAction* show_action = menu_->addAction("Show section");
  show_action->setMenu(show_menu_);

  connect(show_mapper_, SIGNAL(mapped(int)), SLOT(ToggleVisible(int)));
}

void PlaylistHeader::contextMenuEvent(QContextMenuEvent* e) {
  menu_section_ = logicalIndexAt(e->pos());

  if (menu_section_ == -1)
    hide_action_->setVisible(false);
  else {
    hide_action_->setVisible(true);

    QString title(model()->headerData(menu_section_, Qt::Horizontal).toString());
    hide_action_->setText("Hide " + title);
  }

  show_menu_->clear();
  for (int i=0 ; i<model()->columnCount() ; ++i) {
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
}


