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

#include "equalizer.h"
#include "ui_equalizer.h"

#include <QInputDialog>
#include <QMessageBox>
#include <QSettings>
#include <QShortcut>
#include <QtDebug>

#include "ui/iconloader.h"
#include "widgets/equalizerslider.h"

// We probably don't need to translate these, right?
const char* Equalizer::kGainText[] = {"60", "170", "310", "600", "1k",
                                      "3k", "6k",  "12k", "14k", "16k"};

const char* Equalizer::kSettingsGroup = "Equalizer";

Equalizer::Equalizer(QWidget* parent)
    : QDialog(parent), ui_(new Ui_Equalizer), loading_(false) {
  ui_->setupUi(this);

  // Icons
  ui_->preset_del->setIcon(IconLoader::Load("list-remove", IconLoader::Base));
  ui_->preset_save->setIcon(IconLoader::Load("document-save", IconLoader::Base));

  preamp_ = AddSlider(tr("Pre-amp"));

  QFrame* line = new QFrame(ui_->slider_container);
  line->setFrameShape(QFrame::VLine);
  line->setFrameShadow(QFrame::Sunken);
  ui_->slider_container->layout()->addWidget(line);

  for (int i = 0; i < kBands; ++i) gain_[i] = AddSlider(kGainText[i]);

  // Must be done before the signals are connected
  ReloadSettings();

  connect(ui_->enable, SIGNAL(toggled(bool)), SIGNAL(EnabledChanged(bool)));
  connect(ui_->enable, SIGNAL(toggled(bool)), ui_->slider_container,
          SLOT(setEnabled(bool)));
  connect(ui_->enable, SIGNAL(toggled(bool)), SLOT(Save()));
  connect(ui_->preset, SIGNAL(currentIndexChanged(int)),
          SLOT(PresetChanged(int)));
  connect(ui_->preset_save, SIGNAL(clicked()), SLOT(SavePreset()));
  connect(ui_->preset_del, SIGNAL(clicked()), SLOT(DelPreset()));
  connect(ui_->balance_slider, SIGNAL(valueChanged(int)),
          SLOT(StereoSliderChanged(int)));

  QShortcut* close = new QShortcut(QKeySequence::Close, this);
  connect(close, SIGNAL(activated()), SLOT(close()));
}

Equalizer::~Equalizer() { delete ui_; }

void Equalizer::ReloadSettings() {
  QSettings s;
  s.beginGroup(kSettingsGroup);

  presets_.clear();
  ui_->preset->clear();

  // Load presets
  int count = s.beginReadArray("presets");
  for (int i = 0; i < count; ++i) {
    s.setArrayIndex(i);
    AddPreset(s.value("name").toString(),
              s.value("params").value<Equalizer::Params>());
  }
  s.endArray();

  if (count == 0) LoadDefaultPresets();

  // Selected preset
  QString selected_preset = s.value("selected_preset", "Custom").toString();
  QString selected_preset_display_name =
      QString(tr(qPrintable(selected_preset)));
  int selected_index = ui_->preset->findText(selected_preset_display_name);
  if (selected_index != -1) ui_->preset->setCurrentIndex(selected_index);

  // Enabled?
  ui_->enable->setChecked(s.value("enabled", false).toBool());
  ui_->slider_container->setEnabled(ui_->enable->isChecked());

  int stereo_balance = s.value("stereo_balance", 0).toInt();
  ui_->balance_slider->setValue(stereo_balance);
  StereoSliderChanged(stereo_balance);

  PresetChanged(selected_preset);
}

void Equalizer::LoadDefaultPresets() {
  AddPreset(QT_TRANSLATE_NOOP("Equalizer", "Custom"),
            Params(0, 0, 0, 0, 0, 0, 0, 0, 0, 0));
  AddPreset(QT_TRANSLATE_NOOP("Equalizer", "Classical"),
            Params(0, 0, 0, 0, 0, 0, -40, -40, -40, -50));
  AddPreset(QT_TRANSLATE_NOOP("Equalizer", "Club"),
            Params(0, 0, 20, 30, 30, 30, 20, 0, 0, 0));
  AddPreset(QT_TRANSLATE_NOOP("Equalizer", "Dance"),
            Params(50, 35, 10, 0, 0, -30, -40, -40, 0, 0));
  // Dubstep equalizer created by Devyn Collier Johnson
  AddPreset(QT_TRANSLATE_NOOP("Equalizer", "Dubstep"),
            Params(0, 36, 85, 58, 30, 0, 36, 60, 96, 62, 0));
  AddPreset(QT_TRANSLATE_NOOP("Equalizer", "Full Bass"),
            Params(70, 70, 70, 40, 20, -45, -50, -55, -55, -55));
  AddPreset(QT_TRANSLATE_NOOP("Equalizer", "Full Treble"),
            Params(-50, -50, -50, -25, 15, 55, 80, 80, 80, 85));
  AddPreset(QT_TRANSLATE_NOOP("Equalizer", "Full Bass + Treble"),
            Params(35, 30, 0, -40, -25, 10, 45, 55, 60, 60));
  // HipHop equalizer created by Devyn Collier Johnson
  AddPreset(QT_TRANSLATE_NOOP("Equalizer", "HipHop"),
            Params(88, 100, 100, -23, -27, 12, 100, 0, 50, 73, 0));
  // Kuduro equalizer created by Devyn Collier Johnson
  AddPreset(QT_TRANSLATE_NOOP("Equalizer", "Kuduro"),
            Params(65, -100, 15, 62, -38, 88, 100, -46, 23, -100, 0));
  AddPreset(QT_TRANSLATE_NOOP("Equalizer", "Laptop/Headphones"),
            Params(25, 50, 25, -20, 0, -30, -40, -40, 0, 0));
  AddPreset(QT_TRANSLATE_NOOP("Equalizer", "Large Hall"),
            Params(50, 50, 30, 30, 0, -25, -25, -25, 0, 0));
  AddPreset(QT_TRANSLATE_NOOP("Equalizer", "Live"),
            Params(-25, 0, 20, 25, 30, 30, 20, 15, 15, 10));
  AddPreset(QT_TRANSLATE_NOOP("Equalizer", "Party"),
            Params(35, 35, 0, 0, 0, 0, 0, 0, 35, 35));
  AddPreset(QT_TRANSLATE_NOOP("Equalizer", "Pop"),
            Params(-10, 25, 35, 40, 25, -5, -15, -15, -10, -10));
  // Psychedelic equalizer created by Devyn Collier Johnson
  AddPreset(QT_TRANSLATE_NOOP("Equalizer", "Psychedelic"),
            Params(100, 100, 0, 40, 0, 67, 79, 0, 30, -100, 37));
  AddPreset(QT_TRANSLATE_NOOP("Equalizer", "Reggae"),
            Params(0, 0, -5, -30, 0, -35, -35, 0, 0, 0));
  AddPreset(QT_TRANSLATE_NOOP("Equalizer", "Rock"),
            Params(40, 25, -30, -40, -20, 20, 45, 55, 55, 55));
  AddPreset(QT_TRANSLATE_NOOP("Equalizer", "Soft"),
            Params(25, 10, -5, -15, -5, 20, 45, 50, 55, 60));
  AddPreset(QT_TRANSLATE_NOOP("Equalizer", "Ska"),
            Params(-15, -25, -25, -5, 20, 30, 45, 50, 55, 50));
  AddPreset(QT_TRANSLATE_NOOP("Equalizer", "Soft Rock"),
            Params(20, 20, 10, -5, -25, -30, -20, -5, 15, 45));
  AddPreset(QT_TRANSLATE_NOOP("Equalizer", "Techno"),
            Params(40, 30, 0, -30, -25, 0, 40, 50, 50, 45));
  AddPreset(QT_TRANSLATE_NOOP("Equalizer", "Zero"),
            Params(0, 0, 0, 0, 0, 0, 0, 0, 0, 0));
}

void Equalizer::AddPreset(const QString& name, const Params& params) {
  QString name_displayed = tr(qPrintable(name));
  presets_[name] = params;
  if (ui_->preset->findText(name_displayed) == -1) {
    ui_->preset->addItem(name_displayed,  // name to display (translated)
                         QVariant(name)   // original name
                         );
  }
}

void Equalizer::PresetChanged(int index) {
  PresetChanged(ui_->preset->itemData(index).toString());
}

void Equalizer::PresetChanged(const QString& name) {
  if (presets_.contains(last_preset_)) {
    if (presets_[last_preset_] != current_params()) {
      SaveCurrentPreset();
    }
  }
  last_preset_ = name;

  Params& p = presets_[name];

  loading_ = true;
  preamp_->set_value(p.preamp);
  for (int i = 0; i < kBands; ++i) gain_[i]->set_value(p.gain[i]);
  loading_ = false;

  ParametersChanged();
  Save();
}

void Equalizer::SavePreset() {
  QString name = SaveCurrentPreset();
  if (!name.isEmpty()) {
    last_preset_ = name;
    ui_->preset->setCurrentIndex(ui_->preset->findText(tr(qPrintable(name))));
  }
}

QString Equalizer::SaveCurrentPreset() {
  QString name =
      QInputDialog::getText(this, tr("Save preset"), tr("Name"),
                            QLineEdit::Normal, tr(qPrintable(last_preset_)));
  ;
  if (name.isEmpty()) return QString();

  AddPreset(name, current_params());
  Save();
  return name;
}

void Equalizer::DelPreset() {
  QString name = ui_->preset->itemData(ui_->preset->currentIndex()).toString();
  QString name_displayed = ui_->preset->currentText();
  if (!presets_.contains(name) || name.isEmpty()) return;

  int ret = QMessageBox::question(
      this, tr("Delete preset"),
      tr("Are you sure you want to delete the \"%1\" preset?")
          .arg(name_displayed),
      QMessageBox::Yes, QMessageBox::No);

  if (ret == QMessageBox::No) return;

  presets_.remove(name);
  ui_->preset->removeItem(ui_->preset->currentIndex());
  Save();
}

EqualizerSlider* Equalizer::AddSlider(const QString& label) {
  EqualizerSlider* ret = new EqualizerSlider(label, ui_->slider_container);
  ui_->slider_container->layout()->addWidget(ret);
  connect(ret, SIGNAL(ValueChanged(int)), SLOT(ParametersChanged()));

  return ret;
}

bool Equalizer::is_enabled() const { return ui_->enable->isChecked(); }

int Equalizer::preamp_value() const { return preamp_->value(); }

QList<int> Equalizer::gain_values() const {
  QList<int> ret;
  for (int i = 0; i < kBands; ++i) {
    ret << gain_[i]->value();
  }
  return ret;
}

Equalizer::Params Equalizer::current_params() const {
  QList<int> gains = gain_values();

  Params ret;
  ret.preamp = preamp_value();
  std::copy(gains.begin(), gains.end(), ret.gain);
  return ret;
}

float Equalizer::stereo_balance() const {
  return qBound(-1.0f, ui_->balance_slider->value() / 100.0f, 1.0f);
}

void Equalizer::ParametersChanged() {
  if (loading_) return;

  emit ParametersChanged(preamp_value(), gain_values());
}

void Equalizer::Save() {
  QSettings s;
  s.beginGroup(kSettingsGroup);

  // Presets
  s.beginWriteArray("presets", presets_.count());
  int i = 0;
  for (const QString& name : presets_.keys()) {
    s.setArrayIndex(i++);
    s.setValue("name", name);
    s.setValue("params", QVariant::fromValue(presets_[name]));
  }
  s.endArray();

  // Selected preset
  s.setValue("selected_preset",
             ui_->preset->itemData(ui_->preset->currentIndex()).toString());

  // Enabled?
  s.setValue("enabled", ui_->enable->isChecked());

  s.setValue("stereo_balance", ui_->balance_slider->value());
}

void Equalizer::closeEvent(QCloseEvent* e) {
  QString name = ui_->preset->currentText();
  if (!presets_.contains(name)) return;

  if (presets_[name] == current_params()) return;

  SavePreset();
}

Equalizer::Params::Params() : preamp(0) {
  for (int i = 0; i < Equalizer::kBands; ++i) gain[i] = 0;
}

Equalizer::Params::Params(int g0, int g1, int g2, int g3, int g4, int g5,
                          int g6, int g7, int g8, int g9, int pre)
    : preamp(pre) {
  gain[0] = g0;
  gain[1] = g1;
  gain[2] = g2;
  gain[3] = g3;
  gain[4] = g4;
  gain[5] = g5;
  gain[6] = g6;
  gain[7] = g7;
  gain[8] = g8;
  gain[9] = g9;
}

bool Equalizer::Params::operator==(const Equalizer::Params& other) const {
  if (preamp != other.preamp) return false;
  for (int i = 0; i < Equalizer::kBands; ++i) {
    if (gain[i] != other.gain[i]) return false;
  }
  return true;
}

bool Equalizer::Params::operator!=(const Equalizer::Params& other) const {
  return !(*this == other);
}

void Equalizer::StereoSliderChanged(int value) {
  emit StereoBalanceChanged(stereo_balance());
  Save();
}

QDataStream& operator<<(QDataStream& s, const Equalizer::Params& p) {
  s << p.preamp;
  for (int i = 0; i < Equalizer::kBands; ++i) s << p.gain[i];
  return s;
}

QDataStream& operator>>(QDataStream& s, Equalizer::Params& p) {
  s >> p.preamp;
  for (int i = 0; i < Equalizer::kBands; ++i) s >> p.gain[i];
  return s;
}
