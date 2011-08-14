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

#include "backgroundstreamssettingspage.h"
#include "behavioursettingspage.h"
#include "config.h"
#include "globalshortcutssettingspage.h"
#include "iconloader.h"
#include "playbacksettingspage.h"
#include "networkproxysettingspage.h"
#include "notificationssettingspage.h"
#include "mainwindow.h"
#include "settingsdialog.h"
#include "core/backgroundstreams.h"
#include "core/logging.h"
#include "core/networkproxyfactory.h"
#include "engines/enginebase.h"
#include "engines/gstengine.h"
#include "internet/digitallyimportedsettingspage.h"
#include "internet/magnatunesettingspage.h"
#include "library/librarysettingspage.h"
#include "playlist/playlistview.h"
#include "songinfo/songinfosettingspage.h"
#include "transcoder/transcodersettingspage.h"

#include "widgets/osdpretty.h"

#include "ui_settingsdialog.h"

#ifdef HAVE_LIBLASTFM
# include "internet/lastfmsettingspage.h"
#endif

#ifdef HAVE_WIIMOTEDEV
# include "wiimotedev/wiimotesettingspage.h"
#endif

#ifdef HAVE_REMOTE
# include "remote/remotesettingspage.h"
#endif

#ifdef HAVE_SPOTIFY
# include "internet/spotifysettingspage.h"
#endif

#include <QDesktopWidget>
#include <QPushButton>
#include <QScrollArea>


SettingsDialog::SettingsDialog(BackgroundStreams* streams, QWidget* parent)
  : QDialog(parent),
    model_(NULL),
    gst_engine_(NULL),
    song_info_view_(NULL),
    streams_(streams),
    ui_(new Ui_SettingsDialog),
    loading_settings_(false)
{
  ui_->setupUi(this);

  AddPage(Page_Playback, new PlaybackSettingsPage(this));
  AddPage(Page_Behaviour, new BehaviourSettingsPage(this));
  AddPage(Page_SongInformation, new SongInfoSettingsPage(this));
  AddPage(Page_GlobalShortcuts, new GlobalShortcutsSettingsPage(this));
  AddPage(Page_Notifications, new NotificationsSettingsPage(this));
  AddPage(Page_Library, new LibrarySettingsPage(this));

#ifdef HAVE_LIBLASTFM
  AddPage(Page_Lastfm, new LastFMSettingsPage(this));
#endif

#ifdef HAVE_SPOTIFY
  AddPage(Page_Spotify, new SpotifySettingsPage(this));
#endif

  AddPage(Page_Magnatune, new MagnatuneSettingsPage(this));
  AddPage(Page_DigitallyImported, new DigitallyImportedSettingsPage(this));
  AddPage(Page_BackgroundStreams, new BackgroundStreamsSettingsPage(this));
  AddPage(Page_Proxy, new NetworkProxySettingsPage(this));
  AddPage(Page_Transcoding, new TranscoderSettingsPage(this));

#ifdef HAVE_REMOTE
  AddPage(Page_Remote, new RemoteSettingsPage(this));
#endif

#ifdef HAVE_WIIMOTEDEV
  AddPage(Page_Wiimotedev, new WiimoteSettingsPage(this));
#endif

  // List box
  connect(ui_->list, SIGNAL(currentTextChanged(QString)), SLOT(CurrentTextChanged(QString)));
  ui_->list->setCurrentRow(Page_Playback);

  // Make sure the list is big enough to show all the items
  ui_->list->setMinimumWidth(ui_->list->sizeHintForColumn(0));

  ui_->buttonBox->button(QDialogButtonBox::Cancel)->setShortcut(QKeySequence::Close);
}

SettingsDialog::~SettingsDialog() {
  delete ui_;
}

void SettingsDialog::AddPage(Page id, SettingsPage* page) {
  // Connect page's signals to the settings dialog's signals
  connect(page, SIGNAL(NotificationPreview(OSD::Behaviour,QString,QString)),
                SIGNAL(NotificationPreview(OSD::Behaviour,QString,QString)));
  connect(page, SIGNAL(SetWiimotedevInterfaceActived(bool)),
                SIGNAL(SetWiimotedevInterfaceActived(bool)));

  // Create the list item
  QListWidgetItem* item = new QListWidgetItem(page->windowIcon(),
                                              page->windowTitle());
  ui_->list->addItem(item);

  if (!page->IsEnabled()) {
    item->setFlags(Qt::NoItemFlags);
  }

  // Create a scroll area containing the page
  QScrollArea* area = new QScrollArea;
  area->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  area->setWidget(page);
  area->setWidgetResizable(true);
  area->setFrameShape(QFrame::NoFrame);

  // Add the page to the stack
  ui_->stacked_widget->addWidget(area);

  // Remember where the page is
  PageData data;
  data.index_ = ui_->list->row(item);
  data.scroll_area_ = area;
  data.page_ = page;
  pages_[id] = data;
}

void SettingsDialog::accept() {
  // Save settings
  foreach (const PageData& data, pages_.values()) {
    data.page_->Save();
  }

  QDialog::accept();
}

void SettingsDialog::showEvent(QShowEvent* e) {
  // Load settings
  loading_settings_ = true;
  foreach (const PageData& data, pages_.values()) {
    data.page_->Load();
  }
  loading_settings_ = false;

  // Resize the dialog if it's too big
  const QSize available = QApplication::desktop()->availableGeometry(this).size();
  if (available.height() < height()) {
    resize(width(), sizeHint().height());
  }

  QDialog::showEvent(e);
}

void SettingsDialog::OpenAtPage(Page page) {
  if (!pages_.contains(page)) {
    return;
  }

  ui_->list->setCurrentRow(pages_[page].index_);
  show();
}

void SettingsDialog::CurrentTextChanged(const QString& text) {
  ui_->title->setText("<b>" + text + "</b>");
}
