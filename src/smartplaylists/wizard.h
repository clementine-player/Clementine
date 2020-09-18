/* This file is part of Clementine.
   Copyright 2010, David Sansome <me@davidsansome.com>

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

#ifndef SMARTPLAYLISTWIZARD_H
#define SMARTPLAYLISTWIZARD_H

#include <QWizard>

#include "generator_fwd.h"

class Application;
class LibraryBackend;
class Ui_SmartPlaylistWizardFinishPage;

namespace smart_playlists {

class WizardPlugin;

class Wizard : public QWizard {
  Q_OBJECT

 public:
  Wizard(Application* app, LibraryBackend* library, QWidget* parent);
  ~Wizard();

  void SetGenerator(GeneratorPtr gen);
  GeneratorPtr CreateGenerator() const;

 protected:
  void initializePage(int id);

 private:
  class TypePage;
  class FinishPage;

  void AddPlugin(WizardPlugin* plugin);

 private slots:
  void TypeChanged(int index);

 private:
  Application* app_;
  LibraryBackend* library_;
  TypePage* type_page_;
  FinishPage* finish_page_;
  int finish_id_;

  int type_index_;
  QList<WizardPlugin*> plugins_;
};

}  // namespace smart_playlists

#endif  // SMARTPLAYLISTWIZARD_H
