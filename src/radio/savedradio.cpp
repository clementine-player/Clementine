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

#include "savedradio.h"
#include "ui/addstreamdialog.h"
#include "ui/iconloader.h"

#include <QSettings>
#include <QMenu>

const char* SavedRadio::kServiceName = "SavedRadio";
const char* SavedRadio::kSettingsGroup = "SavedRadio";

SavedRadio::SavedRadio(RadioModel* parent)
  : RadioService(kServiceName, parent),
    root_(NULL),
    context_menu_(new QMenu),
    edit_dialog_(new AddStreamDialog)
{
  add_action_ = context_menu_->addAction(IconLoader::Load("media-playback-start"), tr("Add to playlist"), this, SLOT(AddToPlaylist()));
  remove_action_ = context_menu_->addAction(IconLoader::Load("list-remove"), tr("Remove"), this, SLOT(Remove()));
  edit_action_ = context_menu_->addAction(IconLoader::Load("edit-rename"), tr("Edit..."), this, SLOT(Edit()));
  context_menu_->addSeparator();
  context_menu_->addAction(IconLoader::Load("document-open-remote"), tr("Add another stream..."), this, SIGNAL(ShowAddStreamDialog()));

  edit_dialog_->set_save_visible(false);

  LoadStreams();
}

SavedRadio::~SavedRadio() {
  delete context_menu_;
}

RadioItem* SavedRadio::CreateRootItem(RadioItem* parent) {
  root_ = new RadioItem(this, RadioItem::Type_Service, tr("Your radio streams"), parent);
  root_->icon = IconLoader::Load("document-open-remote");
  return root_;
}

void SavedRadio::LazyPopulate(RadioItem* item) {
  switch (item->type) {
    case RadioItem::Type_Service:
      foreach (const Stream& stream, streams_)
        ItemForStream(stream, root_);

      break;

    default:
      break;
  }

  item->lazy_loaded = true;
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

void SavedRadio::ShowContextMenu(RadioItem* item, const QModelIndex&,
                                 const QPoint& global_pos) {
  context_item_ = item;

  add_action_->setEnabled(item != root_);
  remove_action_->setEnabled(item != root_);
  edit_action_->setEnabled(item != root_);

  context_menu_->popup(global_pos);
}

void SavedRadio::Remove() {
  streams_.removeAll(Stream(QUrl(context_item_->key)));
  context_item_->parent->DeleteNotify(context_item_->row);
  SaveStreams();
}

void SavedRadio::Edit() {
  edit_dialog_->set_name(context_item_->display_text);
  edit_dialog_->set_url(context_item_->key);
  if (edit_dialog_->exec() == QDialog::Rejected)
    return;

  int i = streams_.indexOf(Stream(QUrl(context_item_->key)));
  Stream& stream = streams_[i];
  stream.name_ = edit_dialog_->name();
  stream.url_ = edit_dialog_->url();

  context_item_->display_text = stream.name_;
  context_item_->key = stream.url_.toString();
  context_item_->ChangedNotify();
}

void SavedRadio::AddToPlaylist() {
  emit AddItemToPlaylist(context_item_);
}

RadioItem* SavedRadio::ItemForStream(const Stream& stream, RadioItem* parent) {
  RadioItem* s = new RadioItem(this, Type_Stream, stream.url_.toString(), parent);
  if (!stream.name_.isEmpty())
    s->display_text = stream.name_;
  s->lazy_loaded = true;
  s->icon = QIcon(":last.fm/icon_radio.png");
  s->playable = true;
  s->use_song_loader = true;
  return s;
}

void SavedRadio::Add(const QUrl &url, const QString& name) {
  if (streams_.contains(Stream(url)))
    return;

  Stream stream(url, name);
  streams_ << stream;

  if (root_->lazy_loaded) {
    RadioItem* s = ItemForStream(stream, NULL);
    s->InsertNotify(root_);
  }
  SaveStreams();
}
