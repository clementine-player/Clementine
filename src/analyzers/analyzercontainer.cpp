/* This file is part of Clementine.
   Copyright 2010-2011, David Sansome <davidsansome@gmail.com>
   Copyright 2010, 2014, John Maguire <john.maguire@gmail.com>
   Copyright 2011-2012, Arnaud Bienner <arnaud.bienner@gmail.com>
   Copyright 2013, Vasily Fomin <vasili.fomin@gmail.com>
   Copyright 2014, Krzysztof Sobiecki <sobkas@gmail.com>
   Copyright 2014-2015, Mark Furneaux <mark@furneaux.ca>

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

#include "analyzercontainer.h"

#include <QHBoxLayout>
#include <QMouseEvent>
#include <QSettings>
#include <QTimer>
#include <QtDebug>

#include "baranalyzer.h"
#include "blockanalyzer.h"
#include "boomanalyzer.h"
#include "core/logging.h"
#include "rainbowanalyzer.h"
#include "sonogram.h"
#include "turbine.h"

const char* AnalyzerContainer::kSettingsGroup = "Analyzer";
const char* AnalyzerContainer::kSettingsFramerate = "framerate";

// Framerates
const int AnalyzerContainer::kLowFramerate = 20;
const int AnalyzerContainer::kMediumFramerate = 25;
const int AnalyzerContainer::kHighFramerate = 30;
const int AnalyzerContainer::kSuperHighFramerate = 60;

AnalyzerContainer::AnalyzerContainer(QWidget* parent)
    : QWidget(parent),
      current_framerate_(kMediumFramerate),
      context_menu_(new QMenu(this)),
      context_menu_framerate_(new QMenu(tr("Framerate"), this)),
      group_(new QActionGroup(this)),
      group_framerate_(new QActionGroup(this)),
      visualisation_action_(nullptr),
      double_click_timer_(new QTimer(this)),
      ignore_next_click_(false),
      psychedelic_colors_on_(false),
      current_analyzer_(nullptr),
      engine_(nullptr) {
  QHBoxLayout* layout = new QHBoxLayout(this);
  setLayout(layout);
  layout->setContentsMargins(0, 0, 0, 0);

  // Init framerate sub-menu
  AddFramerate(tr("Low (%1 fps)").arg(kLowFramerate), kLowFramerate);
  AddFramerate(tr("Medium (%1 fps)").arg(kMediumFramerate), kMediumFramerate);
  AddFramerate(tr("High (%1 fps)").arg(kHighFramerate), kHighFramerate);
  AddFramerate(tr("Super high (%1 fps)").arg(kSuperHighFramerate),
               kSuperHighFramerate);

  context_menu_->addMenu(context_menu_framerate_);
  context_menu_->addSeparator();

  AddAnalyzerType<BarAnalyzer>();
  AddAnalyzerType<BlockAnalyzer>();
  AddAnalyzerType<BoomAnalyzer>();
  AddAnalyzerType<Sonogram>();
  AddAnalyzerType<TurbineAnalyzer>();
  AddAnalyzerType<Rainbow::NyanCatAnalyzer>();
  AddAnalyzerType<Rainbow::RainbowDashAnalyzer>();

  disable_action_ = context_menu_->addAction(tr("No analyzer"), this,
                                             SLOT(DisableAnalyzer()));
  disable_action_->setCheckable(true);
  group_->addAction(disable_action_);

  context_menu_->addSeparator();
  psychedelic_enable_ = context_menu_->addAction(
      tr("Use Psychedelic Colors"), this, SLOT(TogglePsychedelicColors()));
  psychedelic_enable_->setCheckable(true);

  context_menu_->addSeparator();
  // Visualisation action gets added in SetActions

  double_click_timer_->setSingleShot(true);
  double_click_timer_->setInterval(250);
  connect(double_click_timer_, SIGNAL(timeout()), SLOT(ShowPopupMenu()));

  Load();
}

void AnalyzerContainer::SetActions(QAction* visualisation) {
  visualisation_action_ = visualisation;
  context_menu_->addAction(visualisation_action_);
}

void AnalyzerContainer::mouseReleaseEvent(QMouseEvent* e) {
  if (e->button() == Qt::LeftButton) {
    if (ignore_next_click_) {
      ignore_next_click_ = false;
    } else {
      // Might be the first click in a double click, so wait a while before
      // actually doing anything
      double_click_timer_->start();
      last_click_pos_ = e->globalPos();
    }
  } else if (e->button() == Qt::RightButton) {
    context_menu_->popup(e->globalPos());
  }
}

void AnalyzerContainer::ShowPopupMenu() {
  context_menu_->popup(last_click_pos_);
}

void AnalyzerContainer::mouseDoubleClickEvent(QMouseEvent*) {
  double_click_timer_->stop();
  ignore_next_click_ = true;

  if (visualisation_action_) visualisation_action_->trigger();
}

void AnalyzerContainer::wheelEvent(QWheelEvent* e) {
  emit WheelEvent(e->delta());
}

void AnalyzerContainer::SetEngine(EngineBase* engine) {
  if (current_analyzer_) current_analyzer_->set_engine(engine);
  engine_ = engine;
}

void AnalyzerContainer::DisableAnalyzer() {
  delete current_analyzer_;
  current_analyzer_ = nullptr;

  Save();
}

void AnalyzerContainer::TogglePsychedelicColors() {
  psychedelic_colors_on_ = !psychedelic_colors_on_;
  if (current_analyzer_) {
    current_analyzer_->psychedelicModeChanged(psychedelic_colors_on_);
  }
  SavePsychedelic();
}

void AnalyzerContainer::ChangeAnalyzer(int id) {
  QObject* instance = analyzer_types_[id]->newInstance(Q_ARG(QWidget*, this));

  if (!instance) {
    qLog(Warning) << "Couldn't initialise a new"
                  << analyzer_types_[id]->className();
    return;
  }

  delete current_analyzer_;
  current_analyzer_ = qobject_cast<Analyzer::Base*>(instance);
  current_analyzer_->set_engine(engine_);
  // Even if it is not supposed to happen, I don't want to get a dbz error
  current_framerate_ =
      current_framerate_ == 0 ? kMediumFramerate : current_framerate_;
  current_analyzer_->changeTimeout(1000 / current_framerate_);
  current_analyzer_->psychedelicModeChanged(psychedelic_colors_on_);

  layout()->addWidget(current_analyzer_);

  Save();
}

void AnalyzerContainer::ChangeFramerate(int new_framerate) {
  if (current_analyzer_) {
    // Even if it is not supposed to happen, I don't want to get a dbz error
    new_framerate = new_framerate == 0 ? kMediumFramerate : new_framerate;
    current_analyzer_->changeTimeout(1000 / new_framerate);

    // notify the current analyzer that the framerate has changed
    current_analyzer_->framerateChanged();
  }
  SaveFramerate(new_framerate);
}

void AnalyzerContainer::Load() {
  QSettings s;
  s.beginGroup(kSettingsGroup);

  // Colours
  psychedelic_colors_on_ = s.value("psychedelic", false).toBool();
  psychedelic_enable_->setChecked(psychedelic_colors_on_);

  // Analyzer
  QString type = s.value("type", "BlockAnalyzer").toString();
  if (type.isEmpty()) {
    DisableAnalyzer();
    disable_action_->setChecked(true);
  } else {
    for (int i = 0; i < analyzer_types_.count(); ++i) {
      if (type == analyzer_types_[i]->className()) {
        ChangeAnalyzer(i);
        actions_[i]->setChecked(true);
        break;
      }
    }
  }

  // Framerate
  current_framerate_ = s.value(kSettingsFramerate, kMediumFramerate).toInt();
  for (int i = 0; i < framerate_list_.count(); ++i) {
    if (current_framerate_ == framerate_list_[i]) {
      ChangeFramerate(current_framerate_);
      group_framerate_->actions()[i]->setChecked(true);
      break;
    }
  }
}

void AnalyzerContainer::SaveFramerate(int framerate) {
  // For now, framerate is common for all analyzers.
  // Maybe each analyzer should have its own framerate?
  current_framerate_ = framerate;
  QSettings s;
  s.beginGroup(kSettingsGroup);
  s.setValue(kSettingsFramerate, current_framerate_);
}

void AnalyzerContainer::Save() {
  QSettings s;
  s.beginGroup(kSettingsGroup);

  s.setValue("type", current_analyzer_
                         ? current_analyzer_->metaObject()->className()
                         : QVariant());
}

void AnalyzerContainer::SavePsychedelic() {
  QSettings s;
  s.beginGroup(kSettingsGroup);

  s.setValue("psychedelic", psychedelic_colors_on_);
}

void AnalyzerContainer::AddFramerate(const QString& name, int framerate) {
  QAction* action = context_menu_framerate_->addAction(name);
  group_framerate_->addAction(action);
  framerate_list_ << framerate;
  action->setCheckable(true);
  connect(action, &QAction::triggered,
          [this, framerate]() { ChangeFramerate(framerate); });
}
