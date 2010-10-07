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

#include <QFile>
#include <QScrollArea>
#include <QTextEdit>
#include <QVBoxLayout>
#include <QtDebug>

ArtistInfoView::ArtistInfoView(QWidget *parent)
  : SongInfoBase(parent),
    fetcher_(new ArtistInfoFetcher(this)),
    current_request_id_(-1),
    scroll_area_(new QScrollArea),
    container_(new QVBoxLayout)
{
  connect(fetcher_, SIGNAL(ImageReady(int,QUrl)), SLOT(ImageReady(int,QUrl)));
  connect(fetcher_, SIGNAL(InfoReady(int,QString,QWidget*)), SLOT(InfoReady(int,QString,QWidget*)));

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

void ArtistInfoView::AddChild(QWidget* widget) {
  children_ << widget;
  container_->insertWidget(container_->count() - 1, widget);
  widget->show();
}

void ArtistInfoView::Clear() {
  qDeleteAll(children_);
  children_.clear();
}

void ArtistInfoView::Update(const Song& metadata) {
  Clear();
  current_request_id_ = fetcher_->FetchInfo(metadata.artist());
}

void ArtistInfoView::ImageReady(int id, const QUrl& url) {
  if (id != current_request_id_)
    return;

  qDebug() << "Image" << url;
}

void ArtistInfoView::InfoReady(int id, const QString& title, QWidget* widget) {
  if (id != current_request_id_) {
    delete widget;
    return;
  }

  CollapsibleInfoPane* pane = new CollapsibleInfoPane(this);
  pane->SetTitle(title);
  pane->SetWidget(widget);
  AddChild(pane);
}
