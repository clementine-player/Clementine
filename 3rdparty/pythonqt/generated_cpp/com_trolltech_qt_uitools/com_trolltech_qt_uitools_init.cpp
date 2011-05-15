#include <PythonQt.h>
#include "com_trolltech_qt_uitools0.h"
#include "com_trolltech_qt_uitools_init.h"


void PythonQt_init_QtUiTools(PyObject* module) {
PythonQt::priv()->registerClass(&QUiLoader::staticMetaObject, "QtUiTools", PythonQtCreateObject<PythonQtWrapper_QUiLoader>, PythonQtSetInstanceWrapperOnShell<PythonQtShell_QUiLoader>, module, 0);

}
