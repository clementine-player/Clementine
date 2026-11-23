/* This file is part of Clementine.
   Copyright 2010-2012, David Sansome <me@davidsansome.com>
   Copyright 2011, Tyler Rhodes <tyler.s.rhodes@gmail.com>
   Copyright 2011, Paweł Bara <keirangtp@gmail.com>
   Copyright 2014, Arnaud Bienner <arnaud.bienner@gmail.com>
   Copyright 2014, John Maguire <john.maguire@gmail.com>
   Copyright 2014, Krzysztof Sobiecki <sobkas@gmail.com>

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

#include <QMenu>
#include <QSettings>

#include "core/application.h"
#include "core/mimedata.h"
#include "globalsearch/globalsearch.h"
#include "globalsearch/savedradiosearchprovider.h"
#include "internet/core/internetmodel.h"
#include "ui/addstreamdialog.h"
#include "ui/iconloader.h"

const char* SavedRadio::kServiceName = "SavedRadio";
const char* SavedRadio::kSettingsGroup = "SavedRadio";

SavedRadio::SavedRadio(Application* app, InternetModel* parent)
    : InternetService(kServiceName, app, parent, parent), root_(nullptr) {
  LoadStreams();

  app_->global_search()->AddProvider(
      new SavedRadioSearchProvider(this, app_, this));
}

SavedRadio::~SavedRadio() {}

QStandardItem* SavedRadio::CreateRootItem() {
  root_ = new QStandardItem(
      IconLoader::Load("document-open-remote", IconLoader::Base),
      tr("Your radio streams"));
  root_->setData(true, InternetModel::Role_CanLazyLoad);
  return root_;
}

void SavedRadio::LazyPopulate(QStandardItem* item) {
  switch (item->data(InternetModel::Role_Type).toInt()) {
    case InternetModel::Type_Service:
      for (const Stream& stream : streams_) {
        AddStreamToList(stream, root_);
      }

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
  for (int i = 0; i < count; ++i) {
    s.setArrayIndex(i);
    streams_ << Stream(QUrl(s.value("url").toString()),
                       s.value("name").toString(),
                       QUrl(s.value("url_logo").toString()));
  }
  s.endArray();
}

void SavedRadio::SaveStreams() {
  QSettings s;
  s.beginGroup(kSettingsGroup);

  int count = streams_.size();
  s.beginWriteArray("streams", count);
  for (int i = 0; i < count; ++i) {
    s.setArrayIndex(i);
    s.setValue("url", streams_[i].url_);
    s.setValue("name", streams_[i].name_);
    s.setValue("url_logo", streams_[i].url_logo_);
  }
  s.endArray();

  emit StreamsChanged();
}

void SavedRadio::ShowContextMenu(const QPoint& global_pos) {
  if (!context_menu_) {
    context_menu_.reset(new QMenu);
    context_menu_->addActions(GetPlaylistActions());
    remove_action_ = context_menu_->addAction(
        IconLoader::Load("list-remove", IconLoader::Base), tr("Remove"), this,
        SLOT(Remove()));
    edit_action_ = context_menu_->addAction(
        IconLoader::Load("edit-rename", IconLoader::Base), tr("Edit..."), this,
        SLOT(Edit()));
    context_menu_->addSeparator();
    context_menu_->addAction(
        IconLoader::Load("document-open-remote", IconLoader::Base),
        tr("Add another stream..."), this, SIGNAL(ShowAddStreamDialog()));
  }

  const bool is_root =
      model()->current_index().data(InternetModel::Role_Type).toInt() ==
      InternetModel::Type_Service;

  GetAppendToPlaylistAction()->setEnabled(!is_root);
  GetReplacePlaylistAction()->setEnabled(!is_root);
  GetOpenInNewPlaylistAction()->setEnabled(!is_root);
  remove_action_->setEnabled(!is_root);
  edit_action_->setEnabled(!is_root);

  context_menu_->popup(global_pos);
}

void SavedRadio::Remove() {
  QStandardItem* context_item =
      model()->itemFromIndex(model()->current_index());
  if (!context_item || !context_item->parent()) return;

  streams_.removeAll(
      Stream(QUrl(context_item->data(InternetModel::Role_Url).toUrl())));
  context_item->parent()->removeRow(context_item->row());
  SaveStreams();
}

void SavedRadio::Edit() {
  QStandardItem* context_item =
      model()->itemFromIndex(model()->current_index());
  if (!context_item) return;

  if (!edit_dialog_) {
    edit_dialog_.reset(new AddStreamDialog);
    edit_dialog_->set_save_visible(false);
  }

  const QUrl original_url = context_item->data(InternetModel::Role_Url).toUrl();
  const int i = streams_.indexOf(Stream(original_url));
  if (i == -1) return;

  // Re-locate by URL and persistent index after exec(): the nested event loop
  // allows streams_ or the model tree to change while the dialog is open.
  const QPersistentModelIndex item_index(context_item->index());
  edit_dialog_->set_name(context_item->text());
  edit_dialog_->set_url(original_url);
  edit_dialog_->set_url_logo(streams_[i].url_logo_);
  if (edit_dialog_->exec() == QDialog::Rejected) return;

  const int updated_i = streams_.indexOf(Stream(original_url));
  if (updated_i != -1) {
    streams_[updated_i].name_ = edit_dialog_->name();
    streams_[updated_i].url_ = edit_dialog_->url();
    streams_[updated_i].url_logo_ = edit_dialog_->url_logo();
  }

  if (item_index.isValid()) {
    QStandardItem* item = model()->itemFromIndex(item_index);
    if (item) {
      item->setText(edit_dialog_->name());
      item->setData(edit_dialog_->url(), InternetModel::Role_Url);
    }
  }

  SaveStreams();
}

void SavedRadio::AddStreamToList(const Stream& stream, QStandardItem* parent) {
  QStandardItem* s = new QStandardItem(
      IconLoader::Load("icon_radio", IconLoader::Lastfm), stream.name_);
  s->setData(stream.url_, InternetModel::Role_Url);
  s->setData(InternetModel::PlayBehaviour_UseSongLoader,
             InternetModel::Role_PlayBehaviour);
  parent->appendRow(s);
}

void SavedRadio::Add(const QUrl& url, const QString& name,
                     const QUrl& url_logo) {
  if (streams_.contains(Stream(url))) return;

  Stream stream(url, name, url_logo);
  streams_ << stream;

  if (!root_->data(InternetModel::Role_CanLazyLoad).toBool()) {
    AddStreamToList(stream, root_);
  }
  SaveStreams();
}
