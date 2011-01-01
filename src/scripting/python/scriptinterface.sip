class ScriptInterface : QObject {

%TypeHeaderCode
#include "scripting/scriptinterface.h"
%End

signals:
  void SettingsDialogRequested();

private:
  ScriptInterface();
};
