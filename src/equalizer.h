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

#ifndef EQUALIZER_H
#define EQUALIZER_H

#include <QDialog>

#include "ui_equalizer.h"

class EqualizerSlider;

class Equalizer : public QDialog {
  Q_OBJECT

 public:
  Equalizer(QWidget *parent = 0);

  static const int kBands = 10;
  static const char* kGainText[kBands];
  static const char* kSettingsGroup;

  int preamp_value() const;
  QList<int> gain_values() const;

 public slots:
  void ReloadSettings();

 signals:
  void EnabledChanged(bool enabled);
  void ParametersChanged(int preamp, const QList<int>& band_gains);

 private slots:
  void ParametersChanged();

 private:
  EqualizerSlider* AddSlider(const QString& label);

 private:
  Ui::Equalizer ui_;

  EqualizerSlider* preamp_;
  EqualizerSlider* gain_[kBands];
};

#endif // EQUALIZER_H
