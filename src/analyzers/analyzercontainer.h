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

#ifndef ANALYZERCONTAINER_H
#define ANALYZERCONTAINER_H

#include <QWidget>
#include <QMenu>
#include <QSignalMapper>

#include "analyzerbase.h"
#include "engines/engine_fwd.h"

class AnalyzerContainer : public QWidget {
  Q_OBJECT

 public:
  AnalyzerContainer(QWidget* parent);

  void SetEngine(EngineBase* engine);
  void SetActions(QAction* visualisation);

  static const char* kSettingsGroup;
  static const char* kSettingsFramerate;

signals:
  void WheelEvent(int delta);

 protected:
  void mouseReleaseEvent(QMouseEvent*);
  void mouseDoubleClickEvent(QMouseEvent*);
  void wheelEvent(QWheelEvent* e);

 private slots:
  void ChangeAnalyzer(int id);
  void ChangeFramerate(int new_framerate);
  void DisableAnalyzer();
  void ShowPopupMenu();

 private:
  static const int kLowFramerate;
  static const int kMediumFramerate;
  static const int kHighFramerate;
  static const int kSuperHighFramerate;

  void Load();
  void Save();
  void SaveFramerate(int framerate);
  template <typename T>
  void AddAnalyzerType();
  void AddFramerate(const QString& name, int framerate);

 private:
  int current_framerate_;  // fps
  QMenu* context_menu_;
  QMenu* context_menu_framerate_;
  QActionGroup* group_;
  QActionGroup* group_framerate_;
  QSignalMapper* mapper_;
  QSignalMapper* mapper_framerate_;

  QList<const QMetaObject*> analyzer_types_;
  QList<int> framerate_list_;
  QList<QAction*> actions_;
  QAction* disable_action_;

  QAction* visualisation_action_;
  QTimer* double_click_timer_;
  QPoint last_click_pos_;
  bool ignore_next_click_;

  Analyzer::Base* current_analyzer_;
  EngineBase* engine_;
};

template <typename T>
void AnalyzerContainer::AddAnalyzerType() {
  int id = analyzer_types_.count();
  analyzer_types_ << &T::staticMetaObject;

  QAction* action =
      context_menu_->addAction(tr(T::kName), mapper_, SLOT(map()));
  group_->addAction(action);
  mapper_->setMapping(action, id);
  action->setCheckable(true);
  actions_ << action;
}

#endif
