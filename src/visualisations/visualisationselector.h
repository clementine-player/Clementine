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

#ifndef VISUALISATIONSELECTOR_H
#define VISUALISATIONSELECTOR_H

#include <QDialog>

class ProjectMPresetModel;
class ProjectMVisualisation;
class Ui_VisualisationSelector;

class VisualisationSelector : public QDialog {
  Q_OBJECT
public:
  VisualisationSelector(QWidget* parent = 0);
  ~VisualisationSelector();

  void SetVisualisation(ProjectMVisualisation* vis) { vis_ = vis; }

protected:
  void showEvent(QShowEvent *);

private slots:
  void ModeChanged(int mode);
  void SelectAll();
  void SelectNone();

private:
  Ui_VisualisationSelector* ui_;
  ProjectMVisualisation* vis_;

  ProjectMPresetModel* preset_model_;
};

#endif // VISUALISATIONSELECTOR_H
