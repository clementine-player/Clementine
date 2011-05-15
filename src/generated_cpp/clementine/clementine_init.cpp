#include <PythonQt.h>
#include "clementine0.h"


void PythonQt_init_Clementine(PyObject* module) {
PythonQt::priv()->registerClass(&RadioModel::staticMetaObject, "Clementine", PythonQtCreateObject<PythonQtWrapper_RadioModel>, NULL, module, 0);
PythonQt::priv()->registerClass(&RadioService::staticMetaObject, "Clementine", PythonQtCreateObject<PythonQtWrapper_RadioService>, PythonQtSetInstanceWrapperOnShell<PythonQtShell_RadioService>, module, 0);
PythonQt::priv()->registerClass(&UrlHandler::staticMetaObject, "Clementine", PythonQtCreateObject<PythonQtWrapper_UrlHandler>, PythonQtSetInstanceWrapperOnShell<PythonQtShell_UrlHandler>, module, 0);
PythonQt::priv()->registerCPPClass("UrlHandler::LoadResult", "", "Clementine", PythonQtCreateObject<PythonQtWrapper_UrlHandler_LoadResult>, PythonQtSetInstanceWrapperOnShell<PythonQtShell_UrlHandler_LoadResult>, module, 0);

}
