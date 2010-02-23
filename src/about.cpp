#include "about.h"
#include "ui_about.h"

#include <QCoreApplication>

About::About(QWidget *parent)
  : QDialog(parent)
{
  ui_.setupUi(this);

  setWindowTitle(tr("About %1").arg(QCoreApplication::applicationName()));
  ui_.title->setText(QCoreApplication::applicationName());
  ui_.version->setText(tr("Version %1").arg(QCoreApplication::applicationVersion()));
}
