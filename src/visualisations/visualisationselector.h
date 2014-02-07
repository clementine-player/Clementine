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

#ifndef VISUALISATIONSELECTOR_H
#define VISUALISATIONSELECTOR_H

#include <QDialog>

class ProjectMVisualisation;
class Ui_VisualisationSelector;

class VisualisationSelector : public QDialog {
  Q_OBJECT
 public:
  VisualisationSelector(QWidget* parent = 0);
  ~VisualisationSelector();

  void SetVisualisation(ProjectMVisualisation* vis) { vis_ = vis; }

 protected:
  void showEvent(QShowEvent*);
  void hideEvent(QHideEvent*);

 private slots:
  void ModeChanged(int mode);
  void SelectAll();
  void SelectNone();

 private:
  Ui_VisualisationSelector* ui_;
  QPushButton* select_all_;
  QPushButton* select_none_;

  ProjectMVisualisation* vis_;
};

#endif  // VISUALISATIONSELECTOR_H
