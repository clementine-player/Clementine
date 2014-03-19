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

#ifndef WIZARDPLUGIN_H
#define WIZARDPLUGIN_H

#include <QObject>

#include "generator_fwd.h"

class Application;
class LibraryBackend;

class QWizard;

namespace smart_playlists {

class WizardPlugin : public QObject {
  Q_OBJECT

 public:
  WizardPlugin(Application* app, LibraryBackend* library, QObject* parent);

  virtual QString type() const = 0;
  virtual QString name() const = 0;
  virtual QString description() const = 0;
  virtual bool is_dynamic() const { return false; }
  int start_page() const { return start_page_; }

  virtual void SetGenerator(GeneratorPtr gen) = 0;
  virtual GeneratorPtr CreateGenerator() const = 0;

  void Init(QWizard* wizard, int finish_page_id);

 protected:
  virtual int CreatePages(QWizard* wizard, int finish_page_id) = 0;

  Application* app_;
  LibraryBackend* library_;

 private:
  int start_page_;
};

}  // namespace smart_playlists

#endif  // WIZARDPLUGIN_H
