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

#include "stretchheaderview.h"

#include <QDataStream>
#include <QtDebug>

#include <algorithm>

const float StretchHeaderView::kMinimumColumnWidth = 0.01;

StretchHeaderView::StretchHeaderView(Qt::Orientation orientation, QWidget* parent)
  : QHeaderView(orientation, parent),
    stretch_enabled_(false),
    in_mouse_move_event_(false)
{
  connect(this, SIGNAL(sectionResized(int,int,int)), SLOT(SectionResized(int,int,int)));
}

void StretchHeaderView::setModel(QAbstractItemModel* model) {
  QHeaderView::setModel(model);

  if (stretch_enabled_) {
    column_widths_.clear();
    int count = model->columnCount();
    for (int i=0 ; i<count ; ++i) {
      column_widths_ << 1.0 / count;
    }
  }
}

void StretchHeaderView::NormaliseWidths() {
  if (!stretch_enabled_)
    return;

  float sum = 0.0;
  foreach (float width, column_widths_)
    sum += width;

  if (sum != 0.0 && !qFuzzyCompare(sum, 1.0f)) {
    const float mult = 1.0 / sum;
    for (int i=0 ; i<column_widths_.count() ; ++i) {
      column_widths_[i] *= mult;
    }
  }
}

void StretchHeaderView::UpdateWidths(const QList<int>& sections) {
  if (!stretch_enabled_)
    return;

  int total_pixels = 0;
  for (int i=0 ; i<column_widths_.count() ; ++i) {
    const float w = column_widths_[i];
    const int pixels = w * width();
    total_pixels += pixels;

    if (!sections.isEmpty() && !sections.contains(i))
      continue;

    if (pixels == 0 && !isSectionHidden(i))
      hideSection(i);
    else if (pixels != 0 && isSectionHidden(i))
      showSection(i);

    if (pixels != 0)
      resizeSection(i, pixels);
  }

  // Add the remaining pixels to some arbitrary column just to make sure it
  // adds up to the total width.
  for (int i=0 ; i<column_widths_.count() ; ++i) {
    if (!sections.isEmpty() && !sections.contains(i))
      continue;

    const float w = column_widths_[i];
    const int pixels = w * width();
    if (pixels != 0) {
      resizeSection(i, pixels + (width() - total_pixels));
      break;
    }
  }
}

void StretchHeaderView::HideSection(int logical) {
  if (!stretch_enabled_) {
    hideSection(logical);
    return;
  }

  column_widths_[logical] = 0.0;
  NormaliseWidths();
  UpdateWidths();
}

void StretchHeaderView::ShowSection(int logical) {
  if (!stretch_enabled_) {
    showSection(logical);
    return;
  }

  // How many sections are visible already?
  int visible_count = 0;
  for (int i=0 ; i<count() ; ++i) {
    if (!isSectionHidden(i))
      visible_count ++;
  }

  column_widths_[logical] =
      visible_count == 0 ? 1.0 : 1.0 / visible_count;
  NormaliseWidths();
  UpdateWidths();
}

void StretchHeaderView::SetSectionHidden(int logical, bool hidden) {
  hidden ? HideSection(logical) : ShowSection(logical);
}

void StretchHeaderView::resizeEvent(QResizeEvent* event) {
  QHeaderView::resizeEvent(event);

  if (!stretch_enabled_)
    return;

  UpdateWidths();
}

void StretchHeaderView::mouseMoveEvent(QMouseEvent* e) {
  in_mouse_move_event_ = true;
  QHeaderView::mouseMoveEvent(e);
  in_mouse_move_event_ = false;
}

void StretchHeaderView::SectionResized(int logical, int, int new_size) {
  if (!stretch_enabled_)
    return;

  if (in_mouse_move_event_) {
    // Update this section's proportional width
    column_widths_[logical] = float(new_size) / width();

    // Find the visible sections to the right of the section that's being resized
    int visual = visualIndex(logical);
    QList<int> logical_sections_to_resize;
    for (int i=0 ; i<count() ; ++i) {
      if (!isSectionHidden(i) && visualIndex(i) > visual)
        logical_sections_to_resize << i;
    }

    // Resize just those columns
    if (!logical_sections_to_resize.isEmpty()) {
      float sum = 0.0;
      foreach (float width, column_widths_)
        sum += width;

      if (sum != 0.0 && !qFuzzyCompare(sum, 1.0f)) {
        const float d = (1.0 - sum) / logical_sections_to_resize.count();
        foreach (int i, logical_sections_to_resize) {
          column_widths_[i] = qMax(kMinimumColumnWidth, column_widths_[i] + d);
        }
      }

      in_mouse_move_event_ = false;
      UpdateWidths(logical_sections_to_resize);
      in_mouse_move_event_ = true;
    }
  }
}

void StretchHeaderView::ToggleStretchEnabled() {
  SetStretchEnabled(!is_stretch_enabled());
}

void StretchHeaderView::SetStretchEnabled(bool enabled) {
  stretch_enabled_ = enabled;

  if (enabled) {
    // Initialise the list of widths from the current state of the widget
    column_widths_.clear();
    for (int i=0 ; i<count() ; ++i) {
      column_widths_ << float(sectionSize(i)) / width();
    }

    // Stretch the columns to fill the widget
    NormaliseWidths();
    UpdateWidths();
  }

  emit StretchEnabledChanged(enabled);
}
