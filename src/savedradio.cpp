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

#include <QSettings>
#include <QMenu>

const char* SavedRadio::kServiceName = "SavedRadio";
const char* SavedRadio::kSettingsGroup = "SavedRadio";

SavedRadio::SavedRadio(RadioModel* parent)
  : RadioService(kServiceName, parent),
    root_(NULL),
    context_menu_(new QMenu)
{
  add_action_ = context_menu_->addAction(QIcon(":media-playback-start.png"), tr("Add to playlist"), this, SLOT(AddToPlaylist()));
  remove_action_ = context_menu_->addAction(QIcon(":list-remove.png"), tr("Remove"), this, SLOT(Remove()));
  context_menu_->addSeparator();
  context_menu_->addAction(QIcon(":open_stream.png"), tr("Add another stream..."), this, SIGNAL(ShowAddStreamDialog()));

  LoadStreams();
}

SavedRadio::~SavedRadio() {
  delete context_menu_;
}

RadioItem* SavedRadio::CreateRootItem(RadioItem* parent) {
  root_ = new RadioItem(this, RadioItem::Type_Service, tr("Your radio streams"), parent);
  root_->icon = QIcon(":open_stream.png");
  return root_;
}

void SavedRadio::LazyPopulate(RadioItem* item) {
  switch (item->type) {
    case RadioItem::Type_Service:
      foreach (const QString& stream, streams_)
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
    streams_ << s.value("url").toString();
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
    s.setValue("url", streams_[i]);
  }
  s.endArray();
}

void SavedRadio::ShowContextMenu(RadioItem* item, const QPoint& global_pos) {
  context_item_ = item;

  add_action_->setEnabled(item != root_);
  remove_action_->setEnabled(item != root_);

  context_menu_->popup(global_pos);
}

void SavedRadio::Remove() {
  streams_.removeAll(context_item_->key);
  context_item_->parent->DeleteNotify(context_item_->row);
  SaveStreams();
}

void SavedRadio::StartLoading(const QUrl& url) {
  emit StreamReady(url, url);
}

void SavedRadio::AddToPlaylist() {
  emit AddItemToPlaylist(context_item_);
}

RadioItem* SavedRadio::ItemForStream(const QUrl& url, RadioItem* parent) {
  RadioItem* s = new RadioItem(this, Type_Stream, url.toString(), parent);
  s->lazy_loaded = true;
  s->icon = QIcon(":last.fm/icon_radio.png");
  s->playable = true;
  return s;
}

void SavedRadio::Add(const QUrl &url) {
  if (streams_.contains(url.toString()))
    return;

  streams_ << url.toString();

  if (root_->lazy_loaded) {
    RadioItem* s = ItemForStream(url, NULL);
    s->InsertNotify(root_);
  }
  SaveStreams();
}
