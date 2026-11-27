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

#include "gpoddersearchpage.h"

#include <QMessageBox>

#include "core/network.h"
#include "gpodderclient.h"
#include "podcast.h"
#include "podcastdiscoverymodel.h"
#include "ui/iconloader.h"
#include "ui_gpoddersearchpage.h"

GPodderSearchPage::GPodderSearchPage(Application* app, QWidget* parent)
    : AddPodcastPage(app, parent),
      ui_(new Ui_GPodderSearchPage),
      network_(new NetworkAccessManager(this)),
      client_(new GPodderClient(network_, this)) {
  ui_->setupUi(this);
  connect(ui_->search, SIGNAL(clicked()), SLOT(SearchClicked()));
  setWindowIcon(IconLoader::Load("mygpo", IconLoader::Provider));
}

GPodderSearchPage::~GPodderSearchPage() { delete ui_; }

void GPodderSearchPage::SearchClicked() {
  emit Busy(true);

  client_->Search(
      ui_->query->text(),
      [this](const QList<Podcast>& podcasts) { SearchFinished(podcasts); },
      [this](const QString&) { SearchFailed(); });
}

void GPodderSearchPage::SearchFinished(const QList<Podcast>& podcasts) {
  emit Busy(false);

  model()->clear();

  for (const Podcast& podcast : podcasts) {
    model()->appendRow(model()->CreatePodcastItem(podcast));
  }
}

void GPodderSearchPage::SearchFailed() {
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

void GPodderSearchPage::Show() { ui_->query->setFocus(); }
