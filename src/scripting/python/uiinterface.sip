class UIInterface : QObject {

%TypeHeaderCode
#include "scripting/uiinterface.h"
#include "scripting/python/pythonengine.h"
%End

public:
  void AddAction(const QString& id, QAction* action /Transfer/);
%MethodCode
  sipCpp->AddAction(*a0,a1);
  PythonEngine::instance()->RegisterNativeObject(a1);
%End

private:
  UIInterface();
};
