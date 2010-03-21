#include "analyzercontainer.h"
#include "baranalyzer.h"
#include "blockanalyzer.h"

#include <QMouseEvent>
#include <QHBoxLayout>
#include <QSettings>

const char* AnalyzerContainer::kSettingsGroup = "Analyzer";

AnalyzerContainer::AnalyzerContainer(QWidget *parent)
  : QWidget(parent),
    context_menu_(new QMenu(this)),
    group_(new QActionGroup(this)),
    mapper_(new QSignalMapper(this)),
    current_analyzer_(NULL),
    engine_(NULL)
{
  QHBoxLayout* layout = new QHBoxLayout(this);
  setLayout(layout);
  layout->setContentsMargins(0, 0, 0, 0);

  AddAnalyzerType<BarAnalyzer>();
  AddAnalyzerType<BlockAnalyzer>();
  connect(mapper_, SIGNAL(mapped(int)), SLOT(ChangeAnalyzer(int)));

  context_menu_->addSeparator();

  disable_action_ =
      context_menu_->addAction(tr("No analyzer"), this, SLOT(DisableAnalyzer()));
  disable_action_->setCheckable(true);
  group_->addAction(disable_action_);

  Load();
}

void AnalyzerContainer::mouseReleaseEvent(QMouseEvent* e) {
  if (e->button() == Qt::LeftButton || e->button() == Qt::RightButton)
    context_menu_->popup(e->globalPos());
}

void AnalyzerContainer::set_engine(EngineBase *engine) {
  if (current_analyzer_)
    current_analyzer_->set_engine(engine);
  engine_ = engine;
}

void AnalyzerContainer::DisableAnalyzer() {
  delete current_analyzer_;
  current_analyzer_ = NULL;

  Save();
}

void AnalyzerContainer::ChangeAnalyzer(int id) {
  delete current_analyzer_;
  current_analyzer_ = qobject_cast<Analyzer::Base*>(
      analyzer_types_[id]->newInstance(Q_ARG(QWidget*, this)));
  current_analyzer_->set_engine(engine_);

  layout()->addWidget(current_analyzer_);

  Save();
}

void AnalyzerContainer::Load() {
  QSettings s;
  s.beginGroup(kSettingsGroup);

  QString type = s.value("type", "BlockAnalyzer").toString();
  if (type.isEmpty()) {
    DisableAnalyzer();
    disable_action_->setChecked(true);
  } else {
    for (int i=0 ; i<analyzer_types_.count() ; ++i) {
      if (type == analyzer_types_[i]->className()) {
        ChangeAnalyzer(i);
        actions_[i]->setChecked(true);
        break;
      }
    }
  }
}

void AnalyzerContainer::Save() {
  QSettings s;
  s.beginGroup(kSettingsGroup);

  s.setValue("type", current_analyzer_ ?
             current_analyzer_->metaObject()->className() :
             QVariant());
}
