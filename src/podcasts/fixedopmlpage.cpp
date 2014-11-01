/* This file is part of Clementine.
   Copyright 2012, David Sansome <me@davidsansome.com>
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

#include "fixedopmlpage.h"
#include "podcastdiscoverymodel.h"
#include "podcasturlloader.h"
#include "core/closure.h"

#include <QMessageBox>

FixedOpmlPage::FixedOpmlPage(const QUrl& opml_url, const QString& title,
                             const QIcon& icon, Application* app,
                             QWidget* parent)
    : AddPodcastPage(app, parent),
      loader_(new PodcastUrlLoader(this)),
      opml_url_(opml_url),
      done_initial_load_(false) {
  setWindowTitle(title);
  setWindowIcon(icon);
}

void FixedOpmlPage::Show() {
  if (!done_initial_load_) {
    emit Busy(true);
    done_initial_load_ = true;

    PodcastUrlLoaderReply* reply = loader_->Load(opml_url_);
    NewClosure(reply, SIGNAL(Finished(bool)), this,
               SLOT(LoadFinished(PodcastUrlLoaderReply*)), reply);
  }
}

void FixedOpmlPage::LoadFinished(PodcastUrlLoaderReply* reply) {
  reply->deleteLater();
  emit Busy(false);

  if (!reply->is_success()) {
    QMessageBox::warning(this, tr("Failed to load podcast"),
                         reply->error_text(), QMessageBox::Close);
    return;
  }

  switch (reply->result_type()) {
    case PodcastUrlLoaderReply::Type_Podcast:
      for (const Podcast& podcast : reply->podcast_results()) {
        model()->appendRow(model()->CreatePodcastItem(podcast));
      }
      break;

    case PodcastUrlLoaderReply::Type_Opml:
      model()->CreateOpmlContainerItems(reply->opml_results(),
                                        model()->invisibleRootItem());
      break;
  }
}
