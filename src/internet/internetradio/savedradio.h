/* This file is part of Clementine.
   Copyright 2010-2012, David Sansome <me@davidsansome.com>
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

#ifndef INTERNET_INTERNETRADIO_SAVEDRADIO_H_
#define INTERNET_INTERNETRADIO_SAVEDRADIO_H_

#include <memory>

#include "internet/core/internetservice.h"

class QMenu;

class AddStreamDialog;

class SavedRadio : public InternetService {
  Q_OBJECT

 public:
  SavedRadio(Application* app, InternetModel* parent);
  ~SavedRadio();

  enum ItemType {
    Type_Stream = 2000,
  };

  struct Stream {
    explicit Stream(const QUrl& url, const QString& name = QString(),
                    const QUrl& url_logo = QUrl())
        : url_(url), name_(name), url_logo_(url_logo) {}

    // For QList::contains
    bool operator==(const Stream& other) const { return url_ == other.url_; }

    QUrl url_;
    QString name_;
    QUrl url_logo_;
  };
  typedef QList<Stream> StreamList;

  static const char* kServiceName;
  static const char* kSettingsGroup;

  QStandardItem* CreateRootItem();
  void LazyPopulate(QStandardItem* item);

  void ShowContextMenu(const QPoint& global_pos);

  void Add(const QUrl& url, const QString& name = QString(),
           const QUrl& url_logo = QUrl());

  const StreamList& Streams() const { return streams_; }

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
  QStandardItem* root_;

  QAction* remove_action_;
  QAction* edit_action_;

  StreamList streams_;

  std::unique_ptr<AddStreamDialog> edit_dialog_;
};

#endif  // INTERNET_INTERNETRADIO_SAVEDRADIO_H_
