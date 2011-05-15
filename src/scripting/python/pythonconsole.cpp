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

// Needs to be included first
#include "pythonengine.h"

#include "pythonconsole.h"
#include "ui_pythonconsole.h"

#include <gui/PythonQtScriptingConsole.h>

#include <QSortFilterProxyModel>
#include <QStandardItemModel>

PythonConsole::PythonConsole(QWidget *parent)
  : QWidget(parent),
    ui_(new Ui_PythonConsole),
    proxy_model_(new QSortFilterProxyModel(this)),
    engine_(NULL),
    widget_(NULL)
{
  ui_->setupUi(this);
  connect(ui_->modules, SIGNAL(currentIndexChanged(int)), SLOT(ModuleChanged(int)));
}

PythonConsole::~PythonConsole() {
  delete ui_;
}

void PythonConsole::showEvent(QShowEvent* e) {
  engine_->EnsureInitialised();

  QWidget::showEvent(e);
}

void PythonConsole::SetEngine(PythonEngine* engine) {
  engine_ = engine;

  proxy_model_->setSourceModel(engine->modules_model());
  proxy_model_->setDynamicSortFilter(true);

  ui_->modules->setModel(proxy_model_);
}

void PythonConsole::ModuleChanged(int row) {
  const QModelIndex index = proxy_model_->index(row, 0);

  PythonQtObjectPtr ptr;
  ptr.fromVariant(index.data(Qt::UserRole + 1));

  delete widget_;
  widget_ = new PythonQtScriptingConsole(this, ptr);

  layout()->addWidget(widget_);
}
