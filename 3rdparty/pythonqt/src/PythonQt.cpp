/*
*
*  Copyright (C) 2010 MeVis Medical Solutions AG All Rights Reserved.
*
*  This library is free software; you can redistribute it and/or
*  modify it under the terms of the GNU Lesser General Public
*  License as published by the Free Software Foundation; either
*  version 2.1 of the License, or (at your option) any later version.
*
*  This library is distributed in the hope that it will be useful,
*  but WITHOUT ANY WARRANTY; without even the implied warranty of
*  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
*  Lesser General Public License for more details.
*
*  Further, this software is distributed without any warranty that it is
*  free of the rightful claim of any third person regarding infringement
*  or the like.  Any license provided herein, whether implied or
*  otherwise, applies only to this software file.  Patent licenses, if
*  any, provided herein do not apply to combinations of this program with
*  other software, or any other product whatsoever.
*
*  You should have received a copy of the GNU Lesser General Public
*  License along with this library; if not, write to the Free Software
*  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*
*  Contact information: MeVis Medical Solutions AG, Universitaetsallee 29,
*  28359 Bremen, Germany or:
*
*  http://www.mevis.de
*
*/

//----------------------------------------------------------------------------------
/*!
// \file    PythonQt.cpp
// \author  Florian Link
// \author  Last changed by $Author: florian $
// \date    2006-05
*/
//----------------------------------------------------------------------------------

#include "PythonQt.h"
#include "PythonQtImporter.h"
#include "PythonQtClassInfo.h"
#include "PythonQtMethodInfo.h"
#include "PythonQtSignalReceiver.h"
#include "PythonQtConversion.h"
#include "PythonQtStdOut.h"
#include "PythonQtCppWrapperFactory.h"
#include "PythonQtVariants.h"
#include "PythonQtStdDecorators.h"
#include "PythonQtQFileImporter.h"
#include <pydebug.h>
#include <vector>

PythonQt* PythonQt::_self = NULL;
int       PythonQt::_uniqueModuleCount = 0;

void PythonQt_init_QtGuiBuiltin(PyObject*);
void PythonQt_init_QtCoreBuiltin(PyObject*);

void PythonQt::init(int flags, const QByteArray& pythonQtModuleName)
{
  if (!_self) {
    _self = new PythonQt(flags, pythonQtModuleName);

    PythonQtMethodInfo::addParameterTypeAlias("QObjectList", "QList<QObject*>");
    qRegisterMetaType<QList<QObject*> >("QList<void*>");

    PythonQtRegisterToolClassesTemplateConverter(int);
    PythonQtRegisterToolClassesTemplateConverter(float);
    PythonQtRegisterToolClassesTemplateConverter(double);
    PythonQtRegisterToolClassesTemplateConverter(qint32);
    PythonQtRegisterToolClassesTemplateConverter(quint32);
    PythonQtRegisterToolClassesTemplateConverter(qint64);
    PythonQtRegisterToolClassesTemplateConverter(quint64);
    // TODO: which other POD types should be available for QList etc.

    PythonQt::self()->addDecorators(new PythonQtStdDecorators());

    PythonQt_init_QtCoreBuiltin(NULL);
    PythonQt_init_QtGuiBuiltin(NULL);
  
    PythonQtRegisterToolClassesTemplateConverter(QByteArray);
    PythonQtRegisterToolClassesTemplateConverter(QDate);
    PythonQtRegisterToolClassesTemplateConverter(QTime);
    PythonQtRegisterToolClassesTemplateConverter(QDateTime);
    PythonQtRegisterToolClassesTemplateConverter(QUrl);
    PythonQtRegisterToolClassesTemplateConverter(QLocale);
    PythonQtRegisterToolClassesTemplateConverter(QRect);
    PythonQtRegisterToolClassesTemplateConverter(QRectF);
    PythonQtRegisterToolClassesTemplateConverter(QSize);
    PythonQtRegisterToolClassesTemplateConverter(QSizeF);
    PythonQtRegisterToolClassesTemplateConverter(QLine);
    PythonQtRegisterToolClassesTemplateConverter(QLineF);
    PythonQtRegisterToolClassesTemplateConverter(QPoint);
    PythonQtRegisterToolClassesTemplateConverter(QPointF);
    PythonQtRegisterToolClassesTemplateConverter(QRegExp);

    PythonQtRegisterToolClassesTemplateConverter(QFont);
    PythonQtRegisterToolClassesTemplateConverter(QPixmap);
    PythonQtRegisterToolClassesTemplateConverter(QBrush);
    PythonQtRegisterToolClassesTemplateConverter(QColor);
    PythonQtRegisterToolClassesTemplateConverter(QPalette);
    PythonQtRegisterToolClassesTemplateConverter(QIcon);
    PythonQtRegisterToolClassesTemplateConverter(QImage);
    PythonQtRegisterToolClassesTemplateConverter(QPolygon);
    PythonQtRegisterToolClassesTemplateConverter(QRegion);
    PythonQtRegisterToolClassesTemplateConverter(QBitmap);
    PythonQtRegisterToolClassesTemplateConverter(QCursor);
    PythonQtRegisterToolClassesTemplateConverter(QSizePolicy);
    PythonQtRegisterToolClassesTemplateConverter(QKeySequence);
    PythonQtRegisterToolClassesTemplateConverter(QPen);
    PythonQtRegisterToolClassesTemplateConverter(QTextLength);
    PythonQtRegisterToolClassesTemplateConverter(QTextFormat);
    PythonQtRegisterToolClassesTemplateConverter(QMatrix);


    PyObject* pack = PythonQt::priv()->packageByName("QtCore");
    PyObject* pack2 = PythonQt::priv()->packageByName("Qt");
    PyObject* qtNamespace = PythonQt::priv()->getClassInfo("Qt")->pythonQtClassWrapper();
    const char* names[16] = {"SIGNAL", "SLOT", "qAbs", "qBound","qDebug","qWarning","qCritical","qFatal"
                        ,"qFuzzyCompare", "qMax","qMin","qRound","qRound64","qVersion","qrand","qsrand"};
    for (unsigned int i = 0;i<16; i++) {
      PyObject* obj = PyObject_GetAttrString(qtNamespace, names[i]);
      if (obj) {
        PyModule_AddObject(pack, names[i], obj);
        Py_INCREF(obj);
        PyModule_AddObject(pack2, names[i], obj);
      } else {
        std::cerr << "method not found " << names[i];
      }
    }
  }
}

void PythonQt::cleanup()
{
  if (_self) {
    delete _self;
    _self = NULL;
  }
}

PythonQt::PythonQt(int flags, const QByteArray& pythonQtModuleName)
{
  _p = new PythonQtPrivate;
  _p->_initFlags = flags;

  _p->_PythonQtObjectPtr_metaId = qRegisterMetaType<PythonQtObjectPtr>("PythonQtObjectPtr");

  if ((flags & PythonAlreadyInitialized) == 0) {
    Py_SetProgramName("PythonQt");
    if (flags & IgnoreSiteModule) {
      // this prevents the automatic importing of Python site files
      Py_NoSiteFlag = 1;
    }
    Py_Initialize();
  }
  
  // add our own python object types for qt object slots
  if (PyType_Ready(&PythonQtSlotFunction_Type) < 0) {
    std::cerr << "could not initialize PythonQtSlotFunction_Type" << ", in " << __FILE__ << ":" << __LINE__ << std::endl;
  }
  Py_INCREF(&PythonQtSlotFunction_Type);

  // according to Python docs, set the type late here, since it can not safely be stored in the struct when declaring it
  PythonQtClassWrapper_Type.tp_base = &PyType_Type;
  // add our own python object types for classes
  if (PyType_Ready(&PythonQtClassWrapper_Type) < 0) {
    std::cerr << "could not initialize PythonQtClassWrapper_Type" << ", in " << __FILE__ << ":" << __LINE__ << std::endl;
  }
  Py_INCREF(&PythonQtClassWrapper_Type);

  // add our own python object types for CPP instances
  if (PyType_Ready(&PythonQtInstanceWrapper_Type) < 0) {
    PythonQt::handleError();
    std::cerr << "could not initialize PythonQtInstanceWrapper_Type" << ", in " << __FILE__ << ":" << __LINE__ << std::endl;
  }
  Py_INCREF(&PythonQtInstanceWrapper_Type);

  // add our own python object types for redirection of stdout
  if (PyType_Ready(&PythonQtStdOutRedirectType) < 0) {
    std::cerr << "could not initialize PythonQtStdOutRedirectType" << ", in " << __FILE__ << ":" << __LINE__ << std::endl;
  }
  Py_INCREF(&PythonQtStdOutRedirectType);

  initPythonQtModule(flags & RedirectStdOut, pythonQtModuleName);

  _p->setupSharedLibrarySuffixes();

}

PythonQt::~PythonQt() {
  delete _p;
  _p = NULL;
}

PythonQtPrivate::~PythonQtPrivate() {
  delete _defaultImporter;
  _defaultImporter = NULL;

  {
    QHashIterator<QByteArray, PythonQtClassInfo *> i(_knownClassInfos);
    while (i.hasNext()) {
      delete i.next().value();
    }
  }
  PythonQtConv::global_valueStorage.clear();
  PythonQtConv::global_ptrStorage.clear();
  PythonQtConv::global_variantStorage.clear();

  PythonQtMethodInfo::cleanupCachedMethodInfos();
}

PythonQtImportFileInterface* PythonQt::importInterface()
{
  return _self->_p->_importInterface?_self->_p->_importInterface:_self->_p->_defaultImporter;
}

void PythonQt::qObjectNoLongerWrappedCB(QObject* o)
{
  if (_self->_p->_noLongerWrappedCB) {
    (*_self->_p->_noLongerWrappedCB)(o);
  };
}

void PythonQt::registerClass(const QMetaObject* metaobject, const char* package, PythonQtQObjectCreatorFunctionCB* wrapperCreator, PythonQtShellSetInstanceWrapperCB* shell)
{
  _p->registerClass(metaobject, package, wrapperCreator, shell);
}

void PythonQtPrivate::registerClass(const QMetaObject* metaobject, const char* package, PythonQtQObjectCreatorFunctionCB* wrapperCreator, PythonQtShellSetInstanceWrapperCB* shell, PyObject* module, int typeSlots)
{
  // we register all classes in the hierarchy
  const QMetaObject* m = metaobject;
  bool first = true;
  while (m) {
    PythonQtClassInfo* info = lookupClassInfoAndCreateIfNotPresent(m->className());
    if (!info->pythonQtClassWrapper()) {
      info->setTypeSlots(typeSlots);
      info->setupQObject(m);
      createPythonQtClassWrapper(info, package, module);
      if (m->superClass()) {
        PythonQtClassInfo* parentInfo = lookupClassInfoAndCreateIfNotPresent(m->superClass()->className());
        info->addParentClass(PythonQtClassInfo::ParentClassInfo(parentInfo));
      }
    }
    if (first) {
      first = false;
      if (wrapperCreator) {
        info->setDecoratorProvider(wrapperCreator);
      }
      if (shell) {
        info->setShellSetInstanceWrapperCB(shell);
      }
    }
    m = m->superClass();
  }
}

void PythonQtPrivate::createPythonQtClassWrapper(PythonQtClassInfo* info, const char* package, PyObject* module)
{
  PyObject* pack = module?module:packageByName(package);
  PyObject* pyobj = (PyObject*)createNewPythonQtClassWrapper(info, pack);
  PyModule_AddObject(pack, info->className(), pyobj);
  if (!module && package && strncmp(package,"Qt",2)==0) {
    // since PyModule_AddObject steals the reference, we need a incref once more...
    Py_INCREF(pyobj);
    // put all qt objects into Qt as well
    PyModule_AddObject(packageByName("Qt"), info->className(), pyobj);
  }
  info->setPythonQtClassWrapper(pyobj);
}

PyObject* PythonQtPrivate::wrapQObject(QObject* obj)
{
  if (!obj) {
    Py_INCREF(Py_None);
    return Py_None;
  }
  PythonQtInstanceWrapper* wrap = findWrapperAndRemoveUnused(obj);
  if (!wrap) {
    // smuggling it in...
    PythonQtClassInfo* classInfo = _knownClassInfos.value(obj->metaObject()->className());
    if (!classInfo || classInfo->pythonQtClassWrapper()==NULL) {
      registerClass(obj->metaObject());
      classInfo = _knownClassInfos.value(obj->metaObject()->className());
    }
    wrap = createNewPythonQtInstanceWrapper(obj, classInfo);
    //    mlabDebugConst("MLABPython","new qobject wrapper added " << " " << wrap->_obj->className() << " " << wrap->classInfo()->wrappedClassName().latin1());
  } else {
    Py_INCREF(wrap);
    //    mlabDebugConst("MLABPython","qobject wrapper reused " << wrap->_obj->className() << " " << wrap->classInfo()->wrappedClassName().latin1());
  }
  return (PyObject*)wrap;
}

PyObject* PythonQtPrivate::wrapPtr(void* ptr, const QByteArray& name)
{
  if (!ptr) {
    Py_INCREF(Py_None);
    return Py_None;
  }

  PythonQtInstanceWrapper* wrap = findWrapperAndRemoveUnused(ptr);
  if (!wrap) {
    PythonQtClassInfo* info = _knownClassInfos.value(name);
    if (!info) {
      // maybe it is a PyObject, which we can return directly
      if (name == "PyObject") {
        PyObject* p = (PyObject*)ptr;
        Py_INCREF(p);
        return p;
      }

      // we do not know the metaobject yet, but we might know it by it's name:
      if (_knownQObjectClassNames.find(name)!=_knownQObjectClassNames.end()) {
        // yes, we know it, so we can convert to QObject
        QObject* qptr = (QObject*)ptr;
        registerClass(qptr->metaObject());
        info = _knownClassInfos.value(qptr->metaObject()->className());
      }
    }
    if (info && info->isQObject()) {
      QObject* qptr = (QObject*)ptr;
      // if the object is a derived object, we want to switch the class info to the one of the derived class:
      if (name!=(qptr->metaObject()->className())) {
        registerClass(qptr->metaObject());
        info = _knownClassInfos.value(qptr->metaObject()->className());
      }
      wrap = createNewPythonQtInstanceWrapper(qptr, info);
      //    mlabDebugConst("MLABPython","new qobject wrapper added " << " " << wrap->_obj->className() << " " << wrap->classInfo()->wrappedClassName().latin1());
      return (PyObject*)wrap;
    }

    // not a known QObject, so try our wrapper factory:
    QObject* wrapper = NULL;
    for (int i=0; i<_cppWrapperFactories.size(); i++) {
      wrapper = _cppWrapperFactories.at(i)->create(name, ptr);
      if (wrapper) {
        break;
      }
    }

    if (info) {
      // try to downcast in the class hierarchy, which will modify info and ptr if it is successfull
      ptr  = info->castDownIfPossible(ptr, &info);
    }

    if (!info || info->pythonQtClassWrapper()==NULL) {
      // still unknown, register as CPP class
      registerCPPClass(name.constData());
      info = _knownClassInfos.value(name);
    }
    if (wrapper && (info->metaObject() != wrapper->metaObject())) {
      // if we a have a QObject wrapper and the metaobjects do not match, set the metaobject again!
      info->setMetaObject(wrapper->metaObject());
    }
    wrap = createNewPythonQtInstanceWrapper(wrapper, info, ptr);
    //          mlabDebugConst("MLABPython","new c++ wrapper added " << wrap->_wrappedPtr << " " << wrap->_obj->className() << " " << wrap->classInfo()->wrappedClassName().latin1());
  } else {
    Py_INCREF(wrap);
    //mlabDebugConst("MLABPython","c++ wrapper reused " << wrap->_wrappedPtr << " " << wrap->_obj->className() << " " << wrap->classInfo()->wrappedClassName().latin1());
  }
  return (PyObject*)wrap;
}

PyObject* PythonQtPrivate::dummyTuple() {
  static PyObject* dummyTuple = NULL;
  if (dummyTuple==NULL) {
    dummyTuple = PyTuple_New(1);
    PyTuple_SET_ITEM(dummyTuple, 0 , PyString_FromString("dummy"));
  }
  return dummyTuple;
}


PythonQtInstanceWrapper* PythonQtPrivate::createNewPythonQtInstanceWrapper(QObject* obj, PythonQtClassInfo* info, void* wrappedPtr) {
  // call the associated class type to create a new instance...
  PythonQtInstanceWrapper* result = (PythonQtInstanceWrapper*)PyObject_Call(info->pythonQtClassWrapper(), dummyTuple(), NULL);

  result->setQObject(obj);
  result->_wrappedPtr = wrappedPtr;
  result->_ownedByPythonQt = false;
  result->_useQMetaTypeDestroy = false;

  if (wrappedPtr) {
    _wrappedObjects.insert(wrappedPtr, result);
  } else {
    _wrappedObjects.insert(obj, result);
    if (obj->parent()== NULL && _wrappedCB) {
      // tell someone who is interested that the qobject is wrapped the first time, if it has no parent
      (*_wrappedCB)(obj);
    }
  }
  return result;
}

PythonQtClassWrapper* PythonQtPrivate::createNewPythonQtClassWrapper(PythonQtClassInfo* info, PyObject* parentModule) {
  PythonQtClassWrapper* result;

  PyObject* className = PyString_FromString(info->className());

  PyObject* baseClasses = PyTuple_New(1);
  PyTuple_SET_ITEM(baseClasses, 0, (PyObject*)&PythonQtInstanceWrapper_Type);

  PyObject* typeDict = PyDict_New();
  PyObject* moduleName = PyObject_GetAttrString(parentModule, "__name__");
  PyDict_SetItemString(typeDict, "__module__", moduleName);

  PyObject* args  = Py_BuildValue("OOO", className, baseClasses, typeDict);

  // set the class info so that PythonQtClassWrapper_new can read it
  _currentClassInfoForClassWrapperCreation = info;
  // create the new type object by calling the type
  result = (PythonQtClassWrapper *)PyObject_Call((PyObject *)&PythonQtClassWrapper_Type, args, NULL);

  Py_DECREF(baseClasses);
  Py_DECREF(typeDict);
  Py_DECREF(args);
  Py_DECREF(className);

  return result;
}

PyObject*  PythonQtPrivate::createEnumValueInstance(PyObject* enumType, unsigned int enumValue)
{
  PyObject* args = Py_BuildValue("(i)", enumValue);
  PyObject* result = PyObject_Call(enumType, args, NULL);
  Py_DECREF(args);
  return result;
}

PyObject* PythonQtPrivate::createNewPythonQtEnumWrapper(const char* enumName, PyObject* parentObject) {
  PyObject* result;

  PyObject* className = PyString_FromString(enumName);

  PyObject* baseClasses = PyTuple_New(1);
  PyTuple_SET_ITEM(baseClasses, 0, (PyObject*)&PyInt_Type);

  PyObject* module = PyObject_GetAttrString(parentObject, "__module__");
  PyObject* typeDict = PyDict_New();
  PyDict_SetItemString(typeDict, "__module__", module);

  PyObject* args  = Py_BuildValue("OOO", className, baseClasses, typeDict);

  // create the new int derived type object by calling the core type
  result = PyObject_Call((PyObject *)&PyType_Type, args, NULL);

  Py_DECREF(baseClasses);
  Py_DECREF(typeDict);
  Py_DECREF(args);
  Py_DECREF(className);

  return result;
}

PythonQtSignalReceiver* PythonQt::getSignalReceiver(QObject* obj)
{
  PythonQtSignalReceiver* r = _p->_signalReceivers[obj];
  if (!r) {
    r = new PythonQtSignalReceiver(obj);
    _p->_signalReceivers.insert(obj, r);
  }
  return r;
}

bool PythonQt::addSignalHandler(QObject* obj, const char* signal, PyObject* module, const QString& objectname)
{
  bool flag = false;
  PythonQtObjectPtr callable = lookupCallable(module, objectname);
  if (callable) {
    PythonQtSignalReceiver* r = getSignalReceiver(obj);
    flag = r->addSignalHandler(signal, callable);
    if (!flag) {
      // signal not found
    }
  } else {
    // callable not found
  }
  return flag;
}

bool PythonQt::addSignalHandler(QObject* obj, const char* signal, PyObject* receiver)
{
  bool flag = false;
  PythonQtSignalReceiver* r = getSignalReceiver(obj);
  if (r) {
    flag = r->addSignalHandler(signal, receiver);
  }
  return flag;
}

bool PythonQt::removeSignalHandler(QObject* obj, const char* signal, PyObject* module, const QString& objectname)
{
  bool flag = false;
  PythonQtObjectPtr callable = lookupCallable(module, objectname);
  if (callable) {
    PythonQtSignalReceiver* r = _p->_signalReceivers[obj];
    if (r) {
      flag = r->removeSignalHandler(signal, callable);
    }
  } else {
    // callable not found
  }
  return flag;
}

bool PythonQt::removeSignalHandler(QObject* obj, const char* signal, PyObject* receiver)
{
  bool flag = false;
  PythonQtSignalReceiver* r = _p->_signalReceivers[obj];
  if (r) {
    flag = r->removeSignalHandler(signal, receiver);
  }
  return flag;
}

PythonQtObjectPtr PythonQt::lookupCallable(PyObject* module, const QString& name)
{
  PythonQtObjectPtr p = lookupObject(module, name);
  if (p) {
    if (PyCallable_Check(p)) {
      return p;
    }
  }
  PyErr_Clear();
  return NULL;
}

PythonQtObjectPtr PythonQt::lookupObject(PyObject* module, const QString& name)
{
  QStringList l = name.split('.');
  PythonQtObjectPtr p = module;
  PythonQtObjectPtr prev;
  QString s;
  QByteArray b;
  for (QStringList::ConstIterator i = l.begin(); i!=l.end() && p; ++i) {
    prev = p;
    b = (*i).toLatin1();
    if (PyDict_Check(p)) {
      p = PyDict_GetItemString(p, b.data());
    } else {
      p.setNewRef(PyObject_GetAttrString(p, b.data()));
    }
  }
  PyErr_Clear();
  return p;
}

PythonQtObjectPtr PythonQt::getMainModule() {
  //both borrowed
  PythonQtObjectPtr dict = PyImport_GetModuleDict();
  return PyDict_GetItemString(dict, "__main__");
}

PythonQtObjectPtr PythonQt::importModule(const QString& name)
{
  PythonQtObjectPtr mod;
  mod.setNewRef(PyImport_ImportModule(name.toLatin1().constData()));
  return mod;
}


QVariant PythonQt::evalCode(PyObject* object, PyObject* pycode) {
  QVariant result;
  if (pycode) {
    PyObject* dict = NULL;
    if (PyModule_Check(object)) {
      dict = PyModule_GetDict(object);
    } else if (PyDict_Check(object)) {
      dict = object;
    }
    PyObject* r = NULL;
    if (dict) {
      r = PyEval_EvalCode((PyCodeObject*)pycode, dict , dict);
    }
    if (r) {
      result = PythonQtConv::PyObjToQVariant(r);
      Py_DECREF(r);
    } else {
      handleError();
    }
  } else {
    handleError();
  }
  return result;
}

QVariant PythonQt::evalScript(PyObject* object, const QString& script, int start)
{
  QVariant result;
  PythonQtObjectPtr p;
  PyObject* dict = NULL;
  if (PyModule_Check(object)) {
    dict = PyModule_GetDict(object);
  } else if (PyDict_Check(object)) {
    dict = object;
  }
  if (dict) {
    p.setNewRef(PyRun_String(script.toLatin1().data(), start, dict, dict));
  }
  if (p) {
    result = PythonQtConv::PyObjToQVariant(p);
  } else {
    handleError();
  }
  return result;
}

void PythonQt::evalFile(PyObject* module, const QString& filename)
{
  PythonQtObjectPtr code = parseFile(filename);
  if (code) {
    evalCode(module, code);
  } else {
    handleError();
  }
}

PythonQtObjectPtr PythonQt::parseFile(const QString& filename)
{
  PythonQtObjectPtr p;
  p.setNewRef(PythonQtImport::getCodeFromPyc(filename));
  if (!p) {
    handleError();
  }
  return p;
}

PythonQtObjectPtr PythonQt::createModuleFromFile(const QString& name, const QString& filename)
{
  PythonQtObjectPtr code = parseFile(filename);
  PythonQtObjectPtr module = _p->createModule(name, code);
  return module;
}

PythonQtObjectPtr PythonQt::createModuleFromScript(const QString& name, const QString& script)
{
  PyErr_Clear();
  QString scriptCode = script;
  if (scriptCode.isEmpty()) {
    // we always need at least a linefeed
    scriptCode = "\n";
  }
  PythonQtObjectPtr pycode;
  pycode.setNewRef(Py_CompileString((char*)scriptCode.toLatin1().data(), "",  Py_file_input));
  PythonQtObjectPtr module = _p->createModule(name, pycode);
  return module;
}

PythonQtObjectPtr PythonQt::createUniqueModule()
{
  static QString pyQtStr("PythonQt_module");
  QString moduleName = pyQtStr+QString::number(_uniqueModuleCount++);
  return createModuleFromScript(moduleName);
}

void PythonQt::addObject(PyObject* object, const QString& name, QObject* qObject)
{
  if (PyModule_Check(object)) {
    PyModule_AddObject(object, name.toLatin1().data(), _p->wrapQObject(qObject));
  } else if (PyDict_Check(object)) {
    PyDict_SetItemString(object, name.toLatin1().data(), _p->wrapQObject(qObject));
  } else {
    PyObject_SetAttrString(object, name.toLatin1().data(), _p->wrapQObject(qObject));
  }
}

void PythonQt::addVariable(PyObject* object, const QString& name, const QVariant& v)
{
  if (PyModule_Check(object)) {
    PyModule_AddObject(object, name.toLatin1().data(), PythonQtConv::QVariantToPyObject(v));
  } else if (PyDict_Check(object)) {
    PyDict_SetItemString(object, name.toLatin1().data(), PythonQtConv::QVariantToPyObject(v));
  } else {
    PyObject_SetAttrString(object, name.toLatin1().data(), PythonQtConv::QVariantToPyObject(v));
  }
}

void PythonQt::removeVariable(PyObject* object, const QString& name)
{
  if (PyDict_Check(object)) {
    PyDict_DelItemString(object, name.toLatin1().data());
  } else {
    PyObject_DelAttrString(object, name.toLatin1().data());
  }
}

QVariant PythonQt::getVariable(PyObject* object, const QString& objectname)
{
  QVariant result;
  PythonQtObjectPtr obj = lookupObject(object, objectname);
  if (obj) {
    result = PythonQtConv::PyObjToQVariant(obj);
  }
  return result;
}

QStringList PythonQt::introspection(PyObject* module, const QString& objectname, PythonQt::ObjectType type)
{
  QStringList results;

  PythonQtObjectPtr object;
  if (objectname.isEmpty()) {
    object = module;
  } else {
    object = lookupObject(module, objectname);
    if (!object && type == CallOverloads) {
      PyObject* dict = lookupObject(module, "__builtins__");
      if (dict) {
        object = PyDict_GetItemString(dict, objectname.toLatin1().constData());
      }
    }
  }

  if (object) {
    if (type == CallOverloads) {
      if (PythonQtSlotFunction_Check(object)) {
        PythonQtSlotFunctionObject* o = (PythonQtSlotFunctionObject*)object.object();
        PythonQtSlotInfo* info = o->m_ml;

        while (info) {
          results << info->fullSignature();
          info = info->nextInfo();
        }
      } else if (object->ob_type == &PythonQtClassWrapper_Type) {
        PythonQtClassWrapper* o = (PythonQtClassWrapper*)object.object();
        PythonQtSlotInfo* info = o->classInfo()->constructors();

        while (info) {
          results << info->fullSignature();
          info = info->nextInfo();
        }
      } else {
        //TODO: use pydoc!
        PyObject* doc = PyObject_GetAttrString(object, "__doc__");
        if (doc) {
          results << PyString_AsString(doc);
          Py_DECREF(doc);
        }
      }
    } else {
      PyObject* keys = NULL;
      bool isDict = false;
      if (PyDict_Check(object)) {
        keys = PyDict_Keys(object);
        isDict = true;
      } else {
        keys = PyObject_Dir(object);
      }
      if (keys) {
        int count = PyList_Size(keys);
        PyObject* key;
        PyObject* value;
        QString keystr;
        for (int i = 0;i<count;i++) {
          key = PyList_GetItem(keys,i);
          if (isDict) {
            value = PyDict_GetItem(object, key);
            Py_INCREF(value);
          } else {
            value = PyObject_GetAttr(object, key);
          }
          if (!value) continue;
          keystr = PyString_AsString(key);
          static const QString underscoreStr("__tmp");
          if (!keystr.startsWith(underscoreStr)) {
            switch (type) {
            case Anything:
              results << keystr;
              break;
            case Class:
              if (value->ob_type == &PyClass_Type) {
                results << keystr;
              }
              break;
            case Variable:
              if (value->ob_type != &PyClass_Type
                && value->ob_type != &PyCFunction_Type
                && value->ob_type != &PyFunction_Type
                && value->ob_type != &PyModule_Type
                ) {
                results << keystr;
              }
              break;
            case Function:
              if (value->ob_type == &PyFunction_Type ||
                value->ob_type == &PyMethod_Type
                ) {
                results << keystr;
              }
              break;
            case Module:
              if (value->ob_type == &PyModule_Type) {
                results << keystr;
              }
              break;
            default:
              std::cerr << "PythonQt: introspection: unknown case" << ", in " << __FILE__ << ":" << __LINE__ << std::endl;
            }
          }
          Py_DECREF(value);
        }
        Py_DECREF(keys);
      }
    }
  }
  return results;
}

QVariant PythonQt::call(PyObject* object, const QString& name, const QVariantList& args)
{
  PythonQtObjectPtr callable = lookupCallable(object, name);
  if (callable) {
    return call(callable, args);
  } else {
    return QVariant();
  }
}

QVariant PythonQt::call(PyObject* callable, const QVariantList& args)
{
  QVariant r;
  PythonQtObjectPtr result;
  result.setNewRef(callAndReturnPyObject(callable, args));
  if (result) {
    r = PythonQtConv::PyObjToQVariant(result);
  } else {
    PythonQt::self()->handleError();
  }
  return r;
}

PyObject* PythonQt::callAndReturnPyObject(PyObject* callable, const QVariantList& args)
{
  PyObject* result = NULL;
  if (callable) {
    PythonQtObjectPtr pargs;
    int count = args.size();
    if (count>0) {
      pargs.setNewRef(PyTuple_New(count));
    }
    bool err = false;
    // transform QVariants to Python
    for (int i = 0; i < count; i++) {
      PyObject* arg = PythonQtConv::QVariantToPyObject(args.at(i));
      if (arg) {
        // steals reference, no unref
        PyTuple_SetItem(pargs, i,arg);
      } else {
        err = true;
        break;
      }
    }

    if (!err) {
      PyErr_Clear();
      result = PyObject_CallObject(callable, pargs);
    }
  }
  return result;
}

void PythonQt::addInstanceDecorators(QObject* o)
{
  _p->addDecorators(o, PythonQtPrivate::InstanceDecorator);
}

void PythonQt::addClassDecorators(QObject* o)
{
  _p->addDecorators(o, PythonQtPrivate::StaticDecorator | PythonQtPrivate::ConstructorDecorator | PythonQtPrivate::DestructorDecorator);
}

void PythonQt::addDecorators(QObject* o)
{
  _p->addDecorators(o, PythonQtPrivate::AllDecorators);
}

void PythonQt::registerQObjectClassNames(const QStringList& names)
{
  _p->registerQObjectClassNames(names);
}

void PythonQt::setImporter(PythonQtImportFileInterface* importInterface)
{
  _p->_importInterface = importInterface;
  PythonQtImport::init();
}

void PythonQt::setImporterIgnorePaths(const QStringList& paths)
{
  _p->_importIgnorePaths = paths;
}

const QStringList& PythonQt::getImporterIgnorePaths()
{
  return _p->_importIgnorePaths;
}

void PythonQt::addWrapperFactory(PythonQtCppWrapperFactory* factory)
{
  _p->_cppWrapperFactories.append(factory);
}

//---------------------------------------------------------------------------------------------------
PythonQtPrivate::PythonQtPrivate()
{
  _importInterface = NULL;
  _defaultImporter = new PythonQtQFileImporter;
  _noLongerWrappedCB = NULL;
  _wrappedCB = NULL;
  _currentClassInfoForClassWrapperCreation = NULL;
}

void PythonQtPrivate::setupSharedLibrarySuffixes()
{
  _sharedLibrarySuffixes.clear();
  PythonQtObjectPtr imp;
  imp.setNewRef(PyImport_ImportModule("imp"));
  int cExtensionCode = imp.getVariable("C_EXTENSION").toInt();
  QVariant result = imp.call("get_suffixes");
  foreach (QVariant entry, result.toList()) {
    QVariantList suffixEntry = entry.toList();
    if (suffixEntry.count()==3) {
      int code = suffixEntry.at(2).toInt();
      if (code == cExtensionCode) {
        _sharedLibrarySuffixes << suffixEntry.at(0).toString();
      }
    }
  }
}

PythonQtClassInfo* PythonQtPrivate::currentClassInfoForClassWrapperCreation()
{
  PythonQtClassInfo* info = _currentClassInfoForClassWrapperCreation;
  _currentClassInfoForClassWrapperCreation = NULL;
  return info;
}

void PythonQtPrivate::addDecorators(QObject* o, int decoTypes)
{
  o->setParent(this);
  int numMethods = o->metaObject()->methodCount();
  for (int i = 0; i < numMethods; i++) {
    QMetaMethod m = o->metaObject()->method(i);
    if ((m.methodType() == QMetaMethod::Method ||
      m.methodType() == QMetaMethod::Slot) && m.access() == QMetaMethod::Public) {
      if (qstrncmp(m.signature(), "new_", 4)==0) {
        if ((decoTypes & ConstructorDecorator) == 0) continue;
        const PythonQtMethodInfo* info = PythonQtMethodInfo::getCachedMethodInfo(m, NULL);
        if (info->parameters().at(0).pointerCount == 1) {
          QByteArray signature = m.signature();
          QByteArray nameOfClass = signature.mid(4, signature.indexOf('(')-4);
          PythonQtClassInfo* classInfo = lookupClassInfoAndCreateIfNotPresent(nameOfClass);
          PythonQtSlotInfo* newSlot = new PythonQtSlotInfo(NULL, m, i, o, PythonQtSlotInfo::ClassDecorator);
          classInfo->addConstructor(newSlot);
        }
      } else if (qstrncmp(m.signature(), "delete_", 7)==0) {
        if ((decoTypes & DestructorDecorator) == 0) continue;
        QByteArray signature = m.signature();
        QByteArray nameOfClass = signature.mid(7, signature.indexOf('(')-7);
        PythonQtClassInfo* classInfo = lookupClassInfoAndCreateIfNotPresent(nameOfClass);
        PythonQtSlotInfo* newSlot = new PythonQtSlotInfo(NULL, m, i, o, PythonQtSlotInfo::ClassDecorator);
        classInfo->setDestructor(newSlot);
      } else if (qstrncmp(m.signature(), "static_", 7)==0) {
        if ((decoTypes & StaticDecorator) == 0) continue;
        QByteArray signature = m.signature();
        QByteArray nameOfClass = signature.mid(signature.indexOf('_')+1);
        nameOfClass = nameOfClass.mid(0, nameOfClass.indexOf('_'));
        PythonQtClassInfo* classInfo = lookupClassInfoAndCreateIfNotPresent(nameOfClass);
        PythonQtSlotInfo* newSlot = new PythonQtSlotInfo(NULL, m, i, o, PythonQtSlotInfo::ClassDecorator);
        classInfo->addDecoratorSlot(newSlot);
      } else {
        if ((decoTypes & InstanceDecorator) == 0) continue;
        const PythonQtMethodInfo* info = PythonQtMethodInfo::getCachedMethodInfo(m, NULL);
        if (info->parameters().count()>1) {
          PythonQtMethodInfo::ParameterInfo p = info->parameters().at(1);
          if (p.pointerCount==1) {
            PythonQtClassInfo* classInfo = lookupClassInfoAndCreateIfNotPresent(p.name);
            PythonQtSlotInfo* newSlot = new PythonQtSlotInfo(NULL, m, i, o, PythonQtSlotInfo::InstanceDecorator);
            classInfo->addDecoratorSlot(newSlot);
          }
        }
      }
    }
  }
}

void PythonQtPrivate::registerQObjectClassNames(const QStringList& names)
{
  foreach(QString name, names) {
    _knownQObjectClassNames.insert(name.toLatin1(), true);
  }
}

void PythonQtPrivate::removeSignalEmitter(QObject* obj)
{
  _signalReceivers.remove(obj);
}

bool PythonQt::handleError()
{
  bool flag = false;
  if (PyErr_Occurred()) {

    // currently we just print the error and the stderr handler parses the errors
    PyErr_Print();

    /*
    // EXTRA: the format of the ptype and ptraceback is not really documented, so I use PyErr_Print() above
    PyObject *ptype;
    PyObject *pvalue;
    PyObject *ptraceback;
    PyErr_Fetch( &ptype, &pvalue, &ptraceback);

      Py_XDECREF(ptype);
      Py_XDECREF(pvalue);
      Py_XDECREF(ptraceback);
    */
    PyErr_Clear();
    flag = true;
  }
  return flag;
}

void PythonQt::addSysPath(const QString& path)
{
  PythonQtObjectPtr sys;
  sys.setNewRef(PyImport_ImportModule("sys"));
  PythonQtObjectPtr obj = lookupObject(sys, "path");
  PyList_Insert(obj, 0, PythonQtConv::QStringToPyObject(path));
}

void PythonQt::overwriteSysPath(const QStringList& paths)
{
  PythonQtObjectPtr sys;
  sys.setNewRef(PyImport_ImportModule("sys"));
  PyModule_AddObject(sys, "path", PythonQtConv::QStringListToPyList(paths));
}

void PythonQt::setModuleImportPath(PyObject* module, const QStringList& paths)
{
  PyModule_AddObject(module, "__path__", PythonQtConv::QStringListToPyList(paths));
}

void PythonQt::stdOutRedirectCB(const QString& str)
{
  emit PythonQt::self()->pythonStdOut(str);
}

void PythonQt::stdErrRedirectCB(const QString& str)
{
  emit PythonQt::self()->pythonStdErr(str);
}

void PythonQt::setQObjectWrappedCallback(PythonQtQObjectWrappedCB* cb)
{
  _p->_wrappedCB = cb;
}

void PythonQt::setQObjectNoLongerWrappedCallback(PythonQtQObjectNoLongerWrappedCB* cb)
{
  _p->_noLongerWrappedCB = cb;
}



static PyMethodDef PythonQtMethods[] = {
  {NULL, NULL, 0, NULL}
};

void PythonQt::initPythonQtModule(bool redirectStdOut, const QByteArray& pythonQtModuleName)
{
  QByteArray name = "PythonQt";
  if (!pythonQtModuleName.isEmpty()) {
    name = pythonQtModuleName;
  }
  _p->_pythonQtModule = Py_InitModule(name.constData(), PythonQtMethods);
  _p->_pythonQtModuleName = name;
  
  if (redirectStdOut) {
    PythonQtObjectPtr sys;
    PythonQtObjectPtr out;
    PythonQtObjectPtr err;
    sys.setNewRef(PyImport_ImportModule("sys"));
    // create a redirection object for stdout and stderr
    out = PythonQtStdOutRedirectType.tp_new(&PythonQtStdOutRedirectType,NULL, NULL);
    ((PythonQtStdOutRedirect*)out.object())->_cb = stdOutRedirectCB;
    err = PythonQtStdOutRedirectType.tp_new(&PythonQtStdOutRedirectType,NULL, NULL);
    ((PythonQtStdOutRedirect*)err.object())->_cb = stdErrRedirectCB;
    // replace the built in file objects with our own objects
    PyModule_AddObject(sys, "stdout", out);
    PyModule_AddObject(sys, "stderr", err);
  }
}

void PythonQt::registerCPPClass(const char* typeName, const char* parentTypeName, const char* package, PythonQtQObjectCreatorFunctionCB* wrapperCreator,  PythonQtShellSetInstanceWrapperCB* shell)
{
  _p->registerCPPClass(typeName, parentTypeName, package, wrapperCreator, shell);
}


PythonQtClassInfo* PythonQtPrivate::lookupClassInfoAndCreateIfNotPresent(const char* typeName)
{
  PythonQtClassInfo* info = _knownClassInfos.value(typeName);
  if (!info) {
    info = new PythonQtClassInfo();
    info->setupCPPObject(typeName);
    _knownClassInfos.insert(typeName, info);
  }
  return info;
}

void PythonQt::addPolymorphicHandler(const char* typeName, PythonQtPolymorphicHandlerCB* cb)
{
  _p->addPolymorphicHandler(typeName, cb);
}

void PythonQtPrivate::addPolymorphicHandler(const char* typeName, PythonQtPolymorphicHandlerCB* cb)
{
  PythonQtClassInfo* info = lookupClassInfoAndCreateIfNotPresent(typeName);
  info->addPolymorphicHandler(cb);
}

bool PythonQt::addParentClass(const char* typeName, const char* parentTypeName, int upcastingOffset)
{
  return _p->addParentClass(typeName, parentTypeName, upcastingOffset);
}

bool PythonQtPrivate::addParentClass(const char* typeName, const char* parentTypeName, int upcastingOffset)
{
  PythonQtClassInfo* info = _knownClassInfos.value(typeName);
  if (info) {
    PythonQtClassInfo* parentInfo = lookupClassInfoAndCreateIfNotPresent(parentTypeName);
    info->addParentClass(PythonQtClassInfo::ParentClassInfo(parentInfo, upcastingOffset));
    return true;
  } else {
    return false;
  }
}

void PythonQtPrivate::registerCPPClass(const char* typeName, const char* parentTypeName, const char* package, PythonQtQObjectCreatorFunctionCB* wrapperCreator,  PythonQtShellSetInstanceWrapperCB* shell, PyObject* module, int typeSlots)
{
  PythonQtClassInfo* info = lookupClassInfoAndCreateIfNotPresent(typeName);
  if (!info->pythonQtClassWrapper()) {
    info->setTypeSlots(typeSlots);
    info->setupCPPObject(typeName);
    createPythonQtClassWrapper(info, package, module);
  }
  if (parentTypeName && strcmp(parentTypeName,"")!=0) {
    addParentClass(typeName, parentTypeName, 0);
  }
  if (wrapperCreator) {
    info->setDecoratorProvider(wrapperCreator);
  }
  if (shell) {
    info->setShellSetInstanceWrapperCB(shell);
  }
}

PyObject* PythonQtPrivate::packageByName(const char* name)
{
  if (name==NULL || name[0]==0) {
    name = "private";
  }
  PyObject* v = _packages.value(name);
  if (!v) {
    v = PyImport_AddModule((_pythonQtModuleName + "." + name).constData());
    _packages.insert(name, v);
    // AddObject steals the reference, so increment it!
    Py_INCREF(v);
    PyModule_AddObject(_pythonQtModule, name, v);
  }
  return v;
}

void PythonQtPrivate::handleVirtualOverloadReturnError(const char* signature, const PythonQtMethodInfo* methodInfo, PyObject* result)
{
  QString error = "Return value '" + PythonQtConv::PyObjGetString(result) + "' can not be converted to expected C++ type '" + methodInfo->parameters().at(0).name + "' as return value of virtual method " + signature;
  PyErr_SetString(PyExc_AttributeError, error.toLatin1().data());
  PythonQt::self()->handleError();
}

PyObject* PythonQt::helpCalled(PythonQtClassInfo* info)
{
  if (_p->_initFlags & ExternalHelp) {
    emit pythonHelpRequest(QByteArray(info->className()));
    return Py_BuildValue("");
  } else {
    return PyString_FromString(info->help().toLatin1().data());
  }
}

void PythonQtPrivate::removeWrapperPointer(void* obj)
{
  _wrappedObjects.remove(obj);
}

void PythonQtPrivate::addWrapperPointer(void* obj, PythonQtInstanceWrapper* wrapper)
{
  _wrappedObjects.insert(obj, wrapper);
}

PythonQtInstanceWrapper* PythonQtPrivate::findWrapperAndRemoveUnused(void* obj)
{
  PythonQtInstanceWrapper* wrap = _wrappedObjects.value(obj);
  if (wrap && !wrap->_wrappedPtr && wrap->_obj == NULL) {
    // this is a wrapper whose QObject was already removed due to destruction
    // so the obj pointer has to be a new QObject with the same address...
    // we remove the old one and set the copy to NULL
    wrap->_objPointerCopy = NULL;
    removeWrapperPointer(obj);
    wrap = NULL;
  }
  return wrap;
}

PythonQtObjectPtr PythonQtPrivate::createModule(const QString& name, PyObject* pycode)
{
  PythonQtObjectPtr result;
  if (pycode) {
    result.setNewRef(PyImport_ExecCodeModule((char*)name.toLatin1().data(), pycode));
  } else {
    PythonQt::self()->handleError();
  }
  return result;
}
