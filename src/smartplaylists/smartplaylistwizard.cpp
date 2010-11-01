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

#include "smartplaylistsearchpreview.h"
#include "smartplaylistsearchtermwidget.h"
#include "smartplaylistwizard.h"
#include "ui_smartplaylistwizard.h"
#include "playlistgenerator.h"

SmartPlaylistWizard::SearchPage::SearchPage(QWidget* parent)
  : QWizardPage(parent)
{
}

bool SmartPlaylistWizard::SearchPage::isComplete() const {
  foreach (SmartPlaylistSearchTermWidget* widget, terms_) {
    if (!widget->Term().is_valid())
      return false;
  }
  return true;
}

SmartPlaylistWizard::SmartPlaylistWizard(LibraryBackend* library, QWidget* parent)
  : QWizard(parent),
    ui_(new Ui_SmartPlaylistWizard),
    library_(library)
{
  ui_->setupUi(this);
  ui_->limit_value->setValue(PlaylistGenerator::kDefaultLimit);

  // Create the new search term widget
  ui_->page_query_search->new_term_ = new SmartPlaylistSearchTermWidget(library_, this);
  ui_->page_query_search->new_term_->SetActive(false);
  connect(ui_->page_query_search->new_term_, SIGNAL(Clicked()), SLOT(AddSearchTerm()));

  // Add an empty initial term
  ui_->page_query_search->layout_ = new QVBoxLayout(ui_->page_query_search);
  ui_->page_query_search->layout_->addWidget(ui_->page_query_search->new_term_);
  ui_->page_query_search->layout_->addStretch();
  AddSearchTerm();

  // Add the preview widget at the bottom of the search terms page
  ui_->page_query_search->preview_ = new SmartPlaylistSearchPreview(this);
  ui_->page_query_search->preview_->set_library(library_);
  ui_->page_query_search->layout_->addWidget(ui_->page_query_search->preview_);

  // Add sort field texts
  for (int i=0 ; i<SmartPlaylistSearchTerm::FieldCount ; ++i) {
    const SmartPlaylistSearchTerm::Field field = SmartPlaylistSearchTerm::Field(i);
    const QString field_name = SmartPlaylistSearchTerm::FieldName(field);
    ui_->sort_field_value->addItem(field_name);
  }
  connect(ui_->sort_field_value, SIGNAL(currentIndexChanged(int)), SLOT(UpdateSortOrder()));
  UpdateSortOrder();

  // Set the sort and limit radio buttons back to their defaults - they would
  // have been changed by setupUi
  ui_->sort_random->setChecked(true);
  ui_->limit_none->setChecked(true);

  // Set up the preview widget that's already at the bottom of the sort page
  ui_->sort_preview->set_library(library_);
}

SmartPlaylistWizard::~SmartPlaylistWizard() {
  delete ui_;
}

void SmartPlaylistWizard::UpdateSortOrder() {
  const SmartPlaylistSearchTerm::Field field =
      SmartPlaylistSearchTerm::Field(ui_->sort_field_value->currentIndex());
  const SmartPlaylistSearchTerm::Type type = SmartPlaylistSearchTerm::TypeOf(field);
  const QString asc = SmartPlaylistSearchTerm::FieldSortOrderText(type, true);
  const QString desc = SmartPlaylistSearchTerm::FieldSortOrderText(type, false);

  ui_->sort_order->clear();
  ui_->sort_order->addItem(asc);
  ui_->sort_order->addItem(desc);
}

void SmartPlaylistWizard::AddSearchTerm() {
  SmartPlaylistSearchTermWidget* widget =
      new SmartPlaylistSearchTermWidget(library_, this);
  connect(widget, SIGNAL(RemoveClicked()), SLOT(RemoveSearchTerm()));
  connect(widget, SIGNAL(Changed()), SLOT(UpdateTermPreview()));

  ui_->page_query_search->layout_->insertWidget(
        ui_->page_query_search->terms_.count(), widget);
  ui_->page_query_search->terms_ << widget;

  UpdateTermPreview();
}

void SmartPlaylistWizard::RemoveSearchTerm() {
  SmartPlaylistSearchTermWidget* widget =
      qobject_cast<SmartPlaylistSearchTermWidget*>(sender());
  if (!widget)
    return;

  const int index = ui_->page_query_search->terms_.indexOf(widget);
  if (index == -1)
    return;

  delete ui_->page_query_search->terms_.takeAt(index);
  UpdateTermPreview();
}

void SmartPlaylistWizard::UpdateTermPreview() {
  SmartPlaylistSearch search = MakeSearch();
  emit ui_->page_query_search->completeChanged();
  if (!search.is_valid())
    return;

  // Don't apply limits in the term page
  search.limit_ = -1;

  ui_->page_query_search->preview_->Update(search);
}

void SmartPlaylistWizard::UpdateSortPreview() {
  SmartPlaylistSearch search = MakeSearch();
  if (!search.is_valid())
    return;

  ui_->sort_preview->Update(search);
}

SmartPlaylistSearch SmartPlaylistWizard::MakeSearch() const {
  SmartPlaylistSearch ret;

  // Search terms
  foreach (SmartPlaylistSearchTermWidget* widget, ui_->page_query_search->terms_) {
    SmartPlaylistSearchTerm term = widget->Term();
    if (!term.is_valid())
      return SmartPlaylistSearch();

    ret.terms_ << term;
  }

  // Sort order
  if (ui_->sort_random->isChecked()) {
    ret.sort_type_ = SmartPlaylistSearch::Sort_Random;
  } else {
    const bool ascending = ui_->sort_order->currentIndex() == 0;
    ret.sort_type_ = ascending ? SmartPlaylistSearch::Sort_FieldAsc :
                                 SmartPlaylistSearch::Sort_FieldDesc;
    ret.sort_field_ = SmartPlaylistSearchTerm::Field(
          ui_->sort_field_value->currentIndex());
  }

  // Limit
  if (ui_->limit_none->isChecked())
    ret.limit_ = -1;
  else
    ret.limit_ = ui_->limit_value->value();

  return ret;
}
