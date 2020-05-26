#ifndef CONSOLE_H
#define CONSOLE_H

#include <QDialog>

#include "ui_console.h"

class Application;

class Console : public QDialog {
  Q_OBJECT
 public:
  Console(Application* app, QWidget* parent = nullptr);

 private slots:
  // Database
  void RunQuery();
  // Qt
  void Dump();

 private:
  QList<QObject*> GetTopLevelObjects();
  QObject* FindTopLevelObject(QString& name);
  Ui::Console ui_;
  Application* app_;
};

#endif  // CONSOLE_H
