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

// We probably don't need to translate these, right?
const char* Equalizer::kGainText[] = {
  "30", "60", "125", "250", "500", "1k", "2k", "4k", "8k", "16k"};

const char* Equalizer::kSettingsGroup = "Equalizer";

Equalizer::Equalizer(QWidget *parent)
  : QDialog(parent)
{
  ui_.setupUi(this);
  connect(ui_.enable, SIGNAL(toggled(bool)), SIGNAL(EnabledChanged(bool)));
  connect(ui_.enable, SIGNAL(toggled(bool)), ui_.slider_container, SLOT(setEnabled(bool)));

  preamp_ = AddSlider(tr("Pre-amp"));

  QFrame* line = new QFrame(ui_.slider_container);
  line->setFrameShape(QFrame::VLine);
  line->setFrameShadow(QFrame::Sunken);
  ui_.slider_container->layout()->addWidget(line);

  for (int i=0 ; i<kBands ; ++i)
    gain_[i] = AddSlider(kGainText[i]);
}

void Equalizer::ReloadSettings() {
  QSettings s;
  s.beginGroup(kSettingsGroup);

  ui_.enable->setChecked(s.value("enabled", false).toBool());

  preamp_->set_value(s.value("preamp", 0).toInt());
  for (int i=0 ; i<kBands ; ++i) {
    gain_[i]->set_value(s.value("gain_" + QString::number(i+1), 0).toInt());
  }

  emit EnabledChanged(ui_.enable->isChecked());
  ParametersChanged();
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
}


