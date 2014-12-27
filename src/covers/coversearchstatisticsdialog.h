/* This file is part of Clementine.
   Copyright 2011, David Sansome <davidsansome@gmail.com>
   Copyright 2014, Krzysztof Sobiecki <sobkas@gmail.com>
   Copyright 2014, John Maguire <john.maguire@gmail.com>

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

#ifndef COVERS_COVERSEARCHSTATISTICSDIALOG_H_
#define COVERS_COVERSEARCHSTATISTICSDIALOG_H_

#include <QDialog>

#include "coversearchstatistics.h"

class Ui_CoverSearchStatisticsDialog;

class QVBoxLayout;

class CoverSearchStatisticsDialog : public QDialog {
  Q_OBJECT

 public:
  explicit CoverSearchStatisticsDialog(QWidget* parent = nullptr);
  ~CoverSearchStatisticsDialog();

  void Show(const CoverSearchStatistics& statistics);

 private:
  void AddLine(const QString& label, const QString& value);
  void AddSpacer();

 private:
  Ui_CoverSearchStatisticsDialog* ui_;
  QVBoxLayout* details_layout_;
};

#endif  // COVERS_COVERSEARCHSTATISTICSDIALOG_H_
