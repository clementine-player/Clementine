class ScriptInterface : QObject {

%TypeHeaderCode
#include "scripting/scriptinterface.h"
%End

%Docstring
Helper functions that are specific to a single script running inside Clementine.

An instance of this class is created as a global variable in a script's main
package, similar to the Python global variables C{__name__} and C{__file__}.
You can access it through C{script}:

  >>> def show_settings_dialog():
  ...   pass
  ...
  ... script.SettingsDialogRequested().connect(show_settings_dialog())

The signals in this object are emitted by Clementine when the user does things
related to your script, such as clicking on the Settings... button.  You should
connect these signals to slots in your Python code and respond to them
appropriately.

@group Signals: SettingsDialogRequested
%End

%ConvertToSubClassCode
  // Any classes that inherit from QObject must be added to this list

  #define CLASS(name) {sipName_##name, &sipType_##name}
  static struct class_list {
    const char *name;
    sipTypeDef **type;
  } list[] = {
    CLASS(LibraryBackend),
    CLASS(MergedProxyModel),
    CLASS(NetworkAccessManager),
    CLASS(ParserBase),
    CLASS(PlayerInterface),
    CLASS(Playlist),
    CLASS(PlaylistManagerInterface),
    CLASS(PlaylistParser),
    CLASS(PlaylistSequence),
    CLASS(Queue),
    CLASS(RadioModel),
    CLASS(RadioService),
    CLASS(ScriptInterface),
    CLASS(SettingsDialog),
    CLASS(SongInsertVetoListener),
    CLASS(SongLoader),
    CLASS(TaskManager),
    CLASS(UIInterface),
    {0, 0}
  };
  #undef CLASS

  sipType = NULL;

  for (int i=0 ; ; ++i) {
    struct class_list* p = &list[i];
    if (!p->name) {
      break;
    }

    if (sipCpp->inherits(p->name)) {
      sipType = *p->type;
      break;
    }
  }
%End

signals:
  void SettingsDialogRequested();
%Docstring
SettingsDialogRequested()
Emitted when the user clicks the Settings... button in the Script Manager
dialog.
%End

private:
  ScriptInterface();
};
