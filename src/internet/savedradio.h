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

#include <memory>

#include "internetservice.h"

class QMenu;

class AddStreamDialog;

class SavedRadio : public InternetService {
  Q_OBJECT

 public:
  SavedRadio(Application* app, InternetModel* parent);
  ~SavedRadio();

  enum ItemType { Type_Stream = 2000, };

  struct Stream {
    Stream(const QUrl& url, const QString& name = QString())
        : url_(url), name_(name) {}

    // For QList::contains
    bool operator==(const Stream& other) const { return url_ == other.url_; }

    QUrl url_;
    QString name_;
  };
  typedef QList<Stream> StreamList;

  static const char* kServiceName;
  static const char* kSettingsGroup;

  QStandardItem* CreateRootItem();
  void LazyPopulate(QStandardItem* item);

  void ShowContextMenu(const QPoint& global_pos);

  void Add(const QUrl& url, const QString& name = QString());

  StreamList Streams() const { return streams_; }

signals:
  void ShowAddStreamDialog();
  void StreamsChanged();

 private slots:
  void Remove();
  void Edit();

 private:
  void LoadStreams();
  void SaveStreams();
  void AddStreamToList(const Stream& stream, QStandardItem* parent);

 private:
  QMenu* context_menu_;
  QStandardItem* root_;

  QAction* remove_action_;
  QAction* edit_action_;

  StreamList streams_;

  std::unique_ptr<AddStreamDialog> edit_dialog_;
};

#endif  // SAVEDRADIO_H
