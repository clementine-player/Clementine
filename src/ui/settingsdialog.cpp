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

#include "settingsdialog.h"

#include "appearancesettingspage.h"
#include "backgroundstreamssettingspage.h"
#include "behavioursettingspage.h"
#include "config.h"
#include "core/application.h"
#include "core/backgroundstreams.h"
#include "core/logging.h"
#include "core/networkproxyfactory.h"
#include "core/player.h"
#include "engines/enginebase.h"
#include "engines/gstengine.h"
#include "globalsearch/globalsearchsettingspage.h"
#include "globalshortcutssettingspage.h"
#include "iconloader.h"
#include "internet/core/internetsettingscategory.h"
#include "library/librarysettingspage.h"
#include "mainwindow.h"
#include "networkproxysettingspage.h"
#include "networkremotesettingspage.h"
#include "notificationssettingspage.h"
#include "playbacksettingspage.h"
#include "playlist/playlistview.h"
#include "settingscategory.h"
#include "songinfo/songinfosettingspage.h"
#include "songmetadatasettingspage.h"
#include "transcoder/transcodersettingspage.h"
#include "ui_settingsdialog.h"
#include "widgets/groupediconview.h"
#include "widgets/osdpretty.h"

#ifdef HAVE_WIIMOTEDEV
#include "wiimotedev/wiimotesettingspage.h"
#endif

#include <QAbstractButton>
#include <QPainter>
#include <QPushButton>
#include <QScreen>
#include <QScrollArea>
#include <QWindow>

SettingsItemDelegate::SettingsItemDelegate(QObject* parent)
    : QStyledItemDelegate(parent) {}

QSize SettingsItemDelegate::sizeHint(const QStyleOptionViewItem& option,
                                     const QModelIndex& index) const {
  const bool is_separator =
      index.data(SettingsDialog::Role_IsSeparator).toBool();
  QSize ret = QStyledItemDelegate::sizeHint(option, index);

  if (is_separator) {
    ret.setHeight(ret.height() * 2);
  }

  return ret;
}

void SettingsItemDelegate::paint(QPainter* painter,
                                 const QStyleOptionViewItem& option,
                                 const QModelIndex& index) const {
  const bool is_separator =
      index.data(SettingsDialog::Role_IsSeparator).toBool();

  if (is_separator) {
    GroupedIconView::DrawHeader(painter, option.rect, option.font,
                                option.palette, index.data().toString(),
                                option.state & QStyle::State_Selected);
  } else {
    QStyledItemDelegate::paint(painter, option, index);
  }
}

SettingsDialog::SettingsDialog(Application* app, BackgroundStreams* streams,
                               QWidget* parent)
    : QDialog(parent),
      app_(app),
      model_(app_->directory_model()),
      gst_engine_(qobject_cast<GstEngine*>(app_->player()->engine())),
      song_info_view_(nullptr),
      streams_(streams),
      global_search_(app_->global_search()),
      appearance_(app_->appearance()),
      ui_(new Ui_SettingsDialog),
      loading_settings_(false) {
  ui_->setupUi(this);
  ui_->list->setItemDelegate(new SettingsItemDelegate(this));

  SettingsCategory* general = new SettingsCategory(tr("General"), this);
  AddCategory(general);
  general->AddPage(Page_Playback, new PlaybackSettingsPage(this));
  general->AddPage(Page_Behaviour, new BehaviourSettingsPage(this));
  general->AddPage(Page_Library, new LibrarySettingsPage(this));
  general->AddPage(Page_SongMetadata, new SongMetadataSettingsPage(this));
  general->AddPage(Page_BackgroundStreams,
                   new BackgroundStreamsSettingsPage(this));
  general->AddPage(Page_Proxy, new NetworkProxySettingsPage(this));
  general->AddPage(Page_Transcoding, new TranscoderSettingsPage(this));
  general->AddPage(Page_NetworkRemote, new NetworkRemoteSettingsPage(this));

#ifdef HAVE_WIIMOTEDEV
  WiimoteSettingsPage* wii_page = new WiimoteSettingsPage(this);
  general->AddPage(Page_Wiimotedev, wii_page);
  connect(wii_page, SIGNAL(SetWiimotedevInterfaceActived(bool)),
          SIGNAL(SetWiimotedevInterfaceActived(bool)));
#endif

  // User interface
  SettingsCategory* iface = new SettingsCategory(tr("User interface"), this);
  AddCategory(iface);
  iface->AddPage(Page_GlobalShortcuts, new GlobalShortcutsSettingsPage(this));
  iface->AddPage(Page_GlobalSearch, new GlobalSearchSettingsPage(this));
  iface->AddPage(Page_Appearance, new AppearanceSettingsPage(this));
  iface->AddPage(Page_SongInformation, new SongInfoSettingsPage(this));

  NotificationsSettingsPage* notification_page =
      new NotificationsSettingsPage(this);
  iface->AddPage(Page_Notifications, notification_page);
  connect(notification_page,
          SIGNAL(NotificationPreview(OSD::Behaviour, QString, QString)),
          SIGNAL(NotificationPreview(OSD::Behaviour, QString, QString)));

  // Internet services category
  AddCategory(new InternetSettingsCategory(this));

  // List box
  connect(ui_->list,
          SIGNAL(currentItemChanged(QTreeWidgetItem*, QTreeWidgetItem*)),
          SLOT(CurrentItemChanged(QTreeWidgetItem*)));
  ui_->list->setCurrentItem(pages_[Page_Playback].item_);

  // Make sure the list is big enough to show all the items
  ui_->list->setMinimumWidth(
      static_cast<QAbstractItemView*>(ui_->list)->sizeHintForColumn(0));

  ui_->buttonBox->button(QDialogButtonBox::Cancel)
      ->setShortcut(QKeySequence::Close);
}

SettingsDialog::~SettingsDialog() { delete ui_; }

void SettingsDialog::AddCategory(SettingsCategory* category) {
  ui_->list->invisibleRootItem()->addChild(category);
  // This must not be called before it's added to the parent.
  category->setExpanded(true);
}

void SettingsDialog::AddPageToStack(Page id, SettingsPage* page,
                                    QTreeWidgetItem* item) {
  // Create a scroll area containing the page
  QScrollArea* area = new QScrollArea;
  area->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  area->setWidget(page);
  area->setWidgetResizable(true);
  area->setFrameShape(QFrame::NoFrame);
  area->setMinimumWidth(page->layout()->minimumSize().width());

  // Add the page to the stack
  ui_->stacked_widget->addWidget(area);

  // Remember where the page is
  PageData data;
  data.item_ = item;
  data.scroll_area_ = area;
  data.page_ = page;
  pages_[id] = data;
}

void SettingsDialog::accept() {
  for (const PageData& data : pages_.values()) {
    data.page_->Accept();
  }
  QDialog::accept();
}

void SettingsDialog::reject() {
  // Notify each page that user clicks on Cancel
  for (const PageData& data : pages_.values()) {
    data.page_->Reject();
  }

  QDialog::reject();
}

void SettingsDialog::DialogButtonClicked(QAbstractButton* button) {
  // While we only connect Apply at the moment, this might change in the future
  if (ui_->buttonBox->button(QDialogButtonBox::Apply) == button) {
    for (const PageData& data : pages_.values()) {
      data.page_->Apply();
    }
  }
}

void SettingsDialog::showEvent(QShowEvent* e) {
  // Load settings
  loading_settings_ = true;
  for (const PageData& data : pages_.values()) {
    data.page_->Load();
  }
  loading_settings_ = false;

  // Resize the dialog if it's too big
#if (QT_VERSION >= QT_VERSION_CHECK(5, 14, 0))
  QScreen* screen = QWidget::screen();
#else
  QScreen* screen =
      (window() && window()->windowHandle() ? window()->windowHandle()->screen()
                                            : QGuiApplication::primaryScreen());
#endif
  if (screen) {
    const QRect available = screen->availableGeometry();
    if (available.height() < height()) {
      resize(width(), sizeHint().height());
    }
  }

  QDialog::showEvent(e);
}

void SettingsDialog::OpenAtPage(Page page) {
  if (!pages_.contains(page)) {
    return;
  }

  ui_->list->setCurrentItem(pages_[page].item_);
  show();
}

void SettingsDialog::CurrentItemChanged(QTreeWidgetItem* item) {
  if (!(item->flags() & Qt::ItemIsSelectable)) {
    return;
  }

  // Set the title
  ui_->title->setText("<b>" + item->text(0) + "</b>");

  // Display the right page
  for (const PageData& data : pages_.values()) {
    if (data.item_ == item) {
      ui_->stacked_widget->setCurrentWidget(data.scroll_area_);
      return;
    }
  }
  qLog(Debug) << "Didn't find page for item!";
}
