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

#include "config.h"
#include "core/backgroundstreams.h"
#include "core/networkproxyfactory.h"
#include "iconloader.h"
#include "mainwindow.h"
#include "settingsdialog.h"
#include "engines/enginebase.h"
#include "engines/gstengine.h"
#include "playlist/playlistview.h"
#include "songinfo/songinfofetcher.h"
#include "songinfo/songinfotextview.h"
#include "widgets/osd.h"
#include "widgets/osdpretty.h"

#include "ui_settingsdialog.h"

#ifdef HAVE_LIBLASTFM
# include "radio/lastfmconfig.h"
#endif

#ifdef HAVE_WIIMOTEDEV
# include "ui/wiimotedevshortcutsconfig.h"
# include "ui_wiimotedevshortcutsconfig.h"
# include "wiimotedev/shortcuts.h"
#endif

#ifdef HAVE_REMOTE
# include "remote/remoteconfig.h"
#endif

#include <QColorDialog>
#include <QDir>
#include <QFontDialog>
#include <QSettings>

#include <QtDebug>

void SettingsDialog::AddStream(const QString& name) {
  QGroupBox* box = new QGroupBox(tr(name.toUtf8()));
  QSlider* slider = new QSlider(Qt::Horizontal, box);
  QCheckBox* check = new QCheckBox(box);
  QHBoxLayout* layout = new QHBoxLayout(box);
  layout->addWidget(slider);
  layout->addWidget(check);

  QVBoxLayout* streams_layout = qobject_cast<QVBoxLayout*>(
        ui_->streams_page->layout());
  streams_layout->insertWidget(streams_layout->count() - 1, box);

  slider->setProperty("stream_name", name);
  check->setProperty("stream_name", name);

  connect(slider, SIGNAL(valueChanged(int)), SLOT(StreamVolumeChanged(int)));
  connect(check, SIGNAL(toggled(bool)), SLOT(EnableStream(bool)));

  slider->setValue(streams_->GetStreamVolume(name));
  check->setCheckState(streams_->IsPlaying(name) ? Qt::Checked : Qt::Unchecked);
}

void SettingsDialog::EnableStream(bool enabled) {
  const QString name = sender()->property("stream_name").toString();
  streams_->EnableStream(name, enabled);
}

void SettingsDialog::StreamVolumeChanged(int value) {
  const QString name = sender()->property("stream_name").toString();
  streams_->SetStreamVolume(name, value);
}

void SettingsDialog::AddStreams() {
  foreach (const QString& name, streams_->streams()) {
    AddStream(name);
  }
}

SettingsDialog::SettingsDialog(BackgroundStreams* streams, QWidget* parent)
  : QDialog(parent),
    gst_engine_(NULL),
    ui_(new Ui_SettingsDialog),
    loading_settings_(false),
    pretty_popup_(new OSDPretty(OSDPretty::Mode_Draggable)),
    streams_(streams)
{
  ui_->setupUi(this);
  pretty_popup_->SetMessage(tr("OSD Preview"), tr("Drag to reposition"),
                            QImage(":nocover.png"));

#ifdef HAVE_LIBLASTFM
  ui_->list->insertItem(Page_Lastfm, tr("Last.fm"));
  ui_->list->item(Page_Lastfm)->setIcon(QIcon(":/last.fm/as.png"));

  QWidget* lastfm_page = new QWidget;
  QVBoxLayout* lastfm_layout = new QVBoxLayout;
  lastfm_layout->setContentsMargins(0, 0, 0, 0);
  lastfm_config_ = new LastFMConfig;
  lastfm_layout->addWidget(lastfm_config_);
  lastfm_page->setLayout(lastfm_layout);

  ui_->stacked_widget->insertWidget(Page_Lastfm, lastfm_page);

  connect(lastfm_config_, SIGNAL(ValidationComplete(bool)), SLOT(ValidationComplete(bool)));
#endif

#ifdef HAVE_REMOTE
  ui_->list->insertItem(Page_Remote, tr("Remote Control"));
  ui_->list->item(Page_Remote)->setIcon(IconLoader::Load("network-server"));

  QWidget* remote_page = new QWidget;
  QVBoxLayout* remote_layout = new QVBoxLayout;
  remote_layout->setContentsMargins(0, 0, 0, 0);

  remote_config_ = new RemoteConfig;
  remote_layout->addWidget(remote_config_);
  remote_page->setLayout(remote_layout);

  ui_->stacked_widget->insertWidget(Page_Remote, remote_page);

  connect(remote_config_, SIGNAL(ValidationComplete(bool)), SLOT(ValidationComplete(bool)));
#endif

  // Icons
  ui_->list->item(Page_Playback)->setIcon(IconLoader::Load("media-playback-start"));
  ui_->list->item(Page_SongInformation)->setIcon(IconLoader::Load("view-media-lyrics"));
  ui_->list->item(Page_GlobalShortcuts)->setIcon(IconLoader::Load("input-keyboard"));
  ui_->list->item(Page_Notifications)->setIcon(IconLoader::Load("help-hint"));
  ui_->list->item(Page_Library)->setIcon(IconLoader::Load("folder-sound"));
  ui_->list->item(Page_BackgroundStreams)->setIcon(QIcon(":/icons/32x32/weather-showers-scattered.png"));
  ui_->list->item(Page_Proxy)->setIcon(IconLoader::Load("applications-internet"));

  AddStreams();

#ifdef HAVE_WIIMOTEDEV
  // Wiimotedev page
  ui_->list->insertItem(Page_Wiimotedev, "Wiimotedev");
  ui_->list->item(Page_Wiimotedev)->setIcon(QIcon(":/icons/32x32/wiimotedev.png"));

  QWidget* wiimotedev_page = new QWidget(this);
  wiimotedev_page->setObjectName(QString::fromUtf8("wiimotedev_page"));
  QVBoxLayout* wiimotedev_layout = new QVBoxLayout(wiimotedev_page);
  wiimotedev_layout->setObjectName(QString::fromUtf8("wiimotedev_layout"));
  wiimotedev_config_ = new WiimotedevShortcutsConfig(wiimotedev_page);
  wiimotedev_config_->setObjectName(QString::fromUtf8("wiimotedev_config"));
  wiimotedev_layout->addWidget(wiimotedev_config_);

  ui_->stacked_widget->addWidget(wiimotedev_page);

  connect(wiimotedev_config_, SIGNAL(SetWiimotedevInterfaceActived(bool)), this, SIGNAL(SetWiimotedevInterfaceActived(bool)));
#endif

  // Playback
  connect(ui_->fading_cross, SIGNAL(toggled(bool)), SLOT(FadingOptionsChanged()));
  connect(ui_->fading_out, SIGNAL(toggled(bool)), SLOT(FadingOptionsChanged()));
  connect(ui_->fading_auto, SIGNAL(toggled(bool)), SLOT(FadingOptionsChanged()));
  connect(ui_->gst_plugin, SIGNAL(currentIndexChanged(int)), SLOT(GstPluginChanged(int)));

  connect(ui_->replaygain_preamp, SIGNAL(valueChanged(int)), SLOT(RgPreampChanged(int)));
  ui_->replaygain_preamp_label->setMinimumWidth(
      QFontMetrics(ui_->replaygain_preamp_label->font()).width("-WW.W dB"));
  RgPreampChanged(ui_->replaygain_preamp->value());

  // Behaviour
  connect(ui_->b_show_tray_icon_, SIGNAL(toggled(bool)), SLOT(ShowTrayIconToggled(bool)));

  ui_->doubleclick_addmode->setItemData(0, MainWindow::AddBehaviour_Append);
  ui_->doubleclick_addmode->setItemData(1, MainWindow::AddBehaviour_Load);
  ui_->doubleclick_addmode->setItemData(2, MainWindow::AddBehaviour_OpenInNew);
  ui_->doubleclick_addmode->setItemData(3, MainWindow::AddBehaviour_Enqueue);

  ui_->doubleclick_playmode->setItemData(0, MainWindow::PlayBehaviour_Never);
  ui_->doubleclick_playmode->setItemData(1, MainWindow::PlayBehaviour_IfStopped);
  ui_->doubleclick_playmode->setItemData(2, MainWindow::PlayBehaviour_Always);

  ui_->menu_playmode->setItemData(0, MainWindow::PlayBehaviour_Never);
  ui_->menu_playmode->setItemData(1, MainWindow::PlayBehaviour_IfStopped);
  ui_->menu_playmode->setItemData(2, MainWindow::PlayBehaviour_Always);

  // Populate the language combo box.  We do this by looking at all the
  // compiled in translations.
  QDir dir(":/translations/");
  QStringList codes(dir.entryList(QStringList() << "*.qm"));
  QRegExp lang_re("^clementine_(.*).qm$");
  foreach (const QString& filename, codes) {
    // The regex captures the "ru" from "clementine_ru.qm"
    if (!lang_re.exactMatch(filename))
      continue;

    QString code = lang_re.cap(1);
    QString name = QString("%1 (%2)").arg(
        QLocale::languageToString(QLocale(code).language()), code);

    language_map_[name] = code;
  }

  language_map_["English (en)"] = "en";

  // Sort the names and show them in the UI
  QStringList names = language_map_.keys();
  qStableSort(names);
  ui_->language->addItems(names);

  // Song info
  QFile song_info_preview(":/lumberjacksong.txt");
  song_info_preview.open(QIODevice::ReadOnly);
  ui_->song_info_font_preview->setText(QString::fromUtf8(song_info_preview.readAll()));

  connect(ui_->song_info_font_size, SIGNAL(valueChanged(double)), SLOT(SongInfoFontSizeChanged(double)));

  // Global shortcuts
#ifdef Q_OS_MAC
  if (QSysInfo::MacintoshVersion != QSysInfo::MV_SNOWLEOPARD) {
    ui_->list->item(Page_GlobalShortcuts)->setFlags(Qt::NoItemFlags);
  }
#endif

  // List box
  connect(ui_->list, SIGNAL(currentTextChanged(QString)), SLOT(CurrentTextChanged(QString)));
  ui_->list->setCurrentRow(Page_Playback);

  // Notifications
  ui_->notifications_bg_preset->setItemData(0, QColor(OSDPretty::kPresetBlue), Qt::DecorationRole);
  ui_->notifications_bg_preset->setItemData(1, QColor(OSDPretty::kPresetOrange), Qt::DecorationRole);

  connect(ui_->notifications_none, SIGNAL(toggled(bool)), SLOT(NotificationTypeChanged()));
  connect(ui_->notifications_native, SIGNAL(toggled(bool)), SLOT(NotificationTypeChanged()));
  connect(ui_->notifications_tray, SIGNAL(toggled(bool)), SLOT(NotificationTypeChanged()));
  connect(ui_->notifications_pretty, SIGNAL(toggled(bool)), SLOT(NotificationTypeChanged()));
  connect(ui_->notifications_opacity, SIGNAL(valueChanged(int)), SLOT(PrettyOpacityChanged(int)));
  connect(ui_->notifications_bg_preset, SIGNAL(activated(int)), SLOT(PrettyColorPresetChanged(int)));
  connect(ui_->notifications_fg_choose, SIGNAL(clicked()), SLOT(ChooseFgColor()));

  if (!OSD::SupportsNativeNotifications())
    ui_->notifications_native->setEnabled(false);
  if (!OSD::SupportsTrayPopups())
    ui_->notifications_tray->setEnabled(false);

  connect(ui_->stacked_widget, SIGNAL(currentChanged(int)), SLOT(UpdatePopupVisible()));
  connect(ui_->notifications_pretty, SIGNAL(toggled(bool)), SLOT(UpdatePopupVisible()));

  // Make sure the list is big enough to show all the items
  ui_->list->setMinimumWidth(ui_->list->sizeHintForColumn(0));

#ifdef Q_OS_DARWIN
  ui_->b_show_tray_icon_->setEnabled(false);
  ui_->startup_group_->setEnabled(false);
#endif

  ui_->buttonBox->button(QDialogButtonBox::Cancel)->setShortcut(QKeySequence::Close);
}

SettingsDialog::~SettingsDialog() {
  delete pretty_popup_;
  delete ui_;
}

void SettingsDialog::CurrentTextChanged(const QString &text) {
  ui_->title->setText("<b>" + text + "</b>");
}

void SettingsDialog::SetLibraryDirectoryModel(LibraryDirectoryModel* model) {
  ui_->library_config->SetModel(model);
}

void SettingsDialog::SetGlobalShortcutManager(GlobalShortcuts *manager) {
  ui_->global_shortcuts->SetManager(manager);
}

void SettingsDialog::ValidationComplete(bool success) {
  ui_->buttonBox->setEnabled(true);

  if (success)
    accept();
}

void SettingsDialog::accept() {
#ifdef HAVE_LIBLASTFM
  if (lastfm_config_->NeedsValidation()) {
    lastfm_config_->Validate();
    ui_->buttonBox->setEnabled(false);
    return;
  } else {
    lastfm_config_->Save();
  }
#endif

#ifdef HAVE_REMOTE
  if (remote_config_->NeedsValidation()) {
    remote_config_->Validate();
    ui_->buttonBox->setEnabled(false);
    return;
  } else {
    remote_config_->Save();
  }
#endif

  QSettings s;

  // Behaviour
  MainWindow::StartupBehaviour behaviour = MainWindow::Startup_Remember;
  if (ui_->b_always_hide_->isChecked()) behaviour = MainWindow::Startup_AlwaysHide;
  if (ui_->b_always_show_->isChecked()) behaviour = MainWindow::Startup_AlwaysShow;
  if (ui_->b_remember_->isChecked())    behaviour = MainWindow::Startup_Remember;

  MainWindow::AddBehaviour doubleclick_addmode = MainWindow::AddBehaviour(
    ui_->doubleclick_addmode->itemData(ui_->doubleclick_addmode->currentIndex()).toInt());
  MainWindow::PlayBehaviour doubleclick_playmode = MainWindow::PlayBehaviour(
    ui_->doubleclick_playmode->itemData(ui_->doubleclick_playmode->currentIndex()).toInt());
  MainWindow::PlayBehaviour menu_playmode = MainWindow::PlayBehaviour(
    ui_->menu_playmode->itemData(ui_->menu_playmode->currentIndex()).toInt());

  s.beginGroup(MainWindow::kSettingsGroup);
  s.setValue("showtray", ui_->b_show_tray_icon_->isChecked());
  s.setValue("keeprunning", ui_->b_keep_running_->isChecked());
  s.setValue("startupbehaviour", int(behaviour));
  s.setValue("doubleclick_addmode", doubleclick_addmode);
  s.setValue("doubleclick_playmode", doubleclick_playmode);
  s.setValue("menu_playmode", menu_playmode);
  s.endGroup();

  s.beginGroup("General");
  s.setValue("language", language_map_.contains(ui_->language->currentText()) ?
             language_map_[ui_->language->currentText()] : QString());
  s.endGroup();

  // Playback
  s.beginGroup(PlaylistView::kSettingsGroup);
  s.setValue("glow_effect", ui_->current_glow->isChecked());
  s.endGroup();

  s.beginGroup(Engine::Base::kSettingsGroup);
  s.setValue("FadeoutEnabled", ui_->fading_out->isChecked());
  s.setValue("FadeoutDuration", ui_->fading_duration->value());
  s.setValue("CrossfadeEnabled", ui_->fading_cross->isChecked());
  s.setValue("AutoCrossfadeEnabled", ui_->fading_auto->isChecked());
  s.setValue("NoCrossfadeSameAlbum", ui_->fading_samealbum->isChecked());
  s.endGroup();

  s.beginGroup(GstEngine::kSettingsGroup);
  s.setValue("sink", ui_->gst_plugin->itemData(ui_->gst_plugin->currentIndex()).toString());
  s.setValue("device", ui_->gst_device->text());
  s.setValue("rgenabled", ui_->replaygain->isChecked());
  s.setValue("rgmode", ui_->replaygain_mode->currentIndex());
  s.setValue("rgpreamp", float(ui_->replaygain_preamp->value()) / 10 - 15);
  s.setValue("rgcompression", ui_->replaygain_compression->isChecked());
  s.setValue("bufferduration", ui_->buffer_duration->value());
  s.endGroup();

  // Song info
  s.beginGroup(SongInfoTextView::kSettingsGroup);
  s.setValue("font_size", ui_->song_info_font_preview->font().pointSizeF());
  s.setValue("timeout", ui_->song_info_timeout->value());
  s.endGroup();

  ui_->lyric_settings->Save();

  // Wii remotes
#ifdef HAVE_WIIMOTEDEV
  s.beginGroup(WiimotedevShortcuts::kActionsGroup);
  s.remove("");
  foreach (const WiimotedevShortcutsConfig::Shortcut& shortcut, wiimotedev_config_->actions_)
    s.setValue(QString::number(shortcut.button), shortcut.action);
  s.endGroup();

  s.beginGroup(WiimotedevShortcuts::kSettingsGroup);
  s.setValue("first_conf", false);
  s.setValue("enabled", wiimotedev_config_->ui_->wiimotedev_enable->isChecked());
  s.setValue("only_when_focused", wiimotedev_config_->ui_->wiimotedev_focus->isChecked());
  s.setValue("use_active_action", wiimotedev_config_->ui_->wiimotedev_active->isChecked());
  s.setValue("use_notification", wiimotedev_config_->ui_->wiimotedev_notification->isChecked());
  s.setValue("device", wiimotedev_config_->ui_->wiimotedev_device->value());
  s.endGroup();
#endif

  // Notifications
  OSD::Behaviour osd_behaviour = OSD::Disabled;
  if      (ui_->notifications_none->isChecked())   osd_behaviour = OSD::Disabled;
  else if (ui_->notifications_native->isChecked()) osd_behaviour = OSD::Native;
  else if (ui_->notifications_tray->isChecked())   osd_behaviour = OSD::TrayPopup;
  else if (ui_->notifications_pretty->isChecked()) osd_behaviour = OSD::Pretty;

  s.beginGroup(OSD::kSettingsGroup);
  s.setValue("Behaviour", int(osd_behaviour));
  s.setValue("Timeout", ui_->notifications_duration->value() * 1000);
  s.setValue("ShowOnVolumeChange", ui_->notifications_volume->isChecked());
  s.setValue("ShowArt", ui_->notifications_art->isChecked());
  s.endGroup();

  s.beginGroup(OSDPretty::kSettingsGroup);
  s.setValue("foreground_color", pretty_popup_->foreground_color());
  s.setValue("background_color", pretty_popup_->background_color());
  s.setValue("background_opacity", pretty_popup_->background_opacity());
  s.setValue("popup_display", pretty_popup_->popup_display());
  s.setValue("popup_pos", pretty_popup_->popup_pos());
  s.endGroup();

  // Network proxy
  NetworkProxyFactory::Mode mode = NetworkProxyFactory::Mode_System;
  if      (ui_->proxy_direct->isChecked()) mode = NetworkProxyFactory::Mode_Direct;
  else if (ui_->proxy_system->isChecked()) mode = NetworkProxyFactory::Mode_System;
  else if (ui_->proxy_manual->isChecked()) mode = NetworkProxyFactory::Mode_Manual;

  s.beginGroup(NetworkProxyFactory::kSettingsGroup);
  s.setValue("mode", mode);
  s.setValue("type", ui_->proxy_type->currentIndex() == 0 ?
             QNetworkProxy::HttpProxy : QNetworkProxy::Socks5Proxy);
  s.setValue("hostname", ui_->proxy_hostname->text());
  s.setValue("port", ui_->proxy_port->value());
  s.setValue("use_authentication", ui_->proxy_auth->isChecked());
  s.setValue("username", ui_->proxy_username->text());
  s.setValue("password", ui_->proxy_password->text());
  s.endGroup();

  ui_->library_config->Save();
  ui_->magnatune->Save();
  ui_->global_shortcuts->Save();

  streams_->SaveStreams();

  QDialog::accept();

  NetworkProxyFactory::Instance()->ReloadSettings();
}

void SettingsDialog::showEvent(QShowEvent*) {
  QSettings s;
  loading_settings_ = true;

  if (ui_->gst_plugin->count() <= 1 && gst_engine_) {
    GstEngine::PluginDetailsList list = gst_engine_->GetOutputsList();

    ui_->gst_plugin->setItemData(0, GstEngine::kAutoSink);
    foreach (const GstEngine::PluginDetails& details, list) {
      if (details.name == "autoaudiosink")
        continue;

      ui_->gst_plugin->addItem(details.long_name, details.name);
    }
    ui_->gst_group->setEnabled(true);
    ui_->replaygain_group->setEnabled(true);
  }

  // Behaviour
  s.beginGroup(MainWindow::kSettingsGroup);
  ui_->b_show_tray_icon_->setChecked(s.value("showtray", true).toBool());
  ui_->b_keep_running_->setChecked(s.value("keeprunning",
      ui_->b_show_tray_icon_->isChecked()).toBool());
  ui_->doubleclick_addmode->setCurrentIndex(ui_->doubleclick_addmode->findData(
      s.value("doubleclick_addmode", MainWindow::AddBehaviour_Append).toInt()));
  ui_->doubleclick_playmode->setCurrentIndex(ui_->doubleclick_playmode->findData(
      s.value("doubleclick_playmode", MainWindow::PlayBehaviour_IfStopped).toInt()));
  ui_->menu_playmode->setCurrentIndex(ui_->menu_playmode->findData(
      s.value("menu_playmode", MainWindow::PlayBehaviour_IfStopped).toInt()));

  MainWindow::StartupBehaviour behaviour = MainWindow::StartupBehaviour(
      s.value("startupbehaviour", MainWindow::Startup_Remember).toInt());
  switch (behaviour) {
    case MainWindow::Startup_AlwaysHide: ui_->b_always_hide_->setChecked(true); break;
    case MainWindow::Startup_AlwaysShow: ui_->b_always_show_->setChecked(true); break;
    case MainWindow::Startup_Remember:   ui_->b_remember_->setChecked(true);    break;
  }
  s.endGroup();

  s.beginGroup("General");
  QString name = language_map_.key(s.value("language").toString());
  if (name.isEmpty())
    ui_->language->setCurrentIndex(0);
  else
    ui_->language->setCurrentIndex(ui_->language->findText(name));
  s.endGroup();

  // Song Info
  s.beginGroup(SongInfoTextView::kSettingsGroup);
  ui_->song_info_font_size->setValue(
      s.value("font_size", SongInfoTextView::kDefaultFontSize).toReal());
  ui_->song_info_timeout->setValue(
      s.value("timeout", SongInfoFetcher::kDefaultTimeoutDuration).toInt());
  s.endGroup();

  ui_->lyric_settings->Load();

  // Last.fm
#ifdef HAVE_LIBLASTFM
  lastfm_config_->Load();
#endif

#ifdef HAVE_REMOTE
  remote_config_->Load();
#endif

  // Magnatune
  ui_->magnatune->Load();

  // Global Shortcuts
  ui_->global_shortcuts->Load();

  // Playback
  s.beginGroup(PlaylistView::kSettingsGroup);
  ui_->current_glow->setChecked(s.value("glow_effect", true).toBool());
  s.endGroup();

  s.beginGroup(Engine::Base::kSettingsGroup);
  ui_->fading_out->setChecked(s.value("FadeoutEnabled", true).toBool());
  ui_->fading_cross->setChecked(s.value("CrossfadeEnabled", true).toBool());
  ui_->fading_auto->setChecked(s.value("AutoCrossfadeEnabled", false).toBool());
  ui_->fading_duration->setValue(s.value("FadeoutDuration", 2000).toInt());
  ui_->fading_samealbum->setChecked(s.value("NoCrossfadeSameAlbum", true).toBool());
  s.endGroup();

  s.beginGroup(GstEngine::kSettingsGroup);
  QString sink = s.value("sink", GstEngine::kAutoSink).toString();
  ui_->gst_plugin->setCurrentIndex(0);
  for (int i=0 ; i<ui_->gst_plugin->count() ; ++i) {
    if (ui_->gst_plugin->itemData(i).toString() == sink) {
      ui_->gst_plugin->setCurrentIndex(i);
      break;
    }
  }
  ui_->gst_device->setText(s.value("device").toString());
  ui_->replaygain->setChecked(s.value("rgenabled", false).toBool());
  ui_->replaygain_mode->setCurrentIndex(s.value("rgmode", 0).toInt());
  ui_->replaygain_preamp->setValue(s.value("rgpreamp", 0.0).toDouble() * 10 + 150);
  ui_->replaygain_compression->setChecked(s.value("rgcompression", true).toBool());
  ui_->buffer_duration->setValue(s.value("bufferduration", 1000).toInt());
  s.endGroup();

  // Notifications
  s.beginGroup(OSD::kSettingsGroup);
  OSD::Behaviour osd_behaviour = OSD::Behaviour(s.value("Behaviour", OSD::Native).toInt());
  switch (osd_behaviour) {
    case OSD::Native:
      if (OSD::SupportsNativeNotifications()) {
        ui_->notifications_native->setChecked(true);
        break;
      }
      // Fallthrough

    case OSD::Pretty:
      ui_->notifications_pretty->setChecked(true);
      break;

    case OSD::TrayPopup:
      if (OSD::SupportsTrayPopups()) {
        ui_->notifications_tray->setChecked(true);
        break;
      }
      // Fallthrough

    case OSD::Disabled:
    default:
      ui_->notifications_none->setChecked(true);
      break;
  }
  ui_->notifications_duration->setValue(s.value("Timeout", 5000).toInt() / 1000);
  ui_->notifications_volume->setChecked(s.value("ShowOnVolumeChange", false).toBool());
  ui_->notifications_art->setChecked(s.value("ShowArt", true).toBool());
  s.endGroup();

  // Pretty OSD
  pretty_popup_->ReloadSettings();
  ui_->notifications_opacity->setValue(pretty_popup_->background_opacity() * 100);

  QRgb color = pretty_popup_->background_color();
  if (color == OSDPretty::kPresetBlue)
    ui_->notifications_bg_preset->setCurrentIndex(0);
  else if (color == OSDPretty::kPresetOrange)
    ui_->notifications_bg_preset->setCurrentIndex(1);
  else
    ui_->notifications_bg_preset->setCurrentIndex(2);
  ui_->notifications_bg_preset->setItemData(2, QColor(color), Qt::DecorationRole);
  UpdatePopupVisible();

  ui_->library_config->Load();

  // Network proxy
  s.beginGroup(NetworkProxyFactory::kSettingsGroup);
  NetworkProxyFactory::Mode mode = NetworkProxyFactory::Mode(
      s.value("mode", NetworkProxyFactory::Mode_System).toInt());
  switch (mode) {
  case NetworkProxyFactory::Mode_Manual:
    ui_->proxy_manual->setChecked(true);
    break;

  case NetworkProxyFactory::Mode_Direct:
    ui_->proxy_direct->setChecked(true);
    break;

  case NetworkProxyFactory::Mode_System:
  default:
    ui_->proxy_system->setChecked(true);
    break;
  }

  ui_->proxy_type->setCurrentIndex(s.value("type", QNetworkProxy::HttpProxy)
      .toInt() == QNetworkProxy::HttpProxy ? 0 : 1);
  ui_->proxy_hostname->setText(s.value("hostname").toString());
  ui_->proxy_port->setValue(s.value("port").toInt());
  ui_->proxy_auth->setChecked(s.value("use_authentication", false).toBool());
  ui_->proxy_username->setText(s.value("username").toString());
  ui_->proxy_password->setText(s.value("password").toString());
  s.endGroup();

  loading_settings_ = false;
}

void SettingsDialog::hideEvent(QHideEvent *) {
  pretty_popup_->hide();
}

void SettingsDialog::NotificationTypeChanged() {
  bool enabled = !ui_->notifications_none->isChecked();
  bool pretty = ui_->notifications_pretty->isChecked();

  ui_->notifications_general->setEnabled(enabled);
  ui_->notifications_pretty_group->setEnabled(pretty);
}

void SettingsDialog::PrettyOpacityChanged(int value) {
  pretty_popup_->set_background_opacity(qreal(value) / 100.0);
}

void SettingsDialog::UpdatePopupVisible() {
  pretty_popup_->setVisible(
      isVisible() &&
      ui_->notifications_pretty->isChecked() &&
      ui_->stacked_widget->currentWidget() == ui_->notifications_page);
}

void SettingsDialog::PrettyColorPresetChanged(int index) {
  if (loading_settings_)
    return;

  switch (index) {
  case 0:
    pretty_popup_->set_background_color(OSDPretty::kPresetBlue);
    break;

  case 1:
    pretty_popup_->set_background_color(OSDPretty::kPresetOrange);
    break;

  case 2:
  default:
    ChooseBgColor();
    break;
  }
}

void SettingsDialog::ChooseBgColor() {
  QColor color = QColorDialog::getColor(pretty_popup_->background_color(), this);
  if (!color.isValid())
    return;

  pretty_popup_->set_background_color(color.rgb());
  ui_->notifications_bg_preset->setItemData(2, color, Qt::DecorationRole);
}

void SettingsDialog::ChooseFgColor() {
  QColor color = QColorDialog::getColor(pretty_popup_->foreground_color(), this);
  if (!color.isValid())
    return;

  pretty_popup_->set_foreground_color(color.rgb());
}

void SettingsDialog::ShowTrayIconToggled(bool on) {
  ui_->b_always_hide_->setEnabled(on);
  if (!on && ui_->b_always_hide_->isChecked())
    ui_->b_remember_->setChecked(true);
}

void SettingsDialog::GstPluginChanged(int index) {
  QString name = ui_->gst_plugin->itemData(index).toString();

  bool enabled = GstEngine::DoesThisSinkSupportChangingTheOutputDeviceToAUserEditableString(name);

  ui_->gst_device->setEnabled(enabled);
  ui_->gst_device_label->setEnabled(enabled);
}

void SettingsDialog::RgPreampChanged(int value) {
  float db = float(value) / 10 - 15;
  QString db_str;
  db_str.sprintf("%+.1f dB", db);
  ui_->replaygain_preamp_label->setText(db_str);
}

void SettingsDialog::FadingOptionsChanged() {
  ui_->fading_options->setEnabled(
      ui_->fading_out->isChecked() || ui_->fading_cross->isChecked() ||
      ui_->fading_auto->isChecked());
}

void SettingsDialog::OpenAtPage(Page page) {
  ui_->list->setCurrentRow(page);
  show();
}

void SettingsDialog::SetSongInfoView(SongInfoView* view) {
  ui_->lyric_settings->SetSongInfoView(view);
}

void SettingsDialog::SongInfoFontSizeChanged(double value) {
  QFont font;
  font.setPointSizeF(value);

  ui_->song_info_font_preview->setFont(font);
}
