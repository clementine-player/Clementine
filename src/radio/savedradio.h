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

#ifndef SAVEDRADIO_H
#define SAVEDRADIO_H

#include "radioservice.h"

#include <boost/scoped_ptr.hpp>

class QMenu;

class AddStreamDialog;

class SavedRadio : public RadioService {
  Q_OBJECT

 public:
  SavedRadio(RadioModel* parent);
  ~SavedRadio();

  enum ItemType {
    Type_Stream = 2000,
  };

  static const char* kServiceName;
  static const char* kSettingsGroup;

  QStandardItem* CreateRootItem();
  void LazyPopulate(QStandardItem* item);

  void ShowContextMenu(const QModelIndex& index, const QPoint& global_pos);

  void Add(const QUrl& url, const QString& name = QString());

 signals:
  void ShowAddStreamDialog();

 private slots:
  void AddToPlaylist();
  void LoadToPlaylist();
  void Remove();
  void Edit();

 private:
  struct Stream {
    Stream(const QUrl& url, const QString& name = QString())
      : url_(url), name_(name) {}

    // For QList::contains
    bool operator ==(const Stream& other) const { return url_ == other.url_; }

    QUrl url_;
    QString name_;
  };

  void LoadStreams();
  void SaveStreams();
  void AddStreamToList(const Stream& stream, QStandardItem* parent);

 private:
  QMenu* context_menu_;
  QStandardItem* context_item_;
  QStandardItem* root_;

  QAction* add_action_;
  QAction* load_action_;
  QAction* remove_action_;
  QAction* edit_action_;

  QList<Stream> streams_;

  boost::scoped_ptr<AddStreamDialog> edit_dialog_;
};

#endif // SAVEDRADIO_H
