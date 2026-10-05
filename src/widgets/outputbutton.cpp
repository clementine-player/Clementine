/* This file is part of Clementine.
   Copyright 2026, John Maguire <john.maguire@gmail.com>

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

#include "outputbutton.h"

#include <QActionGroup>
#include <QEvent>
#include <QMenu>
#include <QPainter>

#include "engines/enginerouter.h"
#include "ui/iconloader.h"

namespace {

// The sizes the bundled glyphs come in.
const int kGlyphSizes[] = {22, 32, 48};

}  // namespace

OutputButton::OutputButton(EngineRouter* router, QWidget* parent)
    : QToolButton(parent), router_(router), menu_(new QMenu(this)) {
  setAutoRaise(true);
  setIconSize(QSize(22, 22));
  setPopupMode(QToolButton::InstantPopup);
  setMenu(menu_);
  // A Cast button is just the icon, without the arrow for a menu.
  setStyleSheet("QToolButton::menu-indicator { image: none; }");

  connect(router_, SIGNAL(OutputsChanged()), SLOT(OutputsChanged()));
  connect(menu_, SIGNAL(triggered(QAction*)), SLOT(OutputChosen(QAction*)));
  OutputsChanged();
}

QIcon OutputButton::Glyph(const QString& name) const {
  const QIcon icon = IconLoader::Load(name, IconLoader::Base);
  const QColor color = palette().color(QPalette::ButtonText);

  QIcon ret;
  for (int size : kGlyphSizes) {
    QPixmap pixmap = icon.pixmap(QSize(size, size));
    if (pixmap.isNull()) continue;
    QPainter painter(&pixmap);
    painter.setCompositionMode(QPainter::CompositionMode_SourceIn);
    painter.fillRect(pixmap.rect(), color);
    painter.end();
    ret.addPixmap(pixmap);
  }
  return ret;
}

QString OutputButton::OutputName(Engine::Base* engine) const {
  if (engine == router_->outputs().first()) return tr("This computer");
  return engine->output_name();
}

QIcon OutputButton::OutputIcon(Engine::Base* engine) const {
  if (engine == router_->outputs().first()) {
    return IconLoader::Load("audio-card", IconLoader::Base);
  }
  const QString icon = engine->output_icon();
  if (icon == "cast") return Glyph(icon);
  if (icon.isEmpty()) return QIcon();
  return IconLoader::Load(icon, IconLoader::Base);
}

void OutputButton::OutputsChanged() {
  if (!router_) return;
  const QList<Engine::Base*> outputs = router_->outputs();

  menu_->clear();
  QActionGroup* group = new QActionGroup(menu_);
  for (Engine::Base* engine : outputs) {
    QAction* action = menu_->addAction(OutputIcon(engine), OutputName(engine));
    action->setCheckable(true);
    action->setChecked(engine == router_->active_engine());
    action->setData(QVariant::fromValue<QObject*>(engine));
    group->addAction(action);
    // This computer, then the devices.
    if (engine == outputs.first() && outputs.size() > 1) menu_->addSeparator();
  }

  const bool local = router_->is_local();
  setIcon(Glyph(local ? "cast" : "cast-connected"));
  setToolTip(
      local ? tr("Play on another device")
            : tr("Playing on %1").arg(OutputName(router_->active_engine())));
  // Like a Cast button, only offered when there's somewhere to cast to.
  setVisible(outputs.size() > 1);
}

void OutputButton::OutputChosen(QAction* action) {
  if (!router_) return;
  QObject* chosen = action->data().value<QObject*>();
  // The menu is rebuilt whenever the outputs change, but check anyway.
  for (Engine::Base* engine : router_->outputs()) {
    if (engine == chosen) {
      router_->SetOutput(engine);
      return;
    }
  }
}

void OutputButton::changeEvent(QEvent* e) {
  QToolButton::changeEvent(e);
  // The glyphs are drawn in the palette's colours.
  if (e->type() == QEvent::PaletteChange) OutputsChanged();
}
