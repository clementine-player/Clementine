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
// \file    PythonQtSignalReceiver.cpp
// \author  Florian Link
// \author  Last changed by $Author: florian $
// \date    2006-05
*/
//----------------------------------------------------------------------------------

#include "PythonQtSignalReceiver.h"
#include "PythonQtClassInfo.h"
#include "PythonQtMethodInfo.h"
#include "PythonQtConversion.h"
#include <QMetaObject>
#include <QMetaMethod>
#include "funcobject.h"

void PythonQtSignalTarget::call(void **arguments) const {
  PyObject* result = call(_callable, methodInfo(), arguments);
  if (result) {
    Py_DECREF(result);
  }
}

PyObject* PythonQtSignalTarget::call(PyObject* callable, const PythonQtMethodInfo* methodInfos, void **arguments, bool skipFirstArgumentOfMethodInfo)
{
  // Note: we check if the callable is a PyFunctionObject and has a fixed number of arguments
  // if that is the case, we only pass these arguments to python and skip the additional arguments from the signal
  
  int numPythonArgs = -1;
  if (PyFunction_Check(callable)) {
    PyObject* o = callable;
    PyFunctionObject* func = (PyFunctionObject*)o;
    PyCodeObject* code = (PyCodeObject*)func->func_code;
    if (!(code->co_flags & 0x04)) {
      numPythonArgs = code->co_argcount;
    } else {
      // variable numbers of arguments allowed
    }
  } else if (PyMethod_Check(callable)) {
    PyObject* o = callable;
    PyMethodObject* method = (PyMethodObject*)o;
    if (PyFunction_Check(method->im_func)) {
      PyFunctionObject* func = (PyFunctionObject*)method->im_func;
      PyCodeObject* code = (PyCodeObject*)func->func_code;
      if (!(code->co_flags & 0x04)) {
        numPythonArgs = code->co_argcount - 1; // we subtract one because the first is "self"
      } else {
        // variable numbers of arguments allowed
      }
    }
  }

  const PythonQtMethodInfo* m = methodInfos;
  // parameterCount includes return value:
  int count = m->parameterCount();
  if (numPythonArgs!=-1) {
    if (count>numPythonArgs+1) {
      // take less arguments
      count = numPythonArgs+1;
    }
  }

  PyObject* pargs = NULL;
  if (count>1) {
    pargs = PyTuple_New(count-1);
  }
  bool err = false;
  // transform Qt values to Python
  const QList<PythonQtMethodInfo::ParameterInfo>& params = m->parameters();
  for (int i = 1; i < count; i++) {
    const PythonQtMethodInfo::ParameterInfo& param = params.at(i);
    PyObject* arg = PythonQtConv::ConvertQtValueToPython(param, arguments[i]);
    if (arg) {
      // steals reference, no unref
      PyTuple_SetItem(pargs, i-1,arg);
    } else {
      err = true;
      break;
    }
  }

  PyObject* result = NULL;
  if (!err) {
    PyErr_Clear();
    result = PyObject_CallObject(callable, pargs);
    if (result) {
      // ok
    } else {
      PythonQt::self()->handleError();
    }
  }
  if (pargs) {
    // free the arguments again
    Py_DECREF(pargs);
  }

  return result;
}

//------------------------------------------------------------------------------

PythonQtSignalReceiver::PythonQtSignalReceiver(QObject* obj):PythonQtSignalReceiverBase(obj)
{
  _obj = obj;
  
  // fetch the class info for object, since we will need to for correct enum resolution in
  // signals
  _objClassInfo = PythonQt::priv()->getClassInfo(obj->metaObject());
  if (!_objClassInfo || !_objClassInfo->isQObject()) {
    PythonQt::self()->registerClass(obj->metaObject());
    _objClassInfo = PythonQt::priv()->getClassInfo(obj->metaObject());
  }
  // force decorator/enum creation
  _objClassInfo->decorator();

  _slotCount = staticMetaObject.methodOffset();
}

PythonQtSignalReceiver::~PythonQtSignalReceiver()
{
  PythonQt::priv()->removeSignalEmitter(_obj);
}


bool PythonQtSignalReceiver::addSignalHandler(const char* signal, PyObject* callable)
{
  bool flag = false;
  int sigId = getSignalIndex(signal);
  if (sigId>=0) {
    // create PythonQtMethodInfo from signal
    QMetaMethod meta = _obj->metaObject()->method(sigId);
    const PythonQtMethodInfo* signalInfo = PythonQtMethodInfo::getCachedMethodInfo(meta, _objClassInfo);
    PythonQtSignalTarget t(sigId, signalInfo, _slotCount, callable);
    _targets.append(t);
    // now connect to ourselves with the new slot id
    QMetaObject::connect(_obj, sigId, this, _slotCount, Qt::AutoConnection, 0);

    _slotCount++;
    flag = true;
  }
  return flag;
}

bool PythonQtSignalReceiver::removeSignalHandler(const char* signal, PyObject* callable)
{
  bool found = false;
  int sigId = getSignalIndex(signal);
  if (sigId>=0) {
    QMutableListIterator<PythonQtSignalTarget> i(_targets);
    while (i.hasNext()) {
      if (i.next().isSame(sigId, callable)) {
        i.remove();
        found = true;
        break;
      }
    }
  }
  return found;
}

void PythonQtSignalReceiver::removeSignalHandlers()
{
  _targets.clear();
}

int PythonQtSignalReceiver::getSignalIndex(const char* signal)
{
  int sigId = _obj->metaObject()->indexOfSignal(signal+1);
  if (sigId<0) {
    QByteArray tmpSig = QMetaObject::normalizedSignature(signal+1);
    sigId = _obj->metaObject()->indexOfSignal(tmpSig);
  }
  return sigId;
}

int PythonQtSignalReceiver::qt_metacall(QMetaObject::Call c, int id, void **arguments)
{
//  mlabDebugConst("PythonQt", "PythonQtSignalReceiver invoke " << _obj->className() << " " << _obj->name() << " " << id);
  if (c != QMetaObject::InvokeMetaMethod) {
    QObject::qt_metacall(c, id, arguments);
  }

  bool found = false;
  foreach(const PythonQtSignalTarget& t, _targets) {
    if (t.slotId() == id) {
      found = true;
      t.call(arguments);
      break;
    }
  }
  return 0;
}

