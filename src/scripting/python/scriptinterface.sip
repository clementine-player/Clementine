class ScriptInterface : QObject {

%TypeHeaderCode
#include "scripting/scriptinterface.h"
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
    CLASS(Player),
    CLASS(Playlist),
    CLASS(PlaylistManager),
    CLASS(PlaylistParser),
    CLASS(PlaylistSequence),
    CLASS(Queue),
    CLASS(RadioModel),
    CLASS(RadioService),
    CLASS(ScriptInterface),
    CLASS(SettingsDialog),
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

public:
  void Abort();

signals:
  void SettingsDialogRequested();

private:
  ScriptInterface();
};
