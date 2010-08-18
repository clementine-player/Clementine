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

#ifndef ANALYZERCONTAINER_H
#define ANALYZERCONTAINER_H

#include <QWidget>
#include <QMenu>
#include <QSignalMapper>

#include "analyzerbase.h"
#include "engines/engine_fwd.h"

#include "analyzer.h"

class AnalyzerContainer : public QWidget {
  Q_OBJECT

public:
  AnalyzerContainer(QWidget* parent);

  void SetEngine(EngineBase* engine);
  void SetActions(QAction* visualisation);

  static const char* kSettingsGroup;

protected:
  void mouseReleaseEvent(QMouseEvent *);
  void mouseDoubleClickEvent(QMouseEvent *);

private slots:
  void ChangeAnalyzer(int id);
  void DisableAnalyzer();
  void ShowPopupMenu();

private:
  void Load();
  void Save();
  template <typename T>
      void AddAnalyzerType();

private:
  QMenu* context_menu_;
  QActionGroup* group_;
  QSignalMapper* mapper_;

  QList<const QMetaObject*> analyzer_types_;
  QList<QAction*> actions_;
  QAction* disable_action_;

  QAction* visualisation_action_;
  QTimer* double_click_timer_;
  QPoint last_click_pos_;
  bool ignore_next_click_;

  QWidget* current_analyzer_;
  EngineBase* engine_;
};

template <typename T>
    void AnalyzerContainer::AddAnalyzerType() {
  int id = analyzer_types_.count();
  analyzer_types_ << &T::staticMetaObject;

  QAction* action = context_menu_->addAction(tr(T::kName), mapper_, SLOT(map()));
  group_->addAction(action);
  mapper_->setMapping(action, id);
  action->setCheckable(true);
  actions_ << action;
}

#endif

