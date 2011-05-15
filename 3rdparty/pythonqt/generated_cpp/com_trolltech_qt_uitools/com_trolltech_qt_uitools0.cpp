#include "com_trolltech_qt_uitools0.h"
#include <PythonQtConversion.h>
#include <PythonQtMethodInfo.h>
#include <PythonQtSignalReceiver.h>
#include <QVariant>
#include <qaction.h>
#include <qactiongroup.h>
#include <qbytearray.h>
#include <qcoreevent.h>
#include <qdir.h>
#include <qiodevice.h>
#include <qlayout.h>
#include <qlist.h>
#include <qobject.h>
#include <qstringlist.h>
#include <quiloader.h>
#include <qwidget.h>

void PythonQtShell_QUiLoader::childEvent(QChildEvent*  arg__1)
{
if (_wrapper) {
  PyObject* obj = PyObject_GetAttrString((PyObject*)_wrapper, "childEvent");
  PyErr_Clear();
  if (obj && !PythonQtSlotFunction_Check(obj)) {
    static const char* argumentList[] ={"" , "QChildEvent*"};
    static const PythonQtMethodInfo* methodInfo = PythonQtMethodInfo::getCachedMethodInfoFromArgumentList(2, argumentList);
    void* args[2] = {NULL, (void*)&arg__1};
    PyObject* result = PythonQtSignalTarget::call(obj, methodInfo, args, true);
    if (result) { Py_DECREF(result); } 
    Py_DECREF(obj);
    return;
  }
}
  QUiLoader::childEvent(arg__1);
}
QAction*  PythonQtShell_QUiLoader::createAction(QObject*  parent, const QString&  name)
{
if (_wrapper) {
  PyObject* obj = PyObject_GetAttrString((PyObject*)_wrapper, "createAction");
  PyErr_Clear();
  if (obj && !PythonQtSlotFunction_Check(obj)) {
    static const char* argumentList[] ={"QAction*" , "QObject*" , "const QString&"};
    static const PythonQtMethodInfo* methodInfo = PythonQtMethodInfo::getCachedMethodInfoFromArgumentList(3, argumentList);
      QAction* returnValue;
    void* args[3] = {NULL, (void*)&parent, (void*)&name};
    PyObject* result = PythonQtSignalTarget::call(obj, methodInfo, args, true);
    if (result) {
      args[0] = PythonQtConv::ConvertPythonToQt(methodInfo->parameters().at(0), result, false, NULL, &returnValue);
      if (args[0]!=&returnValue) {
        if (args[0]==NULL) {
          PythonQt::priv()->handleVirtualOverloadReturnError("createAction", methodInfo, result);
        } else {
          returnValue = *((QAction**)args[0]);
        }
      }
    }
    if (result) { Py_DECREF(result); } 
    Py_DECREF(obj);
    return returnValue;
  }
}
  return QUiLoader::createAction(parent, name);
}
QActionGroup*  PythonQtShell_QUiLoader::createActionGroup(QObject*  parent, const QString&  name)
{
if (_wrapper) {
  PyObject* obj = PyObject_GetAttrString((PyObject*)_wrapper, "createActionGroup");
  PyErr_Clear();
  if (obj && !PythonQtSlotFunction_Check(obj)) {
    static const char* argumentList[] ={"QActionGroup*" , "QObject*" , "const QString&"};
    static const PythonQtMethodInfo* methodInfo = PythonQtMethodInfo::getCachedMethodInfoFromArgumentList(3, argumentList);
      QActionGroup* returnValue;
    void* args[3] = {NULL, (void*)&parent, (void*)&name};
    PyObject* result = PythonQtSignalTarget::call(obj, methodInfo, args, true);
    if (result) {
      args[0] = PythonQtConv::ConvertPythonToQt(methodInfo->parameters().at(0), result, false, NULL, &returnValue);
      if (args[0]!=&returnValue) {
        if (args[0]==NULL) {
          PythonQt::priv()->handleVirtualOverloadReturnError("createActionGroup", methodInfo, result);
        } else {
          returnValue = *((QActionGroup**)args[0]);
        }
      }
    }
    if (result) { Py_DECREF(result); } 
    Py_DECREF(obj);
    return returnValue;
  }
}
  return QUiLoader::createActionGroup(parent, name);
}
QLayout*  PythonQtShell_QUiLoader::createLayout(const QString&  className, QObject*  parent, const QString&  name)
{
if (_wrapper) {
  PyObject* obj = PyObject_GetAttrString((PyObject*)_wrapper, "createLayout");
  PyErr_Clear();
  if (obj && !PythonQtSlotFunction_Check(obj)) {
    static const char* argumentList[] ={"QLayout*" , "const QString&" , "QObject*" , "const QString&"};
    static const PythonQtMethodInfo* methodInfo = PythonQtMethodInfo::getCachedMethodInfoFromArgumentList(4, argumentList);
      QLayout* returnValue;
    void* args[4] = {NULL, (void*)&className, (void*)&parent, (void*)&name};
    PyObject* result = PythonQtSignalTarget::call(obj, methodInfo, args, true);
    if (result) {
      args[0] = PythonQtConv::ConvertPythonToQt(methodInfo->parameters().at(0), result, false, NULL, &returnValue);
      if (args[0]!=&returnValue) {
        if (args[0]==NULL) {
          PythonQt::priv()->handleVirtualOverloadReturnError("createLayout", methodInfo, result);
        } else {
          returnValue = *((QLayout**)args[0]);
        }
      }
    }
    if (result) { Py_DECREF(result); } 
    Py_DECREF(obj);
    return returnValue;
  }
}
  return QUiLoader::createLayout(className, parent, name);
}
QWidget*  PythonQtShell_QUiLoader::createWidget(const QString&  className, QWidget*  parent, const QString&  name)
{
if (_wrapper) {
  PyObject* obj = PyObject_GetAttrString((PyObject*)_wrapper, "createWidget");
  PyErr_Clear();
  if (obj && !PythonQtSlotFunction_Check(obj)) {
    static const char* argumentList[] ={"QWidget*" , "const QString&" , "QWidget*" , "const QString&"};
    static const PythonQtMethodInfo* methodInfo = PythonQtMethodInfo::getCachedMethodInfoFromArgumentList(4, argumentList);
      QWidget* returnValue;
    void* args[4] = {NULL, (void*)&className, (void*)&parent, (void*)&name};
    PyObject* result = PythonQtSignalTarget::call(obj, methodInfo, args, true);
    if (result) {
      args[0] = PythonQtConv::ConvertPythonToQt(methodInfo->parameters().at(0), result, false, NULL, &returnValue);
      if (args[0]!=&returnValue) {
        if (args[0]==NULL) {
          PythonQt::priv()->handleVirtualOverloadReturnError("createWidget", methodInfo, result);
        } else {
          returnValue = *((QWidget**)args[0]);
        }
      }
    }
    if (result) { Py_DECREF(result); } 
    Py_DECREF(obj);
    return returnValue;
  }
}
  return QUiLoader::createWidget(className, parent, name);
}
void PythonQtShell_QUiLoader::customEvent(QEvent*  arg__1)
{
if (_wrapper) {
  PyObject* obj = PyObject_GetAttrString((PyObject*)_wrapper, "customEvent");
  PyErr_Clear();
  if (obj && !PythonQtSlotFunction_Check(obj)) {
    static const char* argumentList[] ={"" , "QEvent*"};
    static const PythonQtMethodInfo* methodInfo = PythonQtMethodInfo::getCachedMethodInfoFromArgumentList(2, argumentList);
    void* args[2] = {NULL, (void*)&arg__1};
    PyObject* result = PythonQtSignalTarget::call(obj, methodInfo, args, true);
    if (result) { Py_DECREF(result); } 
    Py_DECREF(obj);
    return;
  }
}
  QUiLoader::customEvent(arg__1);
}
bool  PythonQtShell_QUiLoader::event(QEvent*  arg__1)
{
if (_wrapper) {
  PyObject* obj = PyObject_GetAttrString((PyObject*)_wrapper, "event");
  PyErr_Clear();
  if (obj && !PythonQtSlotFunction_Check(obj)) {
    static const char* argumentList[] ={"bool" , "QEvent*"};
    static const PythonQtMethodInfo* methodInfo = PythonQtMethodInfo::getCachedMethodInfoFromArgumentList(2, argumentList);
      bool returnValue;
    void* args[2] = {NULL, (void*)&arg__1};
    PyObject* result = PythonQtSignalTarget::call(obj, methodInfo, args, true);
    if (result) {
      args[0] = PythonQtConv::ConvertPythonToQt(methodInfo->parameters().at(0), result, false, NULL, &returnValue);
      if (args[0]!=&returnValue) {
        if (args[0]==NULL) {
          PythonQt::priv()->handleVirtualOverloadReturnError("event", methodInfo, result);
        } else {
          returnValue = *((bool*)args[0]);
        }
      }
    }
    if (result) { Py_DECREF(result); } 
    Py_DECREF(obj);
    return returnValue;
  }
}
  return QUiLoader::event(arg__1);
}
bool  PythonQtShell_QUiLoader::eventFilter(QObject*  arg__1, QEvent*  arg__2)
{
if (_wrapper) {
  PyObject* obj = PyObject_GetAttrString((PyObject*)_wrapper, "eventFilter");
  PyErr_Clear();
  if (obj && !PythonQtSlotFunction_Check(obj)) {
    static const char* argumentList[] ={"bool" , "QObject*" , "QEvent*"};
    static const PythonQtMethodInfo* methodInfo = PythonQtMethodInfo::getCachedMethodInfoFromArgumentList(3, argumentList);
      bool returnValue;
    void* args[3] = {NULL, (void*)&arg__1, (void*)&arg__2};
    PyObject* result = PythonQtSignalTarget::call(obj, methodInfo, args, true);
    if (result) {
      args[0] = PythonQtConv::ConvertPythonToQt(methodInfo->parameters().at(0), result, false, NULL, &returnValue);
      if (args[0]!=&returnValue) {
        if (args[0]==NULL) {
          PythonQt::priv()->handleVirtualOverloadReturnError("eventFilter", methodInfo, result);
        } else {
          returnValue = *((bool*)args[0]);
        }
      }
    }
    if (result) { Py_DECREF(result); } 
    Py_DECREF(obj);
    return returnValue;
  }
}
  return QUiLoader::eventFilter(arg__1, arg__2);
}
void PythonQtShell_QUiLoader::timerEvent(QTimerEvent*  arg__1)
{
if (_wrapper) {
  PyObject* obj = PyObject_GetAttrString((PyObject*)_wrapper, "timerEvent");
  PyErr_Clear();
  if (obj && !PythonQtSlotFunction_Check(obj)) {
    static const char* argumentList[] ={"" , "QTimerEvent*"};
    static const PythonQtMethodInfo* methodInfo = PythonQtMethodInfo::getCachedMethodInfoFromArgumentList(2, argumentList);
    void* args[2] = {NULL, (void*)&arg__1};
    PyObject* result = PythonQtSignalTarget::call(obj, methodInfo, args, true);
    if (result) { Py_DECREF(result); } 
    Py_DECREF(obj);
    return;
  }
}
  QUiLoader::timerEvent(arg__1);
}
QUiLoader* PythonQtWrapper_QUiLoader::new_QUiLoader(QObject*  parent)
{ 
return new PythonQtShell_QUiLoader(parent); }

void PythonQtWrapper_QUiLoader::addPluginPath(QUiLoader* theWrappedObject, const QString&  path)
{
  ( theWrappedObject->addPluginPath(path));
}

QStringList  PythonQtWrapper_QUiLoader::availableLayouts(QUiLoader* theWrappedObject) const
{
  return ( theWrappedObject->availableLayouts());
}

QStringList  PythonQtWrapper_QUiLoader::availableWidgets(QUiLoader* theWrappedObject) const
{
  return ( theWrappedObject->availableWidgets());
}

void PythonQtWrapper_QUiLoader::clearPluginPaths(QUiLoader* theWrappedObject)
{
  ( theWrappedObject->clearPluginPaths());
}

QAction*  PythonQtWrapper_QUiLoader::createAction(QUiLoader* theWrappedObject, QObject*  parent, const QString&  name)
{
  return ( ((PythonQtPublicPromoter_QUiLoader*)theWrappedObject)->promoted_createAction(parent, name));
}

QActionGroup*  PythonQtWrapper_QUiLoader::createActionGroup(QUiLoader* theWrappedObject, QObject*  parent, const QString&  name)
{
  return ( ((PythonQtPublicPromoter_QUiLoader*)theWrappedObject)->promoted_createActionGroup(parent, name));
}

QLayout*  PythonQtWrapper_QUiLoader::createLayout(QUiLoader* theWrappedObject, const QString&  className, QObject*  parent, const QString&  name)
{
  return ( ((PythonQtPublicPromoter_QUiLoader*)theWrappedObject)->promoted_createLayout(className, parent, name));
}

QWidget*  PythonQtWrapper_QUiLoader::createWidget(QUiLoader* theWrappedObject, const QString&  className, QWidget*  parent, const QString&  name)
{
  return ( ((PythonQtPublicPromoter_QUiLoader*)theWrappedObject)->promoted_createWidget(className, parent, name));
}

bool  PythonQtWrapper_QUiLoader::isLanguageChangeEnabled(QUiLoader* theWrappedObject) const
{
  return ( theWrappedObject->isLanguageChangeEnabled());
}

bool  PythonQtWrapper_QUiLoader::isScriptingEnabled(QUiLoader* theWrappedObject) const
{
  return ( theWrappedObject->isScriptingEnabled());
}

bool  PythonQtWrapper_QUiLoader::isTranslationEnabled(QUiLoader* theWrappedObject) const
{
  return ( theWrappedObject->isTranslationEnabled());
}

QWidget*  PythonQtWrapper_QUiLoader::load(QUiLoader* theWrappedObject, QIODevice*  device, QWidget*  parentWidget)
{
  return ( theWrappedObject->load(device, parentWidget));
}

QStringList  PythonQtWrapper_QUiLoader::pluginPaths(QUiLoader* theWrappedObject) const
{
  return ( theWrappedObject->pluginPaths());
}

void PythonQtWrapper_QUiLoader::setLanguageChangeEnabled(QUiLoader* theWrappedObject, bool  enabled)
{
  ( theWrappedObject->setLanguageChangeEnabled(enabled));
}

void PythonQtWrapper_QUiLoader::setScriptingEnabled(QUiLoader* theWrappedObject, bool  enabled)
{
  ( theWrappedObject->setScriptingEnabled(enabled));
}

void PythonQtWrapper_QUiLoader::setTranslationEnabled(QUiLoader* theWrappedObject, bool  enabled)
{
  ( theWrappedObject->setTranslationEnabled(enabled));
}

void PythonQtWrapper_QUiLoader::setWorkingDirectory(QUiLoader* theWrappedObject, const QDir&  dir)
{
  ( theWrappedObject->setWorkingDirectory(dir));
}

QDir  PythonQtWrapper_QUiLoader::workingDirectory(QUiLoader* theWrappedObject) const
{
  return ( theWrappedObject->workingDirectory());
}


