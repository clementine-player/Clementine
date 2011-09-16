#include "globalsearchpopup.h"

#include "ApplicationServices/ApplicationServices.h"

void GlobalSearchPopup::StorePreviousProcess() {
  mac_psn_ = new ProcessSerialNumber;
  GetFrontProcess(mac_psn_);
}

void GlobalSearchPopup::ActivatePreviousProcess() {
  SetFrontProcess(mac_psn_);
  delete mac_psn_;
  mac_psn_ = NULL;
}
