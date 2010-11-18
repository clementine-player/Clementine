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

#include "querywizardplugin.h"
#include "wizard.h"
#include "wizardplugin.h"

#include <QLabel>
#include <QRadioButton>
#include <QSignalMapper>
#include <QVBoxLayout>

namespace smart_playlists {

Wizard::Wizard(LibraryBackend* library, QWidget* parent)
  : QWizard(parent),
    library_(library),
    type_page_(new TypePage(this)),
    type_mapper_(new QSignalMapper(this))
{
  setWindowIcon(QIcon(":/icon.png"));
  setWindowTitle(tr("Smart playlist"));
  resize(687, 628);

  type_page_->setTitle(tr("Playlist type"));
  type_page_->setSubTitle(tr("A smart playlist is a dynamic list of songs that come from your library.  There are different types of smart playlist that offer different ways of selecting songs."));
  addPage(type_page_);

  connect(type_mapper_, SIGNAL(mapped(int)), SLOT(TypeChanged(int)));

  new QVBoxLayout(type_page_);
  AddPlugin(new QueryWizardPlugin(library_, this));
}

Wizard::~Wizard() {
  qDeleteAll(plugins_);
}

void Wizard::AddPlugin(WizardPlugin* plugin) {
  const int index = plugins_.count();
  plugins_ << plugin;
  plugin->Init(this);

  // Create the radio button
  QRadioButton* name = new QRadioButton(plugin->name(), type_page_);
  QLabel* description = new QLabel(plugin->description(), type_page_);
  type_page_->layout()->addWidget(name);
  type_page_->layout()->addWidget(description);

  type_mapper_->setMapping(name, index);
  connect(name, SIGNAL(clicked()), type_mapper_, SLOT(map()));

  if (index == 0) {
    name->setChecked(true);
    TypeChanged(0);
  }
}

void Wizard::TypeChanged(int index) {
  type_page_->next_id_ = plugins_[index]->start_page();
}

} // namespace
