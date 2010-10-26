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

#include "smartplaylistsearchtermwidget.h"
#include "smartplaylistwizard.h"
#include "ui_smartplaylistwizard.h"

SmartPlaylistWizard::SmartPlaylistWizard(LibraryBackend* library, QWidget* parent)
  : QWizard(parent),
    ui_(new Ui_SmartPlaylistWizard),
    library_(library)
{
  ui_->setupUi(this);

  new_search_term_ = new SmartPlaylistSearchTermWidget(library_, this);
  new_search_term_->SetActive(false);
  connect(new_search_term_, SIGNAL(Clicked()), SLOT(AddSearchTerm()));

  search_term_layout_ = new QVBoxLayout(ui_->page_query_search);
  search_term_layout_->addWidget(new_search_term_);
  search_term_layout_->addStretch();
  AddSearchTerm();
}

SmartPlaylistWizard::~SmartPlaylistWizard() {
  delete ui_;
}

void SmartPlaylistWizard::AddSearchTerm() {
  SmartPlaylistSearchTermWidget* widget =
      new SmartPlaylistSearchTermWidget(library_, this);
  connect(widget, SIGNAL(RemoveClicked()), SLOT(RemoveSearchTerm()));

  search_term_layout_->insertWidget(search_terms_.count(), widget);
  search_terms_ << widget;
}

void SmartPlaylistWizard::RemoveSearchTerm() {
  SmartPlaylistSearchTermWidget* widget =
      qobject_cast<SmartPlaylistSearchTermWidget*>(sender());
  if (!widget)
    return;

  const int index = search_terms_.indexOf(widget);
  if (index == -1)
    return;

  delete search_terms_.takeAt(index);
}

