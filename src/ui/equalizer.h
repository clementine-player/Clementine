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

#ifndef EQUALIZER_H
#define EQUALIZER_H

#include <QDialog>
#include <QMetaType>
#include <QMap>

class EqualizerSlider;
class Ui_Equalizer;

class Equalizer : public QDialog {
  Q_OBJECT

 public:
  Equalizer(QWidget* parent = 0);
  ~Equalizer();

  static const int kBands = 10;
  static const char* kGainText[kBands];
  static const char* kSettingsGroup;

  struct Params {
    Params();
    Params(int g0, int g1, int g2, int g3, int g4, int g5, int g6, int g7,
           int g8, int g9, int pre = 0);

    bool operator==(const Params& other) const;
    bool operator!=(const Params& other) const;

    int preamp;
    int gain[kBands];
  };

  bool is_enabled() const;
  int preamp_value() const;
  QList<int> gain_values() const;
  Params current_params() const;
  float stereo_balance() const;

signals:
  void EnabledChanged(bool enabled);
  void ParametersChanged(int preamp, const QList<int>& band_gains);
  void StereoBalanceChanged(float balance);

 protected:
  void closeEvent(QCloseEvent*);

 private slots:
  void ParametersChanged();
  void PresetChanged(const QString& name);
  void PresetChanged(int index);
  void SavePreset();
  void DelPreset();
  void Save();
  void StereoSliderChanged(int value);

 private:
  EqualizerSlider* AddSlider(const QString& label);
  void LoadDefaultPresets();
  void AddPreset(const QString& name, const Params& params);
  void ReloadSettings();
  QString SaveCurrentPreset();

 private:
  Ui_Equalizer* ui_;
  bool loading_;

  QString last_preset_;

  EqualizerSlider* preamp_;
  EqualizerSlider* gain_[kBands];

  QMap<QString, Params> presets_;
};
Q_DECLARE_METATYPE(Equalizer::Params);

QDataStream& operator<<(QDataStream& s, const Equalizer::Params& p);
QDataStream& operator>>(QDataStream& s, Equalizer::Params& p);

#endif  // EQUALIZER_H
