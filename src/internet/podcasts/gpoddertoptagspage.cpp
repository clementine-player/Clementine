/* This file is part of Clementine.
   Copyright 2012, David Sansome <me@davidsansome.com>
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

#include "gpoddertoptagspage.h"

#include <QMessageBox>

#include "core/closure.h"
#include "core/network.h"
#include "gpoddertoptagsmodel.h"
#include "ui/iconloader.h"

const int GPodderTopTagsPage::kMaxTagCount = 100;

GPodderTopTagsPage::GPodderTopTagsPage(Application* app, QWidget* parent)
    : AddPodcastPage(app, parent),
      network_(new NetworkAccessManager(this)),
      api_(new mygpo::ApiRequest(network_)),
      done_initial_load_(false) {
  setWindowTitle(tr("gpodder.net directory"));
  setWindowIcon(IconLoader::Load("mygpo", IconLoader::Provider));

  SetModel(new GPodderTopTagsModel(api_, app, this));
}

GPodderTopTagsPage::~GPodderTopTagsPage() { delete api_; }

void GPodderTopTagsPage::Show() {
  if (!done_initial_load_) {
    // Start the request for list of top-level tags
    emit Busy(true);
    done_initial_load_ = true;

    mygpo::TagListPtr tag_list(api_->topTags(kMaxTagCount));
    NewClosure(tag_list, SIGNAL(finished()), this,
               SLOT(TagListLoaded(mygpo::TagListPtr)), tag_list);
    NewClosure(tag_list, SIGNAL(parseError()), this,
               SLOT(TagListFailed(mygpo::TagListPtr)), tag_list);
    NewClosure(tag_list, SIGNAL(requestError(QNetworkReply::NetworkError)),
               this, SLOT(TagListFailed(mygpo::TagListPtr)), tag_list);
  }
}

void GPodderTopTagsPage::TagListLoaded(mygpo::TagListPtr tag_list) {
  emit Busy(false);

  for (mygpo::TagPtr tag : tag_list->list()) {
    model()->appendRow(model()->CreateFolder(tag->tag()));
  }
}

void GPodderTopTagsPage::TagListFailed(mygpo::TagListPtr list) {
  emit Busy(false);
  done_initial_load_ = false;

  if (QMessageBox::warning(
          nullptr, tr("Failed to fetch directory"),
          tr("There was a problem communicating with gpodder.net"),
          QMessageBox::Retry | QMessageBox::Close,
          QMessageBox::Retry) != QMessageBox::Retry) {
    return;
  }

  // Try doing the search again.
  Show();
}
