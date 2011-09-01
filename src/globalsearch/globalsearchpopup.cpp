#include "globalsearchpopup.h"


GlobalSearchPopup::GlobalSearchPopup(QWidget* parent)
    : QDialog(parent),
      ui_(new Ui_GlobalSearchPopup) {
  ui_->setupUi(this);
}
