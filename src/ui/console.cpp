#include "console.h"

#include <QFont>
#include <QScrollBar>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlRecord>

#include "core/application.h"
#include "core/database.h"
#include "core/logging.h"

Console::Console(Application* app, QWidget* parent)
    : QDialog(parent), app_(app) {
  ui_.setupUi(this);
  connect(ui_.database_run, SIGNAL(clicked()), SLOT(RunQuery()));
  connect(ui_.qt_dump_button, SIGNAL(clicked()), SLOT(Dump()));

  QFont font("Monospace");
  font.setStyleHint(QFont::TypeWriter);

  ui_.database_output->setFont(font);
  ui_.database_query->setFont(font);

  QList<QObject*> objs = GetTopLevelObjects();
  for (QObject* obj : objs)
    ui_.qt_dump_box->addItem(obj->objectName() + " object tree",
                             obj->objectName());
}

void Console::RunQuery() {
  QSqlDatabase db = app_->database()->Connect();
  QSqlQuery query = db.exec(ui_.database_query->text());
  ui_.database_query->clear();

  ui_.database_output->append("<b>&gt; " + query.executedQuery() + "</b>");

  query.next();

  while (query.isValid()) {
    QSqlRecord record = query.record();
    QStringList values;
    for (int i = 0; i < record.count(); ++i) {
      values.append(record.value(i).toString());
    }

    ui_.database_output->append(values.join("|"));

    query.next();
  }

  ui_.database_output->verticalScrollBar()->setValue(
      ui_.database_output->verticalScrollBar()->maximum());
}

void Console::Dump() {
  QString item = ui_.qt_dump_box->currentData().toString();
  QObject* obj = FindTopLevelObject(item);
  if (obj == nullptr) {
    qLog(Error) << "Object not found" << item;
    return;
  }
  obj->dumpObjectTree();
}

QList<QObject*> Console::GetTopLevelObjects() {
  QList<QObject*> objs;
  objs << app_;
  // The parent should be the main window.
  if (parent() != nullptr) objs << parent();

  return objs;
}

QObject* Console::FindTopLevelObject(QString& name) {
  for (QObject* obj : GetTopLevelObjects())
    if (obj->objectName() == name) return obj;
  return nullptr;
}

void Console::AddPage(QWidget* page, const QString& label) {
  ui_.tabWidget->addTab(page, label);
}
