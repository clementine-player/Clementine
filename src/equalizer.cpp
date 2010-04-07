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

#include "equalizer.h"
#include "equalizerslider.h"

#include <QSettings>
#include <QtDebug>
#include <QInputDialog>
#include <QMessageBox>

// We probably don't need to translate these, right?
const char* Equalizer::kGainText[] = {
  "30", "60", "125", "250", "500", "1k", "2k", "4k", "8k", "16k"};

const char* Equalizer::kSettingsGroup = "Equalizer";

Equalizer::Equalizer(QWidget *parent)
  : QDialog(parent),
    loading_(false)
{
  ui_.setupUi(this);
  connect(ui_.enable, SIGNAL(toggled(bool)), SIGNAL(EnabledChanged(bool)));
  connect(ui_.enable, SIGNAL(toggled(bool)), ui_.slider_container, SLOT(setEnabled(bool)));
  connect(ui_.preset, SIGNAL(currentIndexChanged(QString)), SLOT(PresetChanged(QString)));
  connect(ui_.preset_add, SIGNAL(clicked()), SLOT(AddPreset()));
  connect(ui_.preset_del, SIGNAL(clicked()), SLOT(DelPreset()));

  preamp_ = AddSlider(tr("Pre-amp"));

  QFrame* line = new QFrame(ui_.slider_container);
  line->setFrameShape(QFrame::VLine);
  line->setFrameShadow(QFrame::Sunken);
  ui_.slider_container->layout()->addWidget(line);

  for (int i=0 ; i<kBands ; ++i)
    gain_[i] = AddSlider(kGainText[i]);
}

void Equalizer::ReloadSettings() {
  loading_ = true;
  QSettings s;
  s.beginGroup(kSettingsGroup);

  presets_.clear();
  ui_.preset->clear();

  // Load presets
  int count = s.beginReadArray("presets");
  for (int i=0 ; i<count ; ++i) {
    s.setArrayIndex(i);
    AddPreset(s.value("name").toString(),
              s.value("params").value<Equalizer::Params>());
  }
  s.endArray();

  if (count == 0)
    LoadDefaultPresets();

  // Selected preset
  QString selected_preset = s.value("selected_preset", "Custom").toString();
  int selected_index = ui_.preset->findText(selected_preset);
  if (selected_index != -1) {
    ui_.preset->setCurrentIndex(selected_index);
    PresetChanged(selected_preset);
  }

  // Enabled?
  ui_.enable->setChecked(s.value("enabled", false).toBool());

  emit EnabledChanged(ui_.enable->isChecked());
  ParametersChanged();

  loading_ = false;
}

void Equalizer::LoadDefaultPresets() {
  AddPreset("Custom",                 Params(0, 0, 0, 0, 0, 0, 0, 0, 0, 0));
  AddPreset(tr("Classical"),          Params(0, 0, 0, 0, 0, 0, -40, -40, -40, -50));
  AddPreset(tr("Club"),               Params(0, 0, 20, 30, 30, 30, 20, 0, 0, 0));
  AddPreset(tr("Dance"),              Params(50, 35, 10, 0, 0, -30, -40, -40, 0, 0));
  AddPreset(tr("Full Bass"),          Params(70, 70, 70, 40, 20, -45, -50, -55, -55, -55));
  AddPreset(tr("Full Treble"),        Params(-50, -50, -50, -25, 15, 55, 80, 80, 80, 85));
  AddPreset(tr("Full Bass + Treble"), Params(35, 30, 0, -40, -25, 10, 45, 55, 60, 60));
  AddPreset(tr("Laptop/Headphones"),  Params(25, 50, 25, -20, 0, -30, -40, -40, 0, 0));
  AddPreset(tr("Large Hall"),         Params(50, 50, 30, 30, 0, -25, -25, -25, 0, 0));
  AddPreset(tr("Live"),               Params(-25, 0, 20, 25, 30, 30, 20, 15, 15, 10));
  AddPreset(tr("Party"),              Params(35, 35, 0, 0, 0, 0, 0, 0, 35, 35));
  AddPreset(tr("Pop"),                Params(-10, 25, 35, 40, 25, -5, -15, -15, -10, -10));
  AddPreset(tr("Reggae"),             Params(0, 0, -5, -30, 0, -35, -35, 0, 0, 0));
  AddPreset(tr("Rock"),               Params(40, 25, -30, -40, -20, 20, 45, 55, 55, 55));
  AddPreset(tr("Soft"),               Params(25, 10, -5, -15, -5, 20, 45, 50, 55, 60));
  AddPreset(tr("Ska"),                Params(-15, -25, -25, -5, 20, 30, 45, 50, 55, 50));
  AddPreset(tr("Soft Rock"),          Params(20, 20, 10, -5, -25, -30, -20, -5, 15, 45));
  AddPreset(tr("Techno"),             Params(40, 30, 0, -30, -25, 0, 40, 50, 50, 45));
  AddPreset(tr("Zero"),               Params(0, 0, 0, 0, 0, 0, 0, 0, 0, 0));
}

void Equalizer::AddPreset(const QString& name, const Params& params) {
  presets_[name] = params;
  ui_.preset->addItem(name);
}

void Equalizer::PresetChanged(const QString& name) {
  Params& p = presets_[name];

  loading_ = true;
  preamp_->set_value(p.preamp);
  for (int i=0 ; i<kBands ; ++i)
    gain_[i]->set_value(p.gain[i]);
  loading_ = false;

  ParametersChanged();
}

void Equalizer::AddPreset() {
  QString name;
  forever {
    name = QInputDialog::getText(this, tr("New preset"), tr("Name"),
                                 QLineEdit::Normal, name);
    if (name.isEmpty())
      return;

    if (presets_.contains(name)) {
      QMessageBox::information(this, tr("New preset"), tr("There is already a preset with that name"),
                               QMessageBox::Ok);
      continue;
    }

    break;
  }

  AddPreset(name, Params());
  ui_.preset->setCurrentIndex(ui_.preset->findText(name));
  Save();
}

void Equalizer::DelPreset() {
  QString name = ui_.preset->currentText();
  if (!presets_.contains(name) || name.isEmpty())
    return;

  int ret = QMessageBox::question(
      this, tr("Delete preset"),
      tr("Are you sure you want to delete the \"%1\" preset?").arg(name),
      QMessageBox::Yes, QMessageBox::No);

  if (ret == QMessageBox::No)
    return;

  presets_.remove(name);
  ui_.preset->removeItem(ui_.preset->currentIndex());
  Save();
}

EqualizerSlider* Equalizer::AddSlider(const QString &label) {
  EqualizerSlider* ret = new EqualizerSlider(label, ui_.slider_container);
  ui_.slider_container->layout()->addWidget(ret);
  connect(ret, SIGNAL(ValueChanged(int)), SLOT(ParametersChanged()));

  return ret;
}

int Equalizer::preamp_value() const {
  return preamp_->value();
}

QList<int> Equalizer::gain_values() const {
  QList<int> ret;
  for (int i=0 ; i<kBands ; ++i) {
    ret << gain_[i]->value();
  }
  return ret;
}

void Equalizer::ParametersChanged() {
  emit ParametersChanged(preamp_value(), gain_values());

  // Update the preset
  if (!loading_) {
    QString name = ui_.preset->currentText();
    if (!presets_.contains(name) || name.isEmpty())
      return;

    Params& p = presets_[name];
    p.preamp = preamp_->value();
    for (int i=0 ; i<kBands ; ++i)
      p.gain[i] = gain_[i]->value();

    Save();
  }
}

void Equalizer::Save() {
  if (loading_)
    return;

  QSettings s;
  s.beginGroup(kSettingsGroup);

  // Presets
  s.beginWriteArray("presets", presets_.count());
  int i=0;
  foreach (const QString& name, presets_.keys()) {
    s.setArrayIndex(i++);
    s.setValue("name", name);
    s.setValue("params", QVariant::fromValue(presets_[name]));
  }
  s.endArray();

  // Selected preset
  s.setValue("selected_preset", ui_.preset->currentText());

  // Enabled?
  s.setValue("enabled", ui_.enable->isChecked());
}


Equalizer::Params::Params()
  : preamp(0)
{
  for (int i=0 ; i<Equalizer::kBands ; ++i)
    gain[i] = 0;
}

Equalizer::Params::Params(int g0, int g1, int g2, int g3, int g4, int g5,
                          int g6, int g7, int g8, int g9, int pre)
                            : preamp(pre)
{
  gain[0] = g0;  gain[1] = g1; gain[2] = g2;  gain[3] = g3; gain[4] = g4;
  gain[5] = g5;  gain[6] = g6; gain[7] = g7;  gain[8] = g8; gain[9] = g9;
}

QDataStream &operator<<(QDataStream& s, const Equalizer::Params& p) {
  s << p.preamp;
  for (int i=0 ; i<Equalizer::kBands ; ++i)
    s << p.gain[i];
  return s;
}

QDataStream &operator>>(QDataStream& s, Equalizer::Params& p) {
  s >> p.preamp;
  for (int i=0 ; i<Equalizer::kBands ; ++i)
    s >> p.gain[i];
  return s;
}


