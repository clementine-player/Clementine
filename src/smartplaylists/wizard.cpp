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
#include "ui_wizardfinishpage.h"

#include <QLabel>
#include <QRadioButton>
#include <QSignalMapper>
#include <QVBoxLayout>

namespace smart_playlists {

class Wizard::TypePage : public QWizardPage {
public:
  TypePage(QWidget* parent)
    : QWizardPage(parent), next_id_(-1) {}

  int nextId() const { return next_id_; }
  int next_id_;
};

class Wizard::FinishPage : public QWizardPage {
public:
  FinishPage(QWidget* parent)
    : QWizardPage(parent),
      ui_(new Ui_SmartPlaylistWizardFinishPage) {
    ui_->setupUi(this);
    connect(ui_->name, SIGNAL(textChanged(QString)), SIGNAL(completeChanged()));
  }

  ~FinishPage() {
    delete ui_;
  }

  int nextId() const { return -1; }
  bool isComplete() const { return !ui_->name->text().isEmpty(); }

  Ui_SmartPlaylistWizardFinishPage* ui_;
};

Wizard::Wizard(LibraryBackend* library, QWidget* parent)
  : QWizard(parent),
    library_(library),
    type_page_(new TypePage(this)),
    finish_page_(new FinishPage(this)),
    type_index_(-1),
    type_mapper_(new QSignalMapper(this))
{
  setWindowIcon(QIcon(":/icon.png"));
  setWindowTitle(tr("Smart playlist"));
  resize(687, 628);

  // Type page
  type_page_->setTitle(tr("Playlist type"));
  type_page_->setSubTitle(tr("A smart playlist is a dynamic list of songs that come from your library.  There are different types of smart playlist that offer different ways of selecting songs."));
  type_page_->setStyleSheet(
        "QRadioButton { font-weight: bold; }"
        "QLabel { margin-bottom: 1em; margin-left: 24px; }");
  addPage(type_page_);

  // Finish page
  finish_page_->setTitle(tr("Finish"));
  finish_page_->setSubTitle(tr("Choose a name for your smart playlist"));
  finish_id_ = addPage(finish_page_);

  connect(type_mapper_, SIGNAL(mapped(int)), SLOT(TypeChanged(int)));

  new QVBoxLayout(type_page_);
  AddPlugin(new QueryWizardPlugin(library_, this));
}

Wizard::~Wizard() {
  qDeleteAll(plugins_);
}

void Wizard::SetGenerator(GeneratorPtr gen) {
  // Find the right type and jump to the start page
  for (int i=0 ; i<plugins_.count() ; ++i) {
    if (plugins_[i]->type() == gen->type()) {
      TypeChanged(i);
      next();
      break;
    }
  }

  // Set the name
  finish_page_->ui_->name->setText(gen->name());

  // Tell the plugin to load
  plugins_[type_index_]->SetGenerator(gen);
}

void Wizard::AddPlugin(WizardPlugin* plugin) {
  const int index = plugins_.count();
  plugins_ << plugin;
  plugin->Init(this, finish_id_);

  // Create the radio button
  QRadioButton* radio_button = new QRadioButton(plugin->name(), type_page_);
  QLabel* description = new QLabel(plugin->description(), type_page_);
  type_page_->layout()->addWidget(radio_button);
  type_page_->layout()->addWidget(description);

  type_mapper_->setMapping(radio_button, index);
  connect(radio_button, SIGNAL(clicked()), type_mapper_, SLOT(map()));

  if (index == 0) {
    radio_button->setChecked(true);
    TypeChanged(0);
  }
}

void Wizard::TypeChanged(int index) {
  type_index_ = index;
  type_page_->next_id_ = plugins_[type_index_]->start_page();
}

GeneratorPtr Wizard::CreateGenerator() const {
  GeneratorPtr ret;
  if (type_index_ == -1)
    return ret;

  ret = plugins_[type_index_]->CreateGenerator();
  if (!ret)
    return ret;

  ret->set_name(finish_page_->ui_->name->text());
  return ret;
}

} // namespace
