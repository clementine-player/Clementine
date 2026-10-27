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

#include <QEvent>
#include <QFont>
#include <QMenu>
#include <QMessageBox>
#include <QPainter>
#include <QPushButton>

#include "engines/enginerouter.h"
#include "ui/iconloader.h"

namespace {

// The sizes the bundled glyphs come in.
const int kGlyphSizes[] = {22, 32, 48};

}  // namespace

OutputButton::OutputButton(EngineRouter* router, RendererRegistry* registry,
                           QWidget* parent)
    : QToolButton(parent),
      router_(router),
      registry_(registry),
      menu_(new QMenu(this)) {
  setAutoRaise(true);
  setIconSize(QSize(22, 22));
  setPopupMode(QToolButton::InstantPopup);
  setMenu(menu_);
  // A Cast button is just the icon, without the arrow for a menu.
  setStyleSheet("QToolButton::menu-indicator { image: none; }");

  connect(router_, SIGNAL(OutputsChanged()), SLOT(OutputsChanged()));
  connect(menu_, SIGNAL(triggered(QAction*)), SLOT(OutputChosen(QAction*)));
  if (registry_) {
    connect(registry_, SIGNAL(ServingChanged()), SLOT(ServingChanged()));
  }
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
  if (engine == router_->outputs().first()) return Glyph("computer");
  const QString icon = engine->output_icon();
  if (icon == "cast") {
    return Glyph(engine == router_->active_engine() ? "cast-connected" : icon);
  }
  if (icon == "speaker" || icon == "speaker-group" || icon == "tv") {
    return Glyph(icon);
  }
  if (icon.isEmpty()) return QIcon();
  return IconLoader::Load(icon, IconLoader::Base);
}

void OutputButton::OutputsChanged() {
  if (!router_) return;
  const QList<Engine::Base*> outputs = router_->outputs();

  menu_->clear();
  for (Engine::Base* engine : outputs) {
    QAction* action = menu_->addAction(OutputIcon(engine), OutputName(engine));
    action->setData(QVariant::fromValue<QObject*>(engine));
    // The one playing is in bold. A checked item with an icon would get a
    // frame round the icon instead of a check mark, sized for a square icon.
    if (engine == router_->active_engine()) {
      QFont font = action->font();
      font.setBold(true);
      action->setFont(font);
    }
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
  pending_ = nullptr;
  // The menu is rebuilt whenever the outputs change, but check anyway.
  for (Engine::Base* engine : router_->outputs()) {
    if (engine == chosen) {
      PlayOn(engine);
      return;
    }
  }
}

void OutputButton::PlayOn(Engine::Base* engine) {
  const RendererRegistry::Reach reach =
      registry_ ? registry_->ReachFor(engine)
                : RendererRegistry::Reach::Reachable;
  if (reach == RendererRegistry::Reach::Reachable) {
    router_->SetOutput(engine);
    return;
  }

  const QString name = OutputName(engine);
  switch (Ask(name, registry_->ReachMessage(reach, name), reach)) {
    case Answer::TurnOn:
      pending_ = engine;
      emit TurnOnNetworkRemote();
      break;
    case Answer::Settings:
      emit ShowNetworkRemoteSettings();
      break;
    case Answer::Cancel:
      break;
  }
}

void OutputButton::ServingChanged() {
  if (!pending_ || !router_ || !registry_) return;
  // Restarting, the remote is off for a moment first.
  if (registry_->ReachFor(pending_.data()) ==
      RendererRegistry::Reach::RemoteOff) {
    return;
  }

  Engine::Base* engine = pending_.data();
  pending_ = nullptr;
  // If it still can't be played on, such as when the remote doesn't listen
  // on its network, this says so, and offers the settings.
  if (router_->outputs().contains(engine)) PlayOn(engine);
}

OutputButton::Answer OutputButton::Ask(const QString& name,
                                       const QString& message,
                                       RendererRegistry::Reach reach) {
  // TRANSLATORS: The title of the message shown when the user picks a
  // Chromecast or other device to play on, and it can't get the music from
  // Clementine's network remote. %1 is the device's name.
  QMessageBox box(QMessageBox::Information, tr("Play on %1").arg(name), message,
                  QMessageBox::Cancel, window());
  QPushButton* turn_on = nullptr;
  if (reach == RendererRegistry::Reach::RemoteOff) {
    box.setInformativeText(
        // TRANSLATORS: Below the message saying the network remote is off,
        // which a Chromecast or other device needs to play. Turning it on
        // for the device also lets phones running the Clementine Remote app
        // connect, which is why this warns about it.
        tr("Turning on the network remote also lets the Clementine Remote "
           "app, on phones and tablets in your network, control Clementine."));
    // TRANSLATORS: A button that turns on the network remote, so the chosen
    // Chromecast or other device can play.
    turn_on = box.addButton(tr("Turn On"), QMessageBox::AcceptRole);
  }
  QPushButton* settings =
      // TRANSLATORS: A button that opens the Network Remote page of the
      // settings, where the user can fix why a Chromecast or other device
      // can't get music from Clementine. "Network Remote" is that page's
      // name.
      box.addButton(tr("Network Remote Settings..."), QMessageBox::ActionRole);
  box.setDefaultButton(turn_on ? turn_on : settings);
  box.exec();

  if (turn_on && box.clickedButton() == turn_on) return Answer::TurnOn;
  if (box.clickedButton() == settings) return Answer::Settings;
  return Answer::Cancel;
}

void OutputButton::changeEvent(QEvent* e) {
  QToolButton::changeEvent(e);
  // The glyphs are drawn in the palette's colours.
  if (e->type() == QEvent::PaletteChange) OutputsChanged();
}
