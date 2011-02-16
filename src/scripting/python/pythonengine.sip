class PythonEngine {

%TypeHeaderCode
#include "scripting/python/pythonengine.h"
%End

%Docstring
Helper functions for communicating with Clementine's Python interface.

This is a singleton class, you can access its methods directly through the
C{clementine.pythonengine} object:

  >>> clementine.pythonengine.AddLogLine("Everything is ok")
%End

public:
  void AddLogLine(const QString& message, bool error = false);
%Docstring
AddLogLine(message, error=false)
Writes a line to the Clementine script console.

Note that sys.stdout and sys.stderr are already redirected to the script
console, so you will not normally need to call this function.  Instead you can
use C{print} directly::

  # These lines are equivalent
  print "Everything is ok"
  clementine.pythonengine.AddLogLine("Everything is ok")

  # These lines are equivalent
  print >>sys.stderr, "Everything is wrong"
  clementine.pythonengine.AddLogLine("Everything is wrong", error=True)

@param message: A line to write to the log.
@type message: str
@param error: If this is True then the message will appear in red.
@type error: bool
%End

private:
  PythonEngine();
};
