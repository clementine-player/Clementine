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

#include "slimbuttonbox.h"

#include <QEvent>
#include <QFile>
#include <QHBoxLayout>
#include <QPushButton>
#include <QStylePainter>
#include <QStyleOptionTabBarBaseV2>

SlimButtonBox::SlimButtonBox(QWidget* parent)
  : QWidget(parent),
    tab_bar_base_(false)
{
  setLayout(new QHBoxLayout);
  layout()->setSpacing(3);
  layout()->setContentsMargins(6, 0, 0, 0);

  ReloadStylesheet();
}

void SlimButtonBox::AddButton(const QString& text, const QIcon& icon) {
  QPushButton* button = new QPushButton(icon, text, this);
  button->setCheckable(true);

  connect(button, SIGNAL(clicked()), SLOT(ButtonClicked()));

  layout()->addWidget(button);
  buttons_ << button;
}

void SlimButtonBox::ButtonClicked() {
  QPushButton* button = qobject_cast<QPushButton*>(sender());
  if (!button)
    return;

  if (!button->isChecked()) {
    emit CurrentChanged(-1);
  } else {
    // Uncheck all the other buttons as well.  We can't use auto exclusive,
    // because we want to allow no buttons being pressed.
    foreach (QPushButton* other_button, buttons_) {
      if (other_button != button)
        other_button->setChecked(false);
    }

    emit CurrentChanged(buttons_.indexOf(button));
  }
}

void SlimButtonBox::changeEvent(QEvent* e) {
  if (e->type() == QEvent::PaletteChange) {
    ReloadStylesheet();
  }
}

void SlimButtonBox::paintEvent(QPaintEvent*) {
  QStylePainter p(this);

  if (tab_bar_base_) {
    QStyleOptionTabBarBaseV2 opt;
    opt.initFrom(this);
    opt.documentMode = true;
    opt.shape = QTabBar::RoundedSouth;

    p.drawPrimitive(QStyle::PE_FrameTabBarBase, opt);
  }
}

void SlimButtonBox::ReloadStylesheet() {
  QFile resource(":/slimbuttonbox.css");
  resource.open(QIODevice::ReadOnly);
  QString css = QString::fromLatin1(resource.readAll());

  css.replace("%darkhighlight", palette().color(QPalette::Highlight).darker(150).name());
  setStyleSheet(css);
}

void SlimButtonBox::SetTabBarBase(bool tab_bar_base) {
  tab_bar_base_ = tab_bar_base;
  update();
}

bool SlimButtonBox::IsAnyButtonChecked() const {
  foreach (QPushButton* button, buttons_) {
    if (button->isChecked())
      return true;
  }
  return false;
}

void SlimButtonBox::SetCurrentButton(int index) {
  for (int i=0 ; i<buttons_.count() ; ++i) {
    buttons_[i]->setChecked(i == index);
  }
}
