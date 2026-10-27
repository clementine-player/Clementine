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

#ifndef WIDGETS_OUTPUTBUTTON_H_
#define WIDGETS_OUTPUTBUTTON_H_

#include <QPointer>
#include <QToolButton>

#include "networkremote/streaming/rendererregistry.h"

class EngineRouter;
class QMenu;

namespace Engine {
class Base;
}

// Chooses where playback goes: this computer, or one of the EngineRouter's
// other outputs, such as a Cast device.
//
// It looks and behaves like a Cast button: the Cast icon, filled in while
// playback is on another device, and hidden while there's nowhere else to
// play.
//
// Devices such as Cast devices fetch their media from the network remote, so
// with a |registry|, choosing one that couldn't explains why, and offers to
// turn the remote on, or to change its settings.
class OutputButton : public QToolButton {
  Q_OBJECT

 public:
  // What the user chose to do about a device that can't get media.
  enum class Answer { Cancel, TurnOn, Settings };

  explicit OutputButton(EngineRouter* router,
                        RendererRegistry* registry = nullptr,
                        QWidget* parent = nullptr);

 signals:
  // The user asked for the network remote to be turned on, or for its
  // settings. Once it's on, the device they chose is played on, if it can be.
  void TurnOnNetworkRemote();
  void ShowNetworkRemoteSettings();

 protected:
  void changeEvent(QEvent* e) override;

  // Tells the user why |name| can't be played on, in |message|, and asks what
  // to do. TurnOn is only offered when the network remote is off.
  virtual Answer Ask(const QString& name, const QString& message,
                     RendererRegistry::Reach reach);

 private slots:
  void OutputsChanged();
  void OutputChosen(QAction* action);
  void ServingChanged();

 private:
  // The icon called |name|, drawn in the palette's text colour, since the
  // glyphs are a single colour.
  QIcon Glyph(const QString& name) const;
  QIcon OutputIcon(Engine::Base* engine) const;
  QString OutputName(Engine::Base* engine) const;
  // Plays on |engine|, if it can get media; otherwise asks the user.
  void PlayOn(Engine::Base* engine);

  QPointer<EngineRouter> router_;
  QPointer<RendererRegistry> registry_;
  QMenu* menu_;
  // Chosen while the network remote was off, and the user asked to turn it
  // on: played on once it's serving.
  QPointer<Engine::Base> pending_;
};

#endif  // WIDGETS_OUTPUTBUTTON_H_
