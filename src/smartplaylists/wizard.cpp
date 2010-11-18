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

#include "generator.h"
#include "searchpreview.h"
#include "searchtermwidget.h"
#include "wizard.h"
#include "ui_wizard.h"

namespace smart_playlists {

Wizard::SearchPage::SearchPage(QWidget* parent)
  : QWizardPage(parent)
{
}

bool Wizard::SearchPage::isComplete() const {
  if (type_->currentIndex() == 2) // All songs
    return true;

  foreach (SearchTermWidget* widget, terms_) {
    if (!widget->Term().is_valid())
      return false;
  }
  return true;
}

Wizard::Wizard(LibraryBackend* library, QWidget* parent)
  : QWizard(parent),
    ui_(new Ui_SmartPlaylistWizard),
    library_(library)
{
  ui_->setupUi(this);
  ui_->limit_value->setValue(Generator::kDefaultLimit);

  connect(ui_->search_type, SIGNAL(currentIndexChanged(int)), SLOT(SearchTypeChanged()));

  // Get the type combo box
  ui_->page_query_search->type_ = ui_->search_type;

  // Create the new search term widget
  ui_->page_query_search->new_term_ = new SearchTermWidget(library_, this);
  ui_->page_query_search->new_term_->SetActive(false);
  connect(ui_->page_query_search->new_term_, SIGNAL(Clicked()), SLOT(AddSearchTerm()));

  // Add an empty initial term
  ui_->page_query_search->layout_ = static_cast<QVBoxLayout*>(ui_->terms_group->layout());
  ui_->page_query_search->layout_->addWidget(ui_->page_query_search->new_term_);
  AddSearchTerm();

  // Add the preview widget at the bottom of the search terms page
  QVBoxLayout* terms_page_layout = static_cast<QVBoxLayout*>(ui_->page_query_search->layout());
  terms_page_layout->addStretch();
  ui_->page_query_search->preview_ = new SearchPreview(this);
  ui_->page_query_search->preview_->set_library(library_);
  terms_page_layout->addWidget(ui_->page_query_search->preview_);

  // Add sort field texts
  for (int i=0 ; i<SearchTerm::FieldCount ; ++i) {
    const SearchTerm::Field field = SearchTerm::Field(i);
    const QString field_name = SearchTerm::FieldName(field);
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

Wizard::~Wizard() {
  delete ui_;
}

void Wizard::UpdateSortOrder() {
  const SearchTerm::Field field =
      SearchTerm::Field(ui_->sort_field_value->currentIndex());
  const SearchTerm::Type type = SearchTerm::TypeOf(field);
  const QString asc = SearchTerm::FieldSortOrderText(type, true);
  const QString desc = SearchTerm::FieldSortOrderText(type, false);

  ui_->sort_order->clear();
  ui_->sort_order->addItem(asc);
  ui_->sort_order->addItem(desc);
}

void Wizard::AddSearchTerm() {
  SearchTermWidget* widget =
      new SearchTermWidget(library_, this);
  connect(widget, SIGNAL(RemoveClicked()), SLOT(RemoveSearchTerm()));
  connect(widget, SIGNAL(Changed()), SLOT(UpdateTermPreview()));

  ui_->page_query_search->layout_->insertWidget(
        ui_->page_query_search->terms_.count(), widget);
  ui_->page_query_search->terms_ << widget;

  UpdateTermPreview();
}

void Wizard::RemoveSearchTerm() {
  SearchTermWidget* widget =
      qobject_cast<SearchTermWidget*>(sender());
  if (!widget)
    return;

  const int index = ui_->page_query_search->terms_.indexOf(widget);
  if (index == -1)
    return;

  delete ui_->page_query_search->terms_.takeAt(index);
  UpdateTermPreview();
}

void Wizard::UpdateTermPreview() {
  Search search = MakeSearch();
  emit ui_->page_query_search->completeChanged();
  if (!search.is_valid())
    return;

  // Don't apply limits in the term page
  search.limit_ = -1;

  ui_->page_query_search->preview_->Update(search);
}

void Wizard::UpdateSortPreview() {
  Search search = MakeSearch();
  if (!search.is_valid())
    return;

  ui_->sort_preview->Update(search);
}

Search Wizard::MakeSearch() const {
  Search ret;

  // Search type
  ret.search_type_ = Search::SearchType(ui_->search_type->currentIndex());

  // Search terms
  foreach (SearchTermWidget* widget, ui_->page_query_search->terms_) {
    SearchTerm term = widget->Term();
    if (term.is_valid())
      ret.terms_ << term;
  }

  // Sort order
  if (ui_->sort_random->isChecked()) {
    ret.sort_type_ = Search::Sort_Random;
  } else {
    const bool ascending = ui_->sort_order->currentIndex() == 0;
    ret.sort_type_ = ascending ? Search::Sort_FieldAsc :
                                 Search::Sort_FieldDesc;
    ret.sort_field_ = SearchTerm::Field(
          ui_->sort_field_value->currentIndex());
  }

  // Limit
  if (ui_->limit_none->isChecked())
    ret.limit_ = -1;
  else
    ret.limit_ = ui_->limit_value->value();

  return ret;
}

void Wizard::SearchTypeChanged() {
  const bool all = ui_->search_type->currentIndex() == 2;
  ui_->terms_group->setEnabled(!all);

  UpdateTermPreview();
}

} // namespace
