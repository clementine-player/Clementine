class PythonEngine {

%TypeHeaderCode
#include "scripting/python/pythonengine.h"
%End

public:
  void AddLogLine(const QString& message, bool error = false);

private:
  PythonEngine();
};
