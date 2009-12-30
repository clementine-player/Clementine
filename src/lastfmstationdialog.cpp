#include "lastfmstationdialog.h"
#include "ui_lastfmstationdialog.h"

LastFMStationDialog::LastFMStationDialog(QWidget* parent)
  : QDialog(parent)
{
  ui_.setupUi(this);
}

void LastFMStationDialog::SetType(Type type) {
  ui_.type->setCurrentIndex(type);
  ui_.content->clear();
  ui_.content->setFocus(Qt::OtherFocusReason);
}

QString LastFMStationDialog::content() const {
  return ui_.content->text();
}
