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
#include "iconloader.h"
#include "mainwindow.h"
#include "settingsdialog.h"
#include "engines/enginebase.h"
#include "playlist/playlistview.h"
#include "songinfo/songinfotextview.h"
#include "widgets/osd.h"
#include "widgets/osdpretty.h"

#include "ui_settingsdialog.h"

#ifdef ENABLE_WIIMOTEDEV
#include "ui/wiimotedevshortcutsconfig.h"
#include "ui_wiimotedevshortcutsconfig.h"
#include "wiimotedev/shortcuts.h"
#endif

#ifdef HAVE_GSTREAMER
# include "engines/gstengine.h"
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

  // Icons
  ui_->list->item(Page_Playback)->setIcon(IconLoader::Load("media-playback-start"));
  ui_->list->item(Page_SongInformation)->setIcon(IconLoader::Load("view-media-lyrics"));
  ui_->list->item(Page_GlobalShortcuts)->setIcon(IconLoader::Load("input-keyboard"));
  ui_->list->item(Page_Notifications)->setIcon(IconLoader::Load("help-hint"));
  ui_->list->item(Page_Library)->setIcon(IconLoader::Load("folder-sound"));
  ui_->list->item(Page_BackgroundStreams)->setIcon(QIcon(":/icons/32x32/weather-showers-scattered.png"));

  AddStreams();

#ifdef ENABLE_WIIMOTEDEV
  // Wiimotedev page
  ui_->list->addItem("Wiimotedev");
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
#ifdef HAVE_GSTREAMER
  connect(ui_->gst_plugin, SIGNAL(currentIndexChanged(int)), SLOT(GstPluginChanged(int)));
#endif

  connect(ui_->replaygain_preamp, SIGNAL(valueChanged(int)), SLOT(RgPreampChanged(int)));
  ui_->replaygain_preamp_label->setMinimumWidth(
      QFontMetrics(ui_->replaygain_preamp_label->font()).width("-WW.W dB"));
  RgPreampChanged(ui_->replaygain_preamp->value());

  // Behaviour
  connect(ui_->b_show_tray_icon_, SIGNAL(toggled(bool)), SLOT(ShowTrayIconToggled(bool)));

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

  // Last.fm
  connect(ui_->lastfm, SIGNAL(ValidationComplete(bool)), SLOT(LastFMValidationComplete(bool)));

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

void SettingsDialog::LastFMValidationComplete(bool success) {
  ui_->buttonBox->setEnabled(true);

  if (success)
    accept();
}

void SettingsDialog::accept() {
  if (ui_->lastfm->NeedsValidation()) {
    ui_->lastfm->Validate();
    ui_->buttonBox->setEnabled(false);
    return;
  } else {
    ui_->lastfm->Save();
  }

  QSettings s;

  // Behaviour
  MainWindow::StartupBehaviour behaviour;
  if (ui_->b_always_hide_->isChecked()) behaviour = MainWindow::Startup_AlwaysHide;
  if (ui_->b_always_show_->isChecked()) behaviour = MainWindow::Startup_AlwaysShow;
  if (ui_->b_remember_->isChecked())    behaviour = MainWindow::Startup_Remember;

  s.beginGroup(MainWindow::kSettingsGroup);
  s.setValue("showtray", ui_->b_show_tray_icon_->isChecked());
  s.setValue("keeprunning", ui_->b_keep_running_->isChecked());
  s.setValue("startupbehaviour", int(behaviour));
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
  s.endGroup();

#ifdef HAVE_GSTREAMER
  s.beginGroup(GstEngine::kSettingsGroup);
  s.setValue("sink", ui_->gst_plugin->itemData(ui_->gst_plugin->currentIndex()).toString());
  s.setValue("device", ui_->gst_device->text());
  s.setValue("rgenabled", ui_->replaygain->isChecked());
  s.setValue("rgmode", ui_->replaygain_mode->currentIndex());
  s.setValue("rgpreamp", float(ui_->replaygain_preamp->value()) / 10 - 15);
  s.setValue("rgcompression", ui_->replaygain_compression->isChecked());
  s.setValue("bufferduration", ui_->buffer_duration->value());
  s.endGroup();
#endif

  // Song info
  s.beginGroup(SongInfoTextView::kSettingsGroup);
  s.setValue("font_size", ui_->song_info_font_preview->font().pointSizeF());
  s.endGroup();

  ui_->lyric_settings->Save();

  // Wii remotes
#ifdef ENABLE_WIIMOTEDEV
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

  ui_->library_config->Save();
  ui_->magnatune->Save();
  ui_->global_shortcuts->Save();

  streams_->SaveStreams();

  QDialog::accept();
}

void SettingsDialog::showEvent(QShowEvent*) {
  QSettings s;
  loading_settings_ = true;

#ifdef HAVE_GSTREAMER
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
#endif // HAVE_GSTREAMER

  // Behaviour
  s.beginGroup(MainWindow::kSettingsGroup);
  ui_->b_show_tray_icon_->setChecked(s.value("showtray", true).toBool());
  ui_->b_keep_running_->setChecked(s.value("keeprunning",
      ui_->b_show_tray_icon_->isChecked()).toBool());

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
  s.endGroup();

  ui_->lyric_settings->Load();

  // Last.fm
  ui_->lastfm->Load();

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
  s.endGroup();

#ifdef HAVE_GSTREAMER
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
#endif

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
#ifdef HAVE_GSTREAMER
  QString name = ui_->gst_plugin->itemData(index).toString();

  bool enabled = GstEngine::DoesThisSinkSupportChangingTheOutputDeviceToAUserEditableString(name);

  ui_->gst_device->setEnabled(enabled);
  ui_->gst_device_label->setEnabled(enabled);
#endif // HAVE_GSTREAMER
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

void SettingsDialog::SongInfoFontSizeChanged(qreal value) {
  QFont font;
  font.setPointSizeF(value);

  ui_->song_info_font_preview->setFont(font);
}
