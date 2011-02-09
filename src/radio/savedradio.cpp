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

#include "radiomodel.h"
#include "savedradio.h"
#include "core/mimedata.h"
#include "ui/addstreamdialog.h"
#include "ui/iconloader.h"

#include <QSettings>
#include <QMenu>

const char* SavedRadio::kServiceName = "SavedRadio";
const char* SavedRadio::kSettingsGroup = "SavedRadio";

SavedRadio::SavedRadio(RadioModel* parent)
  : RadioService(kServiceName, parent),
    context_menu_(NULL),
    root_(NULL)
{
  LoadStreams();
}

SavedRadio::~SavedRadio() {
  delete context_menu_;
}

QStandardItem* SavedRadio::CreateRootItem() {
  root_ = new QStandardItem(IconLoader::Load("document-open-remote"),
                            tr("Your radio streams"));
  root_->setData(true, RadioModel::Role_CanLazyLoad);
  return root_;
}

void SavedRadio::LazyPopulate(QStandardItem* item) {
  switch (item->data(RadioModel::Role_Type).toInt()) {
    case RadioModel::Type_Service:
      foreach (const Stream& stream, streams_)
        AddStreamToList(stream, root_);

      break;

    default:
      break;
  }
}

void SavedRadio::LoadStreams() {
  // Load saved streams
  QSettings s;
  s.beginGroup(kSettingsGroup);

  int count = s.beginReadArray("streams");
  for (int i=0 ; i<count ; ++i) {
    s.setArrayIndex(i);
    streams_ << Stream(s.value("url").toString(), s.value("name").toString());
  }
  s.endArray();
}

void SavedRadio::SaveStreams() {
  QSettings s;
  s.beginGroup(kSettingsGroup);

  int count = streams_.size();
  s.beginWriteArray("streams", count);
  for (int i=0 ; i<count ; ++i) {
    s.setArrayIndex(i);
    s.setValue("url", streams_[i].url_);
    s.setValue("name", streams_[i].name_);
  }
  s.endArray();
}

void SavedRadio::ShowContextMenu(const QModelIndex& index,
                                 const QPoint& global_pos) {
  if (!context_menu_) {
    context_menu_ = new QMenu;
    add_action_ = context_menu_->addAction(IconLoader::Load("media-playback-start"), tr("Append to current playlist"), this, SLOT(AddToPlaylist()));
    load_action_ = context_menu_->addAction(IconLoader::Load("media-playback-start"), tr("Replace current playlist"), this, SLOT(LoadToPlaylist()));
    open_in_new_playlist_ = context_menu_->addAction(IconLoader::Load("document-new"), tr("Open in new playlist"), this, SLOT(OpenInNewPlaylist()));
    context_menu_->addSeparator();
    remove_action_ = context_menu_->addAction(IconLoader::Load("list-remove"), tr("Remove"), this, SLOT(Remove()));
    edit_action_ = context_menu_->addAction(IconLoader::Load("edit-rename"), tr("Edit..."), this, SLOT(Edit()));
    context_menu_->addSeparator();
    context_menu_->addAction(IconLoader::Load("document-open-remote"), tr("Add another stream..."), this, SIGNAL(ShowAddStreamDialog()));
  }

  context_item_ = model()->itemFromIndex(index);
  const bool is_root = index.data(RadioModel::Role_Type).toInt() == RadioModel::Type_Service;

  add_action_->setEnabled(!is_root);
  load_action_->setEnabled(!is_root);
  open_in_new_playlist_->setEnabled(!is_root);
  remove_action_->setEnabled(!is_root);
  edit_action_->setEnabled(!is_root);

  context_menu_->popup(global_pos);
}

void SavedRadio::Remove() {
  streams_.removeAll(Stream(QUrl(context_item_->data(RadioModel::Role_Url).toUrl())));
  context_item_->parent()->removeRow(context_item_->row());
  SaveStreams();
}

void SavedRadio::Edit() {
  if (!edit_dialog_) {
    edit_dialog_.reset(new AddStreamDialog);
    edit_dialog_->set_save_visible(false);
  }

  edit_dialog_->set_name(context_item_->text());
  edit_dialog_->set_url(context_item_->data(RadioModel::Role_Url).toUrl());
  if (edit_dialog_->exec() == QDialog::Rejected)
    return;

  int i = streams_.indexOf(Stream(QUrl(context_item_->data(RadioModel::Role_Url).toUrl())));
  Stream& stream = streams_[i];
  stream.name_ = edit_dialog_->name();
  stream.url_ = edit_dialog_->url();

  context_item_->setText(stream.name_);
  context_item_->setData(stream.url_, RadioModel::Role_Url);

  SaveStreams();
}

void SavedRadio::AddToPlaylist() {
  AddItemToPlaylist(context_item_->index(), AddMode_Append);
}

void SavedRadio::LoadToPlaylist() {
  AddItemToPlaylist(context_item_->index(), AddMode_Replace);
}

void SavedRadio::OpenInNewPlaylist() {
  AddItemToPlaylist(context_item_->index(), AddMode_OpenInNew);
}

void SavedRadio::AddStreamToList(const Stream& stream, QStandardItem* parent) {
  QStandardItem* s = new QStandardItem(QIcon(":last.fm/icon_radio.png"), stream.name_);
  s->setData(stream.url_, RadioModel::Role_Url);
  s->setData(RadioModel::PlayBehaviour_UseSongLoader, RadioModel::Role_PlayBehaviour);
  parent->appendRow(s);
}

void SavedRadio::Add(const QUrl &url, const QString& name) {
  if (streams_.contains(Stream(url)))
    return;

  Stream stream(url, name);
  streams_ << stream;

  if (!root_->data(RadioModel::Role_CanLazyLoad).toBool()) {
    AddStreamToList(stream, root_);
  }
  SaveStreams();
}
