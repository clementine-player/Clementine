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

#include "querywizardplugin.h"
#include "wizard.h"
#include "wizardplugin.h"
#include "ui_wizardfinishpage.h"

#include <QLabel>
#include <QRadioButton>
#include <QVBoxLayout>

namespace smart_playlists {

class Wizard::TypePage : public QWizardPage {
 public:
  TypePage(QWidget* parent) : QWizardPage(parent), next_id_(-1) {}

  int nextId() const { return next_id_; }
  int next_id_;
};

class Wizard::FinishPage : public QWizardPage {
 public:
  FinishPage(QWidget* parent)
      : QWizardPage(parent), ui_(new Ui_SmartPlaylistWizardFinishPage) {
    ui_->setupUi(this);
    connect(ui_->name, SIGNAL(textChanged(QString)), SIGNAL(completeChanged()));
  }

  ~FinishPage() { delete ui_; }

  int nextId() const { return -1; }
  bool isComplete() const { return !ui_->name->text().isEmpty(); }

  Ui_SmartPlaylistWizardFinishPage* ui_;
};

Wizard::Wizard(Application* app, LibraryBackend* library, QWidget* parent)
    : QWizard(parent),
      app_(app),
      library_(library),
      type_page_(new TypePage(this)),
      finish_page_(new FinishPage(this)),
      type_index_(-1) {
  setWindowIcon(QIcon(":/icon.png"));
  setWindowTitle(tr("Smart playlist"));
  resize(788, 628);

#ifdef Q_OS_MAC
  // MacStyle leaves an ugly empty space on the left side of the dialog.
  setWizardStyle(QWizard::ClassicStyle);
#endif  // Q_OS_MAC

  // Type page
  type_page_->setTitle(tr("Playlist type"));
  type_page_->setSubTitle(
      tr("A smart playlist is a dynamic list of songs that come from your "
         "library.  There are different types of smart playlist that offer "
         "different ways of selecting songs."));
  type_page_->setStyleSheet(
      "QRadioButton { font-weight: bold; }"
      "QLabel { margin-bottom: 1em; margin-left: 24px; }");
  addPage(type_page_);

  // Finish page
  finish_page_->setTitle(tr("Finish"));
  finish_page_->setSubTitle(tr("Choose a name for your smart playlist"));
  finish_id_ = addPage(finish_page_);

  new QVBoxLayout(type_page_);
  AddPlugin(new QueryWizardPlugin(app_, library_, this));

  // Skip the type page - remove this when we have more than one type
  setStartId(2);
}

Wizard::~Wizard() { qDeleteAll(plugins_); }

void Wizard::SetGenerator(GeneratorPtr gen) {
  // Find the right type and jump to the start page
  for (int i = 0; i < plugins_.count(); ++i) {
    if (plugins_[i]->type() == gen->type()) {
      TypeChanged(i);
      // TODO: Put this back in when the setStartId is removed from the ctor
      // next();
      break;
    }
  }

  // Set the name
  finish_page_->ui_->name->setText(gen->name());
  finish_page_->ui_->dynamic->setChecked(gen->is_dynamic());

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

  connect(radio_button, &QRadioButton::clicked,
          [this, index]() { TypeChanged(index); });

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
  if (type_index_ == -1) return ret;

  ret = plugins_[type_index_]->CreateGenerator();
  if (!ret) return ret;

  ret->set_name(finish_page_->ui_->name->text());
  ret->set_dynamic(finish_page_->ui_->dynamic->isChecked());
  return ret;
}

void Wizard::initializePage(int id) {
  if (id == finish_id_) {
    finish_page_->ui_->dynamic_container->setEnabled(
        plugins_[type_index_]->is_dynamic());
  }
  QWizard::initializePage(id);
}

}  // namespace
