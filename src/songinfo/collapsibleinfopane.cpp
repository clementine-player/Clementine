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

#include "collapsibleinfoheader.h"
#include "collapsibleinfopane.h"

#include <QVBoxLayout>

CollapsibleInfoPane::CollapsibleInfoPane(QWidget* parent)
  : QWidget(parent),
    header_(new CollapsibleInfoHeader(this)),
    widget_(NULL)
{
  QVBoxLayout* layout = new QVBoxLayout(this);
  layout->setContentsMargins(0, 0, 0, 0);
  layout->setSpacing(3);
  layout->setSizeConstraint(QLayout::SetMinAndMaxSize);
  setLayout(layout);

  layout->addWidget(header_);

  setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Maximum);

  connect(header_, SIGNAL(ExpandedToggled(bool)), SLOT(ExpandedToggled(bool)));
}

void CollapsibleInfoPane::SetTitle(const QString& title) {
  header_->SetTitle(title);
}

void CollapsibleInfoPane::SetWidget(QWidget* widget) {
  if (widget_)
    delete widget_;

  widget_ = widget;
  layout()->addWidget(widget);
}

void CollapsibleInfoPane::Collapse() {
  header_->SetExpanded(false);
}

void CollapsibleInfoPane::Expand() {
  header_->SetExpanded(true);
}

void CollapsibleInfoPane::ExpandedToggled(bool expanded) {
  widget_->setVisible(expanded);
}
