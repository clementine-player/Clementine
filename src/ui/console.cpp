#include "console.h"

#include <QFont>
#include <QScrollBar>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlRecord>

#include "core/application.h"
#include "core/database.h"

Console::Console(Application* app, QWidget* parent)
    : QDialog(parent), app_(app) {
  ui_.setupUi(this);
  connect(ui_.run, SIGNAL(clicked()), SLOT(RunQuery()));

  QFont font("Monospace");
  font.setStyleHint(QFont::TypeWriter);

  ui_.output->setFont(font);
  ui_.query->setFont(font);
}

void Console::RunQuery() {
  QSqlDatabase db = app_->database()->Connect();
  QSqlQuery query = db.exec(ui_.query->text());
  ui_.query->clear();

  ui_.output->append("<b>&gt; " + query.executedQuery() + "</b>");

  query.next();

  while (query.isValid()) {
    QSqlRecord record = query.record();
    QStringList values;
    for (int i = 0; i < record.count(); ++i) {
      values.append(record.value(i).toString());
    }

    ui_.output->append(values.join("|"));

    query.next();
  }

  ui_.output->verticalScrollBar()->setValue(
      ui_.output->verticalScrollBar()->maximum());
}
