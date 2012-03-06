/* This file is part of Clementine.
   Copyright 2012, David Sansome <me@davidsansome.com>
   
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
#include "addpodcastbyurl.h"
#include "gpoddertoptagspage.h"
#include "podcastbackend.h"
#include "podcastdiscoverymodel.h"
#include "ui_addpodcastdialog.h"
#include "core/application.h"
#include "ui/iconloader.h"
#include "widgets/widgetfadehelper.h"

#include <QPushButton>

AddPodcastDialog::AddPodcastDialog(Application* app, QWidget* parent)
  : QDialog(parent),
    app_(app),
    ui_(new Ui_AddPodcastDialog)
{
  ui_->setupUi(this);
  ui_->details->SetApplication(app);
  ui_->results->SetExpandOnReset(false);
  ui_->results_stack->setCurrentWidget(ui_->results_page);

  fader_ = new WidgetFadeHelper(ui_->details_scroll_area);

  connect(ui_->provider_list, SIGNAL(currentRowChanged(int)), SLOT(ChangePage(int)));
  connect(ui_->details, SIGNAL(LoadingFinished()), fader_, SLOT(StartFade()));

  // Create an Add Podcast button
  add_button_ = new QPushButton(IconLoader::Load("list-add"), tr("Add Podcast"), this);
  add_button_->setEnabled(false);
  connect(add_button_, SIGNAL(clicked()), SLOT(AddPodcast()));
  ui_->button_box->addButton(add_button_, QDialogButtonBox::AcceptRole);

  // Add providers
  AddPage(new AddPodcastByUrl(app, this));
  AddPage(new GPodderTopTagsPage(app, this));

  ui_->provider_list->setCurrentRow(0);
}

AddPodcastDialog::~AddPodcastDialog() {
  delete ui_;
}

void AddPodcastDialog::AddPage(AddPodcastPage* page) {
  pages_.append(page);
  page_is_busy_.append(false);

  ui_->stack->addWidget(page);
  new QListWidgetItem(page->windowIcon(), page->windowTitle(), ui_->provider_list);

  connect(page, SIGNAL(Busy(bool)), SLOT(PageBusyChanged(bool)));
}

void AddPodcastDialog::ChangePage(int index) {
  AddPodcastPage* page = pages_[index];

  ui_->stack->setCurrentIndex(index);
  ui_->stack->setVisible(page->has_visible_widget());
  ui_->results->setModel(page->model());
  ui_->results->setRootIsDecorated(page->model()->is_tree());

  ui_->results_stack->setCurrentWidget(
        page_is_busy_[index] ? ui_->busy_page : ui_->results_page);

  connect(ui_->results->selectionModel(), SIGNAL(currentRowChanged(QModelIndex,QModelIndex)),
          SLOT(ChangePodcast(QModelIndex)));
  ChangePodcast(QModelIndex());
  PageBusyChanged(page_is_busy_[index]);

  page->Show();
}

void AddPodcastDialog::ChangePodcast(const QModelIndex& current) {
  if (!current.isValid() ||
      current.data(PodcastDiscoveryModel::Role_Type).toInt() !=
          PodcastDiscoveryModel::Type_Podcast) {
    ui_->details_scroll_area->hide();
    return;
  }

  if (ui_->details_scroll_area->isVisible()) {
    fader_->StartBlur();
  } else {
    ui_->details_scroll_area->show();
  }

  current_podcast_ = current.data(PodcastDiscoveryModel::Role_Podcast).value<Podcast>();
  ui_->details->SetPodcast(current_podcast_);

  add_button_->setEnabled(current_podcast_.url().isValid());
}

void AddPodcastDialog::PageBusyChanged(bool busy) {
  const int index = pages_.indexOf(qobject_cast<AddPodcastPage*>(sender()));
  if (index == -1)
    return;

  page_is_busy_[index] = busy;

  if (index == ui_->provider_list->currentRow()) {
    ui_->results_stack->setCurrentWidget(busy ? ui_->busy_page : ui_->results_page);
  }
}

void AddPodcastDialog::AddPodcast() {
  app_->podcast_backend()->Subscribe(&current_podcast_);
}
