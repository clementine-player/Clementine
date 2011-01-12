class SettingsDialog : QDialog {

%TypeHeaderCode
#include "radio/radioservice.h"
%End

public:
  enum Page {
    Page_Playback,
    Page_Behaviour,
    Page_SongInformation,
    Page_GlobalShortcuts,
    Page_Notifications,
    Page_Library,
    Page_Magnatune,
    Page_BackgroundStreams,
    Page_Proxy,
  };

  void OpenAtPage(Page page);

private:
  SettingsDialog();
};
