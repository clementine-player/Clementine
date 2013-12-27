#include "ripcd.h"
#include "config.h"
#include "ui_ripcd.h"


RipCD::RipCD(QWidget* parent)
  : QDialog(parent)
{
  ui_.setupUi(this);
  setWindowTitle(tr("Rip CD"));
}
