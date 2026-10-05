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
class OutputButton : public QToolButton {
  Q_OBJECT

 public:
  explicit OutputButton(EngineRouter* router, QWidget* parent = nullptr);

 protected:
  void changeEvent(QEvent* e) override;

 private slots:
  void OutputsChanged();
  void OutputChosen(QAction* action);

 private:
  // The icon called |name|, drawn in the palette's text colour, since the
  // glyphs are a single colour.
  QIcon Glyph(const QString& name) const;
  QIcon OutputIcon(Engine::Base* engine) const;
  QString OutputName(Engine::Base* engine) const;

  QPointer<EngineRouter> router_;
  QMenu* menu_;
};

#endif  // WIDGETS_OUTPUTBUTTON_H_
