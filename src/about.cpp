#include "about.h"
#include "ui_about.h"

#include <QCoreApplication>

About::About(QWidget *parent)
  : QDialog(parent)
{
  ui_.setupUi(this);

  setWindowTitle("About " + QCoreApplication::applicationName());
  ui_.title->setText(QCoreApplication::applicationName());
  ui_.version->setText("Version " + QCoreApplication::applicationVersion());
}
