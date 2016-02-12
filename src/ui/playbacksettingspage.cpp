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

#include "iconloader.h"
#include "playbacksettingspage.h"
#include "settingsdialog.h"
#include "ui_playbacksettingspage.h"
#include "engines/gstengine.h"
#include "playlist/playlist.h"

PlaybackSettingsPage::PlaybackSettingsPage(SettingsDialog* dialog)
    : SettingsPage(dialog), ui_(new Ui_PlaybackSettingsPage) {
  ui_->setupUi(this);
  setWindowIcon(IconLoader::Load("media-playback-start", IconLoader::Base));

  connect(ui_->fading_cross, SIGNAL(toggled(bool)),
          SLOT(FadingOptionsChanged()));
  connect(ui_->fading_out, SIGNAL(toggled(bool)), SLOT(FadingOptionsChanged()));
  connect(ui_->fading_auto, SIGNAL(toggled(bool)),
          SLOT(FadingOptionsChanged()));

  connect(ui_->buffer_min_fill, SIGNAL(valueChanged(int)),
          SLOT(BufferMinFillChanged(int)));
  ui_->buffer_min_fill_value_label->setMinimumWidth(
      QFontMetrics(ui_->buffer_min_fill_value_label->font()).width("WW%"));

  connect(ui_->replaygain_preamp, SIGNAL(valueChanged(int)),
          SLOT(RgPreampChanged(int)));
  ui_->replaygain_preamp_label->setMinimumWidth(
      QFontMetrics(ui_->replaygain_preamp_label->font()).width("-WW.W dB"));
  RgPreampChanged(ui_->replaygain_preamp->value());

  ui_->sample_rate->setItemData(0, GstEngine::kAutoSampleRate);
  ui_->sample_rate->setItemData(1, 44100);
  ui_->sample_rate->setItemData(2, 48000);
  ui_->sample_rate->setItemData(3, 96000);
  ui_->sample_rate->setItemData(4, 192000);
}

PlaybackSettingsPage::~PlaybackSettingsPage() { delete ui_; }

void PlaybackSettingsPage::Load() {
  const GstEngine* engine = dialog()->gst_engine();

  ui_->gst_output->clear();
  for (const GstEngine::OutputDetails& output : engine->GetOutputsList()) {
    // Strip components off the icon name until we find one.
    QStringList components = output.icon_name.split("-");
    QIcon icon;
    while (icon.isNull() && !components.isEmpty()) {
      icon = IconLoader::Load(components.join("-"), IconLoader::Base);
      components.removeLast();
    }

    ui_->gst_output->addItem(icon, output.description,
                             QVariant::fromValue(output));
  }

  QSettings s;

  s.beginGroup(Playlist::kSettingsGroup);
  ui_->current_glow->setChecked(s.value("glow_effect", true).toBool());
  s.endGroup();

  s.beginGroup(Engine::Base::kSettingsGroup);
  ui_->fading_out->setChecked(s.value("FadeoutEnabled", true).toBool());
  ui_->fading_cross->setChecked(s.value("CrossfadeEnabled", true).toBool());
  ui_->fading_auto->setChecked(s.value("AutoCrossfadeEnabled", false).toBool());
  ui_->fading_duration->setValue(s.value("FadeoutDuration", 2000).toInt());
  ui_->fading_samealbum->setChecked(
      s.value("NoCrossfadeSameAlbum", true).toBool());
  ui_->fadeout_pause->setChecked(
      s.value("FadeoutPauseEnabled", false).toBool());
  ui_->fading_pause_duration->setValue(
      s.value("FadeoutPauseDuration", 250).toInt());
  ui_->inhibit_suspend_while_playing->setChecked(
      s.value("InhibitSuspendWhilePlaying", false).toBool());
  s.endGroup();

  s.beginGroup(GstEngine::kSettingsGroup);
  QString sink = s.value("sink", GstEngine::kAutoSink).toString();
  QString device = s.value("device").toString();

  ui_->gst_output->setCurrentIndex(0);
  for (int i = 0; i < ui_->gst_output->count(); ++i) {
    GstEngine::OutputDetails details =
        ui_->gst_output->itemData(i).value<GstEngine::OutputDetails>();

    if (details.gstreamer_plugin_name == sink &&
        details.device_property_value == device) {
      ui_->gst_output->setCurrentIndex(i);
      break;
    }
  }

  ui_->replaygain->setChecked(s.value("rgenabled", false).toBool());
  ui_->replaygain_mode->setCurrentIndex(s.value("rgmode", 0).toInt());
  ui_->replaygain_preamp->setValue(s.value("rgpreamp", 0.0).toDouble() * 10 +
                                   150);
  ui_->replaygain_compression->setChecked(
      s.value("rgcompression", true).toBool());
  ui_->buffer_duration->setValue(s.value("bufferduration", 4000).toInt());
  ui_->mono_playback->setChecked(s.value("monoplayback", false).toBool());
  ui_->sample_rate->setCurrentIndex(ui_->sample_rate->findData(
      s.value("samplerate", GstEngine::kAutoSampleRate).toInt()));
  ui_->buffer_min_fill->setValue(s.value("bufferminfill", 33).toInt());
  s.endGroup();
}

void PlaybackSettingsPage::Save() {
  QSettings s;

  s.beginGroup(Playlist::kSettingsGroup);
  s.setValue("glow_effect", ui_->current_glow->isChecked());
  s.endGroup();

  s.beginGroup(Engine::Base::kSettingsGroup);
  s.setValue("FadeoutEnabled", ui_->fading_out->isChecked());
  s.setValue("FadeoutDuration", ui_->fading_duration->value());
  s.setValue("CrossfadeEnabled", ui_->fading_cross->isChecked());
  s.setValue("AutoCrossfadeEnabled", ui_->fading_auto->isChecked());
  s.setValue("NoCrossfadeSameAlbum", ui_->fading_samealbum->isChecked());
  s.setValue("FadeoutPauseEnabled", ui_->fadeout_pause->isChecked());
  s.setValue("FadeoutPauseDuration", ui_->fading_pause_duration->value());
  s.setValue("InhibitSuspendWhilePlaying",
             ui_->inhibit_suspend_while_playing->isChecked());
  s.endGroup();

  GstEngine::OutputDetails details =
      ui_->gst_output->itemData(ui_->gst_output->currentIndex())
          .value<GstEngine::OutputDetails>();

  s.beginGroup(GstEngine::kSettingsGroup);
  s.setValue("sink", details.gstreamer_plugin_name);
  s.setValue("device", details.device_property_value);
  s.setValue("rgenabled", ui_->replaygain->isChecked());
  s.setValue("rgmode", ui_->replaygain_mode->currentIndex());
  s.setValue("rgpreamp", float(ui_->replaygain_preamp->value()) / 10 - 15);
  s.setValue("rgcompression", ui_->replaygain_compression->isChecked());
  s.setValue("bufferduration", ui_->buffer_duration->value());
  s.setValue("monoplayback", ui_->mono_playback->isChecked());
  s.setValue(
      "samplerate",
      ui_->sample_rate->itemData(ui_->sample_rate->currentIndex()).toInt());
  s.setValue("bufferminfill", ui_->buffer_min_fill->value());
  s.endGroup();

  //Emit inhibit suspend while playing signal 
  emit InhibitSuspendWhilePlaying(
      ui_->inhibit_suspend_while_playing->isChecked());
}

void PlaybackSettingsPage::RgPreampChanged(int value) {
  float db = float(value) / 10 - 15;
  QString db_str;
  db_str.sprintf("%+.1f dB", db);
  ui_->replaygain_preamp_label->setText(db_str);
}

void PlaybackSettingsPage::BufferMinFillChanged(int value) {
  ui_->buffer_min_fill_value_label->setText(QString::number(value) + "%");
}

void PlaybackSettingsPage::FadingOptionsChanged() {
  ui_->fading_options->setEnabled(ui_->fading_out->isChecked() ||
                                  ui_->fading_cross->isChecked() ||
                                  ui_->fading_auto->isChecked());
}
