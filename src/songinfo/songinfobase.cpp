/* This file is part of Clementine.
   Copyright 2010, David Sansome <me@davidsansome.com>

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
#include "core/network.h"

#include <QFile>
#include <QScrollArea>
#include <QSettings>
#include <QSpacerItem>
#include <QTimer>
#include <QVBoxLayout>

const char* SongInfoBase::kSettingsGroup = "SongInfo";

SongInfoBase::SongInfoBase(QWidget* parent)
  : QWidget(parent),
    network_(new NetworkAccessManager(this)),
    fetcher_(new SongInfoFetcher(this)),
    current_request_id_(-1),
    scroll_area_(new QScrollArea),
    container_(new QVBoxLayout),
    section_container_(nullptr),
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
  container_widget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);
  container_widget->setBackgroundRole(QPalette::Base);
  container_->setSizeConstraint(QLayout::SetMinAndMaxSize);
  container_->setContentsMargins(0, 0, 0, 0);
  container_->setSpacing(6);
  scroll_area_->setWidget(container_widget);
  scroll_area_->setWidgetResizable(true);

  // Add a spacer to the bottom of the container
  container_->addStretch();

  // Set stylesheet
  QFile stylesheet(":/songinfo.css");
  stylesheet.open(QIODevice::ReadOnly);
  setStyleSheet(QString::fromAscii(stylesheet.readAll()));

  connect(fetcher_, SIGNAL(ResultReady(int,SongInfoFetcher::Result)),
          SLOT(ResultReady(int,SongInfoFetcher::Result)));
  connect(fetcher_, SIGNAL(InfoResultReady(int,CollapsibleInfoPane::Data)),
          SLOT(InfoResultReady(int,CollapsibleInfoPane::Data)));
}

void SongInfoBase::Clear() {
  fader_->StartFade();

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

  ConnectWidget(section->data().contents_);

  sections_.insert(index, section);
  qobject_cast<QVBoxLayout*>(section_container_->layout())->insertWidget(index, section);
  section->show();
}

void SongInfoBase::AddWidget(QWidget* widget) {
  ConnectWidget(widget);

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

  // Do this after the new pane has been shown otherwise it'll just grab a
  // black rectangle.
  Clear ();
  QTimer::singleShot(0, fader_, SLOT(StartBlur()));
}

void SongInfoBase::InfoResultReady (int id, const CollapsibleInfoPane::Data& data) {
}

void SongInfoBase::ResultReady(int id, const SongInfoFetcher::Result& result) {
  foreach (const CollapsibleInfoPane::Data& data, result.info_) {
    delete data.contents_;
  }
}

void SongInfoBase::CollapseSections() {
  QSettings s;
  s.beginGroup(kSettingsGroup);

  // Sections are already sorted by type and relevance, so the algorithm we use
  // to determine which ones to show by default is:
  //   * In the absense of any user preference, show the first (highest
  //     relevance section of each type and hide the rest)
  //   * If one or more sections in a type have been explicitly hidden/shown
  //     by the user before then hide all sections in that type and show only
  //     the ones that are explicitly shown.

  QMap<CollapsibleInfoPane::Data::Type, CollapsibleInfoPane*> types_;
  QSet<CollapsibleInfoPane::Data::Type> has_user_preference_;
  foreach (CollapsibleInfoPane* pane, sections_) {
    const CollapsibleInfoPane::Data::Type type = pane->data().type_;
    types_.insertMulti(type, pane);

    QVariant preference = s.value(pane->data().id_);
    if (preference.isValid()) {
      has_user_preference_.insert(type);
      if (preference.toBool()) {
        pane->Expand();
      }
    }
  }

  foreach (CollapsibleInfoPane::Data::Type type, types_.keys()) {
    if (!has_user_preference_.contains(type)) {
      // Expand the first one
      types_.values(type).last()->Expand();
    }
  }

  foreach (CollapsibleInfoPane* pane, sections_) {
    connect(pane, SIGNAL(Toggled(bool)), SLOT(SectionToggled(bool)));
  }
}

void SongInfoBase::SectionToggled(bool value) {
  CollapsibleInfoPane* pane = qobject_cast<CollapsibleInfoPane*>(sender());
  if (!pane || !sections_.contains(pane))
    return;

  QSettings s;
  s.beginGroup(kSettingsGroup);
  s.setValue(pane->data().id_, value);
}

void SongInfoBase::ReloadSettings() {
  foreach (CollapsibleInfoPane* pane, sections_) {
    QWidget* contents = pane->data().contents_;
    if (!contents)
      continue;

    QMetaObject::invokeMethod(contents, "ReloadSettings");
  }
}

void SongInfoBase::ConnectWidget(QWidget* widget) {
  const QMetaObject* m = widget->metaObject();

  if (m->indexOfSignal("ShowSettingsDialog()") != -1) {
    connect(widget, SIGNAL(ShowSettingsDialog()), SIGNAL(ShowSettingsDialog()));
  }

  if (m->indexOfSignal("DoGlobalSearch(QString)") != -1) {
    connect(widget, SIGNAL(DoGlobalSearch(QString)), SIGNAL(DoGlobalSearch(QString)));
  }
}

