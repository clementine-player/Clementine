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

#ifndef VISUALISATIONCONTAINER_H
#define VISUALISATIONCONTAINER_H

#include <QWidget>

class ProjectMVisualisation;
class GstEngine;

class VisualisationContainer : public QWidget {
  Q_OBJECT

public:
  VisualisationContainer(QWidget* parent = 0);

  static const char* kSettingsGroup;
  static const int kDefaultWidth;
  static const int kDefaultHeight;

  void SetEngine(GstEngine* engine);

protected:
  // QWidget
  void showEvent(QShowEvent*);
  void hideEvent(QHideEvent*);
  void resizeEvent(QResizeEvent *);

private:
  GstEngine* engine_;
  ProjectMVisualisation* vis_;
};

#endif // VISUALISATIONCONTAINER_H
