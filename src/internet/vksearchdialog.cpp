/* This file is part of Clementine.
   Copyright 2013, Vlad Maltsev <shedwardx@gmail.com>

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

#include <QKeyEvent>
#include <QPushButton>

#include "vkservice.h"

#include "vksearchdialog.h"
#include "ui_vksearchdialog.h"

VkSearchDialog::VkSearchDialog(VkService* service, QWidget* parent)
  : QDialog(parent),
    ui(new Ui::VkSearchDialog),
    service_(service),
    last_search_(SearchID(SearchID::UserOrGroup)) {
  ui->setupUi(this);

  timer = new QTimer(this);
  timer->setSingleShot(true);
  timer->setInterval(100);
  connect(timer, SIGNAL(timeout()), SLOT(suggest()));
  connect(ui->searchLine, SIGNAL(textChanged(QString)), timer, SLOT(start()));

  popup = new QTreeWidget(this);
  popup->setColumnCount(2);
  popup->setUniformRowHeights(true);
  popup->setRootIsDecorated(false);
  popup->setEditTriggers(QTreeWidget::NoEditTriggers);
  popup->setSelectionBehavior(QTreeWidget::SelectRows);
  popup->setFrameStyle(QFrame::Box | QFrame::Plain);
  popup->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

  popup->header()->hide();
  popup->installEventFilter(this);
  popup->setMouseTracking(true);

  popup->setWindowFlags(Qt::Popup);
  popup->setFocusPolicy(Qt::NoFocus);
  popup->setFocusProxy(parent);

  connect(popup, SIGNAL(itemSelectionChanged()),
          SLOT(selectionChanged()));
  connect(popup, SIGNAL(clicked(QModelIndex)),
          SLOT(selected()));

  connect(this, SIGNAL(Find(QString)), service_, SLOT(FindUserOrGroup(QString)));
  connect(service_, SIGNAL(UserOrGroupSearchResult(SearchID, MusicOwnerList)),
          this, SLOT(ReceiveResults(SearchID, MusicOwnerList)));

  ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(false);
}

VkSearchDialog::~VkSearchDialog() {
  delete ui;
  delete popup;
}

void VkSearchDialog::suggest() {
  emit Find(ui->searchLine->text());
}

void VkSearchDialog::selected() {
  selectionChanged();
  ui->searchLine->setText(selected_.name());
  timer->stop();
  popup->hide();
}

void VkSearchDialog::ReceiveResults(const SearchID& id, const MusicOwnerList& owners) {
  if (id.id() > last_search_.id()) {
    popup->setUpdatesEnabled(false);
    popup->clear();

    if (owners.count() > 0) {
      for (const MusicOwner &own : owners) {
        popup->addTopLevelItem(createItem(own));
      }
    } else {
      popup->addTopLevelItem(new QTreeWidgetItem(QStringList(tr("Nothig found"))));
    }

    popup->setCurrentItem(popup->topLevelItem(0));

    popup->resizeColumnToContents(0);
    int ch = popup->columnWidth(0);
    if (ch > 0.8*ui->searchLine->width()) {
      popup->setColumnWidth(0, qRound(0.8*ui->searchLine->width()));
    }
    popup->resizeColumnToContents(1);
    popup->adjustSize();
    popup->setUpdatesEnabled(true);

    int elems = (owners.count() > 0) ? owners.count() : 1;
    int h = popup->sizeHintForRow(0) * qMin(10, elems) + 3;

    popup->resize(ui->searchLine->width(), h);

    QPoint relpos = ui->searchLine->pos() + QPoint(0, ui->searchLine->height());
    popup->move(mapToGlobal(relpos));
    popup->setFocus();
    popup->show();
  }
}

void VkSearchDialog::showEvent(QShowEvent*) {
  ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(false);
  selected_ = MusicOwner();
  ui->searchLine->clear();
}

void VkSearchDialog::selectionChanged() {
  if (popup->selectedItems().size() > 0) {
    QTreeWidgetItem* sel = popup->selectedItems()[0];
    selected_ = sel->data(0, Qt::UserRole).value<MusicOwner>();
    ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(selected_.id() != 0);
    ui->buttonBox->button(QDialogButtonBox::Ok)->setDefault(true);
  }
}

MusicOwner VkSearchDialog::found() const {
  return selected_;
}

bool VkSearchDialog::eventFilter(QObject* obj, QEvent* ev) {
  if (obj != popup)
    return false;

  if (ev->type() == QEvent::MouseButtonPress) {
    popup->hide();
    ui->searchLine->setFocus();
    return true;
  }

  if (ev->type() == QEvent::KeyPress) {
    bool consumed = false;
    int key = static_cast<QKeyEvent*>(ev)->key();

    switch (key) {
    case Qt::Key_Enter:
    case Qt::Key_Return:
      selected();
      break;

    case Qt::Key_Escape:
      ui->searchLine->setFocus();
      popup->hide();
      consumed = true;
      break;

    case Qt::Key_Up:
    case Qt::Key_Down:
    case Qt::Key_Home:
    case Qt::Key_End:
    case Qt::Key_PageUp:
    case Qt::Key_PageDown:
      break;

    default:
      ui->searchLine->setFocus();
      ui->searchLine->event(ev);
      break;
    }

    return consumed;
  }

  return false;
}

QTreeWidgetItem* VkSearchDialog::createItem(const MusicOwner& own) {
  QTreeWidgetItem* item = new QTreeWidgetItem(popup);
  item->setText(0, own.name());
  if (own.id() > 0) {
    item->setIcon(0, QIcon(":vk/user.png"));
  } else {
    item->setIcon(0, QIcon(":vk/group.png"));
  }
  item->setData(0, Qt::UserRole, QVariant::fromValue(own));
  item->setText(1, QString::number(own.song_count()));
  item->setTextAlignment(1, Qt::AlignRight);
  item->setTextColor(1, palette().color(QPalette::WindowText));
  return item;
}
