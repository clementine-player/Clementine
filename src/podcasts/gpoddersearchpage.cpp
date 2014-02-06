/* This file is part of Clementine.
   Copyright 2012, David Sansome <me@davidsansome.com>
   
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

#include "gpoddersearchpage.h"
#include "podcast.h"
#include "podcastdiscoverymodel.h"
#include "ui_gpoddersearchpage.h"
#include "core/closure.h"
#include "core/network.h"

#include <QMessageBox>

GPodderSearchPage::GPodderSearchPage(Application* app, QWidget* parent)
  : AddPodcastPage(app, parent),
    ui_(new Ui_GPodderSearchPage),
    network_(new NetworkAccessManager(this)),
    api_(new mygpo::ApiRequest(network_))
{
  ui_->setupUi(this);
  connect(ui_->search, SIGNAL(clicked()), SLOT(SearchClicked()));
}

GPodderSearchPage::~GPodderSearchPage() {
  delete ui_;
  delete api_;
}

void GPodderSearchPage::SearchClicked() {
  emit Busy(true);

  mygpo::PodcastListPtr list(api_->search(ui_->query->text()));
  NewClosure(list, SIGNAL(finished()),
             this, SLOT(SearchFinished(mygpo::PodcastListPtr)),
             list);
  NewClosure(list, SIGNAL(parseError()),
             this, SLOT(SearchFailed(mygpo::PodcastListPtr)),
             list);
  NewClosure(list, SIGNAL(requestError(QNetworkReply::NetworkError)),
             this, SLOT(SearchFailed(mygpo::PodcastListPtr)),
             list);
}

void GPodderSearchPage::SearchFinished(mygpo::PodcastListPtr list) {
  emit Busy(false);

  model()->clear();

  foreach (mygpo::PodcastPtr gpo_podcast, list->list()) {
    Podcast podcast;
    podcast.InitFromGpo(gpo_podcast.data());

    model()->appendRow(model()->CreatePodcastItem(podcast));
  }
}

void GPodderSearchPage::SearchFailed(mygpo::PodcastListPtr list) {
  emit Busy(false);

  model()->clear();

  if (QMessageBox::warning(
        nullptr, tr("Failed to fetch podcasts"),
        tr("There was a problem communicating with gpodder.net"),
        QMessageBox::Retry | QMessageBox::Close,
        QMessageBox::Retry) != QMessageBox::Retry) {
    return;
  }

  // Try doing the search again.
  SearchClicked();
}

void GPodderSearchPage::Show() {
  ui_->query->setFocus();
}
