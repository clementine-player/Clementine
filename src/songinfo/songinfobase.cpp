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

#include "songinfobase.h"

#include <QFile>
#include <QScrollArea>
#include <QSpacerItem>
#include <QVBoxLayout>

SongInfoBase::SongInfoBase(NetworkAccessManager* network, QWidget* parent)
  : QWidget(parent),
    network_(network),
    fetcher_(new SongInfoFetcher(this)),
    current_request_id_(-1),
    scroll_area_(new QScrollArea),
    container_(new QVBoxLayout),
    section_container_(NULL),
    fader_(new WidgetFadeHelper(this, 1000)),
    dirty_(false)
{
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
  container_->setSpacing(6);
  scroll_area_->setWidget(container_widget);
  scroll_area_->setWidgetResizable(true);

  // Add a spacer to the bottom of the container
  container_->addSpacerItem(new QSpacerItem(0, 0, QSizePolicy::Fixed, QSizePolicy::Expanding));

  // Set stylesheet
  QFile stylesheet(":/songinfo.css");
  stylesheet.open(QIODevice::ReadOnly);
  setStyleSheet(QString::fromAscii(stylesheet.readAll()));

  connect(fetcher_, SIGNAL(ResultReady(int,SongInfoFetcher::Result)),
          SLOT(ResultReady(int,SongInfoFetcher::Result)));
}

void SongInfoBase::Clear() {
  fader_->Start();

  qDeleteAll(widgets_);
  widgets_.clear();
  delete section_container_;
  sections_.clear();

  // Container for collapsable sections goes below
  section_container_ = new QWidget;
  section_container_->setLayout(new QVBoxLayout);
  section_container_->layout()->setContentsMargins(0, 0, 0, 0);
  section_container_->layout()->setSpacing(1);
  section_container_->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);
  container_->insertWidget(0, section_container_);
}

void SongInfoBase::AddSection(CollapsibleInfoPane* section) {
  int index = 0;
  for ( ; index<sections_.count() ; ++index) {
    if (section->data() < sections_[index]->data())
      break;
  }

  sections_.insert(index, section);
  qobject_cast<QVBoxLayout*>(section_container_->layout())->insertWidget(index, section);
  section->show();
}

void SongInfoBase::AddWidget(QWidget* widget) {
  container_->insertWidget(container_->count() - 2, widget);
  widgets_ << widget;
}

void SongInfoBase::SongChanged(const Song& metadata) {
  if (isVisible()) {
    MaybeUpdate(metadata);
    dirty_ = false;
  } else {
    queued_metadata_ = metadata;
    dirty_ = true;
  }
}

void SongInfoBase::SongFinished() {
  dirty_ = false;
}

void SongInfoBase::showEvent(QShowEvent* e) {
  if (dirty_) {
    MaybeUpdate(queued_metadata_);
    dirty_ = false;
  }
  QWidget::showEvent(e);
}

void SongInfoBase::MaybeUpdate(const Song& metadata) {
  if (old_metadata_.is_valid()) {
    if (!NeedsUpdate(old_metadata_, metadata)) {
      return;
    }
  }

  Update(metadata);
  old_metadata_ = metadata;
}

void SongInfoBase::Update(const Song& metadata) {
  current_request_id_ = fetcher_->FetchInfo(metadata);
}

void SongInfoBase::ResultReady(int id, const SongInfoFetcher::Result& result) {
  foreach (const CollapsibleInfoPane::Data& data, result.info_) {
    delete data.contents_;
  }
}
