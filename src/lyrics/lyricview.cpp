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

#include "lyricfetcher.h"
#include "lyricview.h"
#include "ui_lyricview.h"

LyricView::LyricView(QWidget *parent)
  : SongInfoBase(parent),
    ui_(new Ui_LyricView),
    fetcher_(NULL),
    current_request_id_(-1)
{
  ui_->setupUi(this);
  ui_->busy_container->setVisible(false);
}

LyricView::~LyricView() {
  delete ui_;
}

void LyricView::set_network(NetworkAccessManager* network) {
  fetcher_ = new LyricFetcher(network, this);
  connect(fetcher_, SIGNAL(SearchResult(int,bool,QString,QString)),
          SLOT(SearchFinished(int,bool,QString,QString)));
  connect(fetcher_, SIGNAL(SearchProgress(int,QString)),
          SLOT(SearchProgress(int,QString)));
}

void LyricView::Update(const Song& metadata) {
  current_request_id_ = fetcher_->SearchAsync(metadata);

  ui_->busy_text->SetText(tr("Searching..."));
  ui_->busy_container->setVisible(true);
}

void LyricView::SearchProgress(int id, const QString& provider) {
  if (id != current_request_id_)
    return;

  ui_->busy_text->SetText(tr("Searching %1...").arg(provider));
}

void LyricView::SearchFinished(int id, bool success, const QString& title, const QString& content) {
  if (id != current_request_id_)
    return;
  current_request_id_ = -1;
  ui_->busy_container->setVisible(false);

  if (success) {
    ui_->content->setHtml(content);
  } else {
    ui_->content->setHtml(tr("No lyrics could be found"));
  }
}
