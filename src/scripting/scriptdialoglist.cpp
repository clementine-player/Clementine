#include "scriptdialoglist.h"

#include <QScrollBar>

ScriptDialogList::ScriptDialogList(QWidget* parent)
  : QListView(parent) {
}

void ScriptDialogList::updateGeometries() {
  QListView::updateGeometries();

  verticalScrollBar()->setSingleStep(15);
}
