/* This file is part of Clementine.
   Copyright 2014, Krzysztof Sobiecki <sobkas@gmail.com>
   Copyright 2014, Maltsev Vlad <shedwardx@gmail.com>

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

#ifndef INTERNET_VK_VKSEARCHDIALOG_H_
#define INTERNET_VK_VKSEARCHDIALOG_H_

#include <QDialog>
#include <QTreeWidget>
#include <QTimer>

#include "vkservice.h"

namespace Ui {
class VkSearchDialog;
}

class VkSearchDialog : public QDialog {
  Q_OBJECT

 public:
  explicit VkSearchDialog(VkService* service, QWidget* parent = 0);
  ~VkSearchDialog();
  MusicOwner found() const;

 signals:
  void Find(const QString& query);

 public slots:
  void ReceiveResults(const SearchID& id, const MusicOwnerList& owners);

 protected:
  void showEvent(QShowEvent*);

 private slots:
  void selectionChanged();
  void suggest();
  void selected();

 private:
  bool eventFilter(QObject* obj, QEvent* ev);
  QTreeWidgetItem* createItem(const MusicOwner& own);

  Ui::VkSearchDialog* ui;
  MusicOwner selected_;
  VkService* service_;
  SearchID last_search_;
  QTreeWidget* popup;
  QTimer* timer;
};

#endif  // INTERNET_VK_VKSEARCHDIALOG_H_
