#ifndef ANALYZERCONTAINER_H
#define ANALYZERCONTAINER_H

#include <QWidget>
#include <QMenu>
#include <QSignalMapper>

#include "analyzerbase.h"
#include "engine_fwd.h"

class AnalyzerContainer : public QWidget {
  Q_OBJECT

public:
  AnalyzerContainer(QWidget* parent);

  void set_engine(EngineBase* engine);

  static const char* kSettingsGroup;

protected:
  void contextMenuEvent(QContextMenuEvent *);

private slots:
  void ChangeAnalyzer(int id);
  void DisableAnalyzer();

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

  Analyzer::Base* current_analyzer_;
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

