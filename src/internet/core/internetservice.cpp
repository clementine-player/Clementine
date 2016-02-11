/* This file is part of Clementine.
   Copyright 2010-2012, David Sansome <me@davidsansome.com>
   Copyright 2011, Tyler Rhodes <tyler.s.rhodes@gmail.com>
   Copyright 2011, Paweł Bara <keirangtp@gmail.com>
   Copyright 2012, Arnaud Bienner <arnaud.bienner@gmail.com>
   Copyright 2014, John Maguire <john.maguire@gmail.com>
   Copyright 2014, Krzysztof Sobiecki <sobkas@gmail.com>
   Copyright 2016, David Ó Laıġeanáın <david.lynam@redbrick.dcu.ie>

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

#include "internet/core/internetservice.h"

#include <QApplication>
#include <QClipboard>
#include <QIcon>
#include <QMenu>
#include <QMessageBox>
#include <QPushButton>
#include <QStandardItem>

#include "internet/core/internetmodel.h"
#include "core/logging.h"
#include "core/mergedproxymodel.h"
#include "core/mimedata.h"
#include "ui/iconloader.h"

InternetService::InternetService(const QString& name, Application* app,
                                 InternetModel* model, QObject* parent)
    : QObject(parent),
      app_(app),
      model_(model),
      name_(name),
      append_to_playlist_([&]() {
        QAction* action = new QAction(
            IconLoader::Load("media-playback-start", IconLoader::Base),
            tr("Append to current playlist"), nullptr);
        connect(action, SIGNAL(triggered()), this, SLOT(AppendToPlaylist()));
        return action;
      }),
      replace_playlist_([&]() {
        QAction* action = new QAction(
            IconLoader::Load("media-playback-start", IconLoader::Base),
            tr("Replace current playlist"), nullptr);
        connect(action, SIGNAL(triggered()), this, SLOT(ReplacePlaylist()));
        return action;
      }),
      open_in_new_playlist_([&]() {
        QAction* action = new QAction(
            IconLoader::Load("document-new", IconLoader::Base),
            tr("Open in new playlist"), nullptr);
        connect(action, SIGNAL(triggered()), this, SLOT(OpenInNewPlaylist()));
        return action;
      }),
      copy_selected_playable_item_url_([&]() {
        QAction* action = new QAction(
            IconLoader::Load("edit-copy", IconLoader::Base),
            tr("Copy URL to clipboard"), nullptr);
        connect(action, SIGNAL(triggered()), this, SLOT(CopySelectedPlayableItemURL()));
        return action;
      }),
      separator_([]() {
        QAction* action = new QAction(nullptr);
        action->setSeparator(true);
        return action;
      }) {}

void InternetService::ShowUrlBox(const QString& title, const QString& url) {
  QMessageBox url_box;
  url_box.setWindowTitle(title);
  url_box.setWindowIcon(QIcon(":/icon.png"));
  url_box.setText(url);
  url_box.setStandardButtons(QMessageBox::Ok);
  QPushButton* copy_to_clipboard_button =
      url_box.addButton(tr("Copy to clipboard"), QMessageBox::ActionRole);

  url_box.exec();

  if (url_box.clickedButton() == copy_to_clipboard_button) {
    QApplication::clipboard()->setText(url);
  }
}

QList<QAction*> InternetService::GetPlaylistActions() {
  return QList<QAction*>() << GetAppendToPlaylistAction()
                           << GetReplacePlaylistAction()
                           << GetOpenInNewPlaylistAction() << separator_.get();
}

QAction* InternetService::GetAppendToPlaylistAction() {
  return append_to_playlist_.get();
}

QAction* InternetService::GetReplacePlaylistAction() {
  return replace_playlist_.get();
}

QAction* InternetService::GetOpenInNewPlaylistAction() {
  return open_in_new_playlist_.get();
}

QAction* InternetService::GetCopySelectedPlayableItemURLAction() {
  return copy_selected_playable_item_url_.get();
}

void InternetService::AddItemToPlaylist(const QModelIndex& index,
                                        AddMode add_mode) {
  AddItemsToPlaylist(QModelIndexList() << index, add_mode);
}

void InternetService::AddItemsToPlaylist(const QModelIndexList& indexes,
                                         AddMode add_mode) {
  QMimeData* data = model()->merged_model()->mimeData(
      model()->merged_model()->mapFromSource(indexes));
  if (MimeData* mime_data = qobject_cast<MimeData*>(data)) {
    mime_data->clear_first_ = add_mode == AddMode_Replace;
    mime_data->open_in_new_playlist_ = add_mode == AddMode_OpenInNew;
  }
  emit AddToPlaylistSignal(data);
}

void InternetService::AppendToPlaylist() {
  AddItemsToPlaylist(model()->selected_indexes(), AddMode_Append);
}

void InternetService::ReplacePlaylist() {
  AddItemsToPlaylist(model()->selected_indexes(), AddMode_Replace);
}

void InternetService::OpenInNewPlaylist() {
  AddItemsToPlaylist(model()->selected_indexes(), AddMode_OpenInNew);
}

void InternetService::CopySelectedPlayableItemURL() const {
  if (selected_playable_item_url_.isEmpty()) return;

  QString url = selected_playable_item_url_.toEncoded();

  qLog(Debug) << "Playable item URL: " << url;
  ShowUrlBox(tr("Copy URL"), url);
}

QStandardItem* InternetService::CreateSongItem(const Song& song) {
  QStandardItem* item = new QStandardItem(song.PrettyTitleWithArtist());
  item->setData(InternetModel::Type_Track, InternetModel::Role_Type);
  item->setData(QVariant::fromValue(song), InternetModel::Role_SongMetadata);
  item->setData(InternetModel::PlayBehaviour_SingleItem,
                InternetModel::Role_PlayBehaviour);
  item->setData(song.url(), InternetModel::Role_Url);

  return item;
}
