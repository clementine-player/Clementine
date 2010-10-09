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

#include "artistinfofetcher.h"
#include "artistinfoview.h"
#include "collapsibleinfopane.h"
#include "widgets/prettyimageview.h"

#include <QFile>
#include <QScrollArea>
#include <QTextEdit>
#include <QVBoxLayout>
#include <QtDebug>

ArtistInfoView::ArtistInfoView(NetworkAccessManager* network, QWidget *parent)
  : SongInfoBase(network, parent),
    fetcher_(new ArtistInfoFetcher(this)),
    current_request_id_(-1),
    scroll_area_(new QScrollArea),
    container_(new QVBoxLayout),
    image_view_(NULL),
    section_container_(NULL)
{
  connect(fetcher_, SIGNAL(ImageReady(int,QUrl)), SLOT(ImageReady(int,QUrl)));
  connect(fetcher_, SIGNAL(InfoReady(int,CollapsibleInfoPane::Data)), SLOT(InfoReady(int,CollapsibleInfoPane::Data)));

  // Add the top-level scroll area
  setLayout(new QVBoxLayout);
  layout()->setContentsMargins(0, 0, 0, 0);
  layout()->addWidget(scroll_area_);

  // Add a container widget to the scroll area
  QWidget* container_widget = new QWidget;
  container_widget->setLayout(container_);
  container_widget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::MinimumExpanding);
  container_widget->setBackgroundRole(QPalette::Base);
  container_->setSizeConstraint(QLayout::SetMinAndMaxSize);
  container_->setContentsMargins(0, 0, 0, 0);
  container_->setSpacing(0);
  scroll_area_->setWidget(container_widget);
  scroll_area_->setWidgetResizable(true);

  // Add a spacer to the bottom of the container
  container_->addSpacerItem(new QSpacerItem(0, 0, QSizePolicy::Fixed, QSizePolicy::Expanding));

  // Set stylesheet
  QFile stylesheet(":/songinfo.css");
  stylesheet.open(QIODevice::ReadOnly);
  setStyleSheet(QString::fromAscii(stylesheet.readAll()));
}

ArtistInfoView::~ArtistInfoView() {
}

void ArtistInfoView::AddSection(CollapsibleInfoPane* section) {
  int index = 0;
  for ( ; index<sections_.count() ; ++index) {
    if (section->data() < sections_[index]->data())
      break;
  }

  sections_.insert(index, section);
  qobject_cast<QVBoxLayout*>(section_container_->layout())->insertWidget(index, section);
  section->show();
}

void ArtistInfoView::Clear() {
  delete image_view_;
  delete section_container_;
  sections_.clear();
}

bool ArtistInfoView::NeedsUpdate(const Song& old_metadata, const Song& new_metadata) const {
  return old_metadata.artist() != new_metadata.artist();
}

void ArtistInfoView::Update(const Song& metadata) {
  Clear();
  current_request_id_ = fetcher_->FetchInfo(metadata.artist());

  // Image view goes at the top
  image_view_ = new PrettyImageView(network_);
  container_->insertWidget(0, image_view_);

  // Container for collapsable sections goes below
  section_container_ = new QWidget;
  section_container_->setLayout(new QVBoxLayout);
  section_container_->layout()->setContentsMargins(0, 0, 0, 0);
  section_container_->layout()->setSpacing(1);
  section_container_->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);
  container_->insertWidget(1, section_container_);
}

void ArtistInfoView::ImageReady(int id, const QUrl& url) {
  if (id != current_request_id_)
    return;

  image_view_->AddImage(url);
}

void ArtistInfoView::InfoReady(int id, const CollapsibleInfoPane::Data& data) {
  if (id != current_request_id_) {
    delete data.contents_;
    return;
  }

  AddSection(new CollapsibleInfoPane(data, this));
}
