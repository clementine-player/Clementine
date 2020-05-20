/* This file is part of Clementine.
   Copyright 2012, David Sansome <me@davidsansome.com>
   Copyright 2012, 2014, John Maguire <john.maguire@gmail.com>
   Copyright 2014, Krzysztof Sobiecki <sobkas@gmail.com>

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

#include "addpodcastdialog.h"

#include <QFileDialog>
#include <QPushButton>
#include <QTimer>

#include "addpodcastbyurl.h"
#include "fixedopmlpage.h"
#include "gpoddersearchpage.h"
#include "gpoddertoptagspage.h"
#include "itunessearchpage.h"
#include "podcastbackend.h"
#include "podcastdiscoverymodel.h"
#include "ui_addpodcastdialog.h"
#include "core/application.h"
#include "ui/iconloader.h"
#include "widgets/widgetfadehelper.h"

const char* AddPodcastDialog::kBbcOpmlUrl =
    "http://www.bbc.co.uk/podcasts.opml";

const char* AddPodcastDialog::kCbcOpmlUrl =
    "http://cbc.ca/podcasts.opml";

AddPodcastDialog::AddPodcastDialog(Application* app, QWidget* parent)
    : QDialog(parent),
      app_(app),
      ui_(new Ui_AddPodcastDialog),
      last_opml_path_(QDir::homePath()) {
  ui_->setupUi(this);
  ui_->details->SetApplication(app);
  ui_->results->SetExpandOnReset(false);
  ui_->results->SetAddOnDoubleClick(false);
  ui_->results_stack->setCurrentWidget(ui_->results_page);

  fader_ = new WidgetFadeHelper(ui_->details_scroll_area);

  connect(ui_->provider_list, SIGNAL(currentRowChanged(int)),
          SLOT(ChangePage(int)));
  connect(ui_->details, SIGNAL(LoadingFinished()), fader_, SLOT(StartFade()));
  connect(ui_->results, SIGNAL(doubleClicked(QModelIndex)),
          SLOT(PodcastDoubleClicked(QModelIndex)));

  // Create Add and Remove Podcast buttons
  add_button_ =
      new QPushButton(IconLoader::Load("list-add", IconLoader::Base), 
                      tr("Add Podcast"), this);
  add_button_->setEnabled(false);
  connect(add_button_, SIGNAL(clicked()), SLOT(AddPodcast()));
  ui_->button_box->addButton(add_button_, QDialogButtonBox::ActionRole);

  remove_button_ =
      new QPushButton(IconLoader::Load("list-remove", IconLoader::Base), 
                                       tr("Unsubscribe"), this);
  remove_button_->setEnabled(false);
  connect(remove_button_, SIGNAL(clicked()), SLOT(RemovePodcast()));
  ui_->button_box->addButton(remove_button_, QDialogButtonBox::ActionRole);

  QPushButton* settings_button = new QPushButton(
      IconLoader::Load("configure", IconLoader::Base), 
      tr("Configure podcasts..."), this);
  connect(settings_button, SIGNAL(clicked()), SLOT(OpenSettingsPage()));
  ui_->button_box->addButton(settings_button, QDialogButtonBox::ResetRole);

  // Create an Open OPML file button
  QPushButton* open_opml_button = new QPushButton(
      IconLoader::Load("document-open", IconLoader::Base), 
      tr("Open OPML file..."), this);
  connect(open_opml_button, SIGNAL(clicked()), this, SLOT(OpenOPMLFile()));
  ui_->button_box->addButton(open_opml_button, QDialogButtonBox::ResetRole);

  // Add providers
  by_url_page_ = new AddPodcastByUrl(app, this);
  AddPage(by_url_page_);
  AddPage(new FixedOpmlPage(QUrl(kBbcOpmlUrl), tr("BBC Podcasts"),
                            IconLoader::Load("bbc", IconLoader::Provider), 
                            app, this));
  AddPage(new FixedOpmlPage(QUrl(kCbcOpmlUrl), tr("CBC Podcasts"),
                            IconLoader::Load("cbc", IconLoader::Provider), 
                            app, this));
  AddPage(new GPodderTopTagsPage(app, this));
  AddPage(new GPodderSearchPage(app, this));
  AddPage(new ITunesSearchPage(app, this));

  ui_->provider_list->setCurrentRow(0);
}

AddPodcastDialog::~AddPodcastDialog() { delete ui_; }

void AddPodcastDialog::ShowWithUrl(const QUrl& url) {
  by_url_page_->SetUrlAndGo(url);
  ui_->provider_list->setCurrentRow(0);
  show();
}

void AddPodcastDialog::ShowWithOpml(const OpmlContainer& opml) {
  by_url_page_->SetOpml(opml);
  ui_->provider_list->setCurrentRow(0);
  show();
}

void AddPodcastDialog::AddPage(AddPodcastPage* page) {
  pages_.append(page);
  page_is_busy_.append(false);

  ui_->stack->addWidget(page);
  new QListWidgetItem(page->windowIcon(), page->windowTitle(),
                      ui_->provider_list);

  connect(page, SIGNAL(Busy(bool)), SLOT(PageBusyChanged(bool)));
}

void AddPodcastDialog::ChangePage(int index) {
  AddPodcastPage* page = pages_[index];

  ui_->stack->setCurrentIndex(index);
  ui_->stack->setVisible(page->has_visible_widget());
  ui_->results->setModel(page->model());

  ui_->results_stack->setCurrentWidget(
      page_is_busy_[index] ? ui_->busy_page : ui_->results_page);

  connect(ui_->results->selectionModel(),
          SIGNAL(currentRowChanged(QModelIndex, QModelIndex)),
          SLOT(ChangePodcast(QModelIndex)));
  ChangePodcast(QModelIndex());
  CurrentPageBusyChanged(page_is_busy_[index]);

  page->Show();
}

void AddPodcastDialog::ChangePodcast(const QModelIndex& current) {
  QVariant podcast_variant = current.data(PodcastDiscoveryModel::Role_Podcast);

  // If the selected item is invalid or not a podcast, hide the details pane.
  if (podcast_variant.isNull()) {
    ui_->details_scroll_area->hide();
    add_button_->setEnabled(false);
    remove_button_->setEnabled(false);
    return;
  }

  current_podcast_ = podcast_variant.value<Podcast>();

  // Start the blur+fade if there's already a podcast in the details pane.
  if (ui_->details_scroll_area->isVisible()) {
    fader_->StartBlur();
  } else {
    ui_->details_scroll_area->show();
  }

  // Update the details pane
  ui_->details->SetPodcast(current_podcast_);

  // Is the user already subscribed to this podcast?
  Podcast subscribed_podcast =
      app_->podcast_backend()->GetSubscriptionByUrl(current_podcast_.url());
  const bool is_subscribed = subscribed_podcast.url().isValid();

  if (is_subscribed) {
    // Use the one from the database which will contain the ID.
    current_podcast_ = subscribed_podcast;
  }

  add_button_->setEnabled(!is_subscribed);
  remove_button_->setEnabled(is_subscribed);
}

void AddPodcastDialog::PageBusyChanged(bool busy) {
  const int index = pages_.indexOf(qobject_cast<AddPodcastPage*>(sender()));
  if (index == -1) return;

  page_is_busy_[index] = busy;

  if (index == ui_->provider_list->currentRow()) {
    CurrentPageBusyChanged(busy);
  }
}

void AddPodcastDialog::CurrentPageBusyChanged(bool busy) {
  ui_->results_stack->setCurrentWidget(busy ? ui_->busy_page
                                            : ui_->results_page);
  ui_->stack->setDisabled(busy);

  QTimer::singleShot(0, this, SLOT(SelectFirstPodcast()));
}

void AddPodcastDialog::SelectFirstPodcast() {
  // Select the first item if there was one.
  const PodcastDiscoveryModel* model =
      pages_[ui_->provider_list->currentRow()]->model();
  if (model->rowCount() > 0) {
    ui_->results->selectionModel()->setCurrentIndex(
        model->index(0, 0), QItemSelectionModel::ClearAndSelect);
  }
}

void AddPodcastDialog::AddPodcast() {
  app_->podcast_backend()->Subscribe(&current_podcast_);
  add_button_->setEnabled(false);
  remove_button_->setEnabled(true);
}

void AddPodcastDialog::PodcastDoubleClicked(const QModelIndex& index) {
  QVariant podcast_variant = index.data(PodcastDiscoveryModel::Role_Podcast);
  if (podcast_variant.isNull()) {
    return;
  }

  current_podcast_ = podcast_variant.value<Podcast>();
  app_->podcast_backend()->Subscribe(&current_podcast_);

  add_button_->setEnabled(false);
  remove_button_->setEnabled(true);
}

void AddPodcastDialog::RemovePodcast() {
  app_->podcast_backend()->Unsubscribe(current_podcast_);
  current_podcast_.set_database_id(-1);
  add_button_->setEnabled(true);
  remove_button_->setEnabled(false);
}

void AddPodcastDialog::OpenSettingsPage() {
  app_->OpenSettingsDialogAtPage(SettingsDialog::Page_Podcasts);
}

void AddPodcastDialog::OpenOPMLFile() {
  const QString filename = QFileDialog::getOpenFileName(
      this, tr("Open OPML file"), last_opml_path_, "OPML files (*.opml)");

  if (filename.isEmpty()) {
    return;
  }

  last_opml_path_ = filename;

  by_url_page_->SetUrlAndGo(QUrl::fromLocalFile(last_opml_path_));
  ChangePage(ui_->stack->indexOf(by_url_page_));
}
