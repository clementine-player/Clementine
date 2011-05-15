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
// \file    PythonQtSlot.cpp
// \author  Florian Link
// \author  Last changed by $Author: florian $
// \date    2006-05
*/
//----------------------------------------------------------------------------------

#include "PythonQt.h"
#include "PythonQtSlot.h"
#include "PythonQtInstanceWrapper.h"
#include "PythonQtClassInfo.h"
#include "PythonQtMisc.h"
#include "PythonQtConversion.h"
#include <iostream>

#define PYTHONQT_MAX_ARGS 32


bool PythonQtCallSlot(PythonQtClassInfo* classInfo, QObject* objectToCall, PyObject* args, bool strict, PythonQtSlotInfo* info, void* firstArgument, PyObject** pythonReturnValue, void** directReturnValuePointer)
{
  static unsigned int recursiveEntry = 0;
  
  if (directReturnValuePointer) {
    *directReturnValuePointer = NULL;
  }
  // store the current storage position, so that we can get back to this state after a slot is called
  // (do this locally, so that we have all positions on the stack
  PythonQtValueStoragePosition globalValueStoragePos;
  PythonQtValueStoragePosition globalPtrStoragePos;
  PythonQtValueStoragePosition globalVariantStoragePos;
  PythonQtConv::global_valueStorage.getPos(globalValueStoragePos);
  PythonQtConv::global_ptrStorage.getPos(globalPtrStoragePos);
  PythonQtConv::global_variantStorage.getPos(globalVariantStoragePos);
  
  recursiveEntry++;
  
  // the arguments that are passed to qt_metacall
  void* argList[PYTHONQT_MAX_ARGS];
  PyObject* result = NULL;
  int argc = info->parameterCount();
  const QList<PythonQtSlotInfo::ParameterInfo>& params = info->parameters();
  
  const PythonQtSlotInfo::ParameterInfo& returnValueParam = params.at(0);
  // set return argument to NULL
  argList[0] = NULL;
  
  bool ok = true;
  bool skipFirst = false;
  if (info->isInstanceDecorator()) {
    skipFirst = true;

    // for decorators on CPP objects, we take the cpp ptr, for QObjects we take the QObject pointer
    void* arg1 = firstArgument;
    if (!arg1) {
      arg1 = objectToCall;
    }
    if (arg1) {
      // upcast to correct parent class
      arg1 = ((char*)arg1)+info->upcastingOffset();
    }

    argList[1] = &arg1;
    if (ok) {
      for (int i = 2; i<argc && ok; i++) {
        const PythonQtSlotInfo::ParameterInfo& param = params.at(i);
        argList[i] = PythonQtConv::ConvertPythonToQt(param, PyTuple_GET_ITEM(args, i-2), strict, classInfo);
        if (argList[i]==NULL) {
          ok = false;
          break;
        }
      }
    }
  } else {
    for (int i = 1; i<argc && ok; i++) {
      const PythonQtSlotInfo::ParameterInfo& param = params.at(i);
      argList[i] = PythonQtConv::ConvertPythonToQt(param, PyTuple_GET_ITEM(args, i-1), strict, classInfo);
      if (argList[i]==NULL) {
        ok = false;
        break;
      }
    }
  }
  
  if (ok) {
    // parameters are ok, now create the qt return value which is assigned to by metacall
    if (returnValueParam.typeId != QMetaType::Void) {
      // create empty default value for the return value
      if (!directReturnValuePointer) {
        // create empty default value for the return value
        argList[0] = PythonQtConv::CreateQtReturnValue(returnValueParam);
        if (argList[0]==NULL) {
          // return value could not be created, maybe we have a registered class with a default constructor, so that we can construct the pythonqt wrapper object and
          // pass its internal pointer
          PythonQtClassInfo* info = PythonQt::priv()->getClassInfo(returnValueParam.name);
          if (info && info->pythonQtClassWrapper()) {
            PyObject* emptyTuple = PyTuple_New(0);
            // 1) default construct an empty object as python object (owned by PythonQt), by calling the meta class with empty arguments
            result = PyObject_Call((PyObject*)info->pythonQtClassWrapper(), emptyTuple, NULL);
            if (result) {
              argList[0] = ((PythonQtInstanceWrapper*)result)->_wrappedPtr;
            }
            Py_DECREF(emptyTuple);            
          } 
        }
      } else {
        // we can use our pointer directly!
        argList[0] = directReturnValuePointer;
      }
    }

    // invoke the slot via metacall
    (info->decorator()?info->decorator():objectToCall)->qt_metacall(QMetaObject::InvokeMetaMethod, info->slotIndex(), argList);

    // handle the return value (which in most cases still needs to be converted to a Python object)
    if (argList[0] || returnValueParam.typeId == QMetaType::Void) {
      if (directReturnValuePointer) {
        result = NULL;
      } else {
        // the resulting object maybe present already, because we created it above at 1)...
        if (!result) {
          result = PythonQtConv::ConvertQtValueToPython(returnValueParam, argList[0]);
        }
      }
    } else {
      QString e = QString("Called ") + info->fullSignature() + ", return type '" + returnValueParam.name + "' is ignored because it is unknown to PythonQt. Probably you should register it using qRegisterMetaType() or add a default constructor decorator to the class.";
      PyErr_SetString(PyExc_ValueError, e.toLatin1().data());
      result = NULL;
    }
  }
  recursiveEntry--;
  
  // reset the parameter storage position to the stored pos to "pop" the parameter stack
  PythonQtConv::global_valueStorage.setPos(globalValueStoragePos);
  PythonQtConv::global_ptrStorage.setPos(globalPtrStoragePos);
  PythonQtConv::global_variantStorage.setPos(globalVariantStoragePos);
  
  *pythonReturnValue = result;
  // NOTE: it is important to only return here, otherwise the stack will not be popped!!!
  return result || (directReturnValuePointer && *directReturnValuePointer);
}

//-----------------------------------------------------------------------------------

static PythonQtSlotFunctionObject *pythonqtslot_free_list = NULL;

PyObject *PythonQtSlotFunction_Call(PyObject *func, PyObject *args, PyObject *kw)
{
  PythonQtSlotFunctionObject* f = (PythonQtSlotFunctionObject*)func;
  PythonQtSlotInfo*    info = f->m_ml;
  if (PyObject_TypeCheck(f->m_self, &PythonQtInstanceWrapper_Type)) {
    PythonQtInstanceWrapper* self = (PythonQtInstanceWrapper*) f->m_self;
    if (!info->isClassDecorator() && (self->_obj==NULL && self->_wrappedPtr==NULL)) {
      QString error = QString("Trying to call '") + f->m_ml->slotName() + "' on a destroyed " + self->classInfo()->className() + " object";
      PyErr_SetString(PyExc_ValueError, error.toLatin1().data());
      return NULL;
    } else {
      return PythonQtSlotFunction_CallImpl(self->classInfo(), self->_obj, info, args, kw, self->_wrappedPtr);
    }
  } else if (f->m_self->ob_type == &PythonQtClassWrapper_Type) {
    PythonQtClassWrapper* type = (PythonQtClassWrapper*) f->m_self;
    if (info->isClassDecorator()) {
      return PythonQtSlotFunction_CallImpl(type->classInfo(), NULL, info, args, kw);
    } else {
      // otherwise, it is an unbound call and we have an instanceDecorator or normal slot...
      Py_ssize_t argc = PyTuple_Size(args);
      if (argc>0) {
        PyObject* firstArg = PyTuple_GET_ITEM(args, 0);
        if (PyObject_TypeCheck(firstArg, (PyTypeObject*)&PythonQtInstanceWrapper_Type)
          && ((PythonQtInstanceWrapper*)firstArg)->classInfo()->inherits(type->classInfo())) {
          PythonQtInstanceWrapper* self = (PythonQtInstanceWrapper*)firstArg;
          if (!info->isClassDecorator() && (self->_obj==NULL && self->_wrappedPtr==NULL)) {
            QString error = QString("Trying to call '") + f->m_ml->slotName() + "' on a destroyed " + self->classInfo()->className() + " object";
            PyErr_SetString(PyExc_ValueError, error.toLatin1().data());
            return NULL;
          }
          // strip the first argument...
          PyObject* newargs = PyTuple_GetSlice(args, 1, argc);
          PyObject* result = PythonQtSlotFunction_CallImpl(self->classInfo(), self->_obj, info, newargs, kw, self->_wrappedPtr);
          Py_DECREF(newargs);
          return result;
        } else {
          // first arg is not of correct type!
          QString error = "slot " + info->fullSignature() + " requires " + type->classInfo()->className() + " instance as first argument, got " + firstArg->ob_type->tp_name;
          PyErr_SetString(PyExc_ValueError, error.toLatin1().data());
          return NULL;
        }
      } else {
        // wrong number of args
        QString error = "slot " + info->fullSignature() + " requires " + type->classInfo()->className() + " instance as first argument.";
        PyErr_SetString(PyExc_ValueError, error.toLatin1().data());
        return NULL;
      }
    }
  }
  return NULL;
}

PyObject *PythonQtSlotFunction_CallImpl(PythonQtClassInfo* classInfo, QObject* objectToCall, PythonQtSlotInfo* info, PyObject *args, PyObject * /*kw*/, void* firstArg, void** directReturnValuePointer)
{
  int argc = args?PyTuple_Size(args):0;

#ifdef PYTHONQT_DEBUG
  std::cout << "called " << info->metaMethod()->typeName() << " " << info->metaMethod()->signature() << std::endl;
#endif

  PyObject* r = NULL;
  bool ok = false;
  if (directReturnValuePointer) {
    *directReturnValuePointer = NULL;
  }
  if (info->nextInfo()) {
    // overloaded slot call, try on all slots with strict conversion first
    bool strict = true;
    PythonQtSlotInfo* i = info;
    while (i) {
      bool skipFirst = i->isInstanceDecorator();
      if (i->parameterCount()-1-(skipFirst?1:0) == argc) {
        PyErr_Clear();
        ok = PythonQtCallSlot(classInfo, objectToCall, args, strict, i, firstArg, &r, directReturnValuePointer);
        if (PyErr_Occurred() || ok) break;
      }
      i = i->nextInfo();
      if (!i) {
        if (strict) {
          // one more run without being strict
          strict = false;
          i = info;
        }
      }
    }
    if (!ok && !PyErr_Occurred()) {
      QString e = QString("Could not find matching overload for given arguments:\n" + PythonQtConv::PyObjGetString(args) + "\n The following slots are available:\n");
      PythonQtSlotInfo* i = info;
      while (i) {
        e += QString(i->fullSignature()) + "\n";
        i = i->nextInfo();
      }
      PyErr_SetString(PyExc_ValueError, e.toLatin1().data());
    }
  } else {
    // simple (non-overloaded) slot call
    bool skipFirst = info->isInstanceDecorator();
    if (info->parameterCount()-1-(skipFirst?1:0) == argc) {
      PyErr_Clear();
      ok = PythonQtCallSlot(classInfo, objectToCall, args, false, info, firstArg, &r, directReturnValuePointer);
      if (!ok && !PyErr_Occurred()) {
        QString e = QString("Called ") + info->fullSignature() + " with wrong arguments: " + PythonQtConv::PyObjGetString(args);
        PyErr_SetString(PyExc_ValueError, e.toLatin1().data());
      }
    } else {
      QString e = QString("Called ") + info->fullSignature() + " with wrong number of arguments: " + PythonQtConv::PyObjGetString(args);
      PyErr_SetString(PyExc_ValueError, e.toLatin1().data());
    }
  }
  
  return r;
}

PyObject *
PythonQtSlotFunction_New(PythonQtSlotInfo *ml, PyObject *self, PyObject *module)
{
  PythonQtSlotFunctionObject *op;
  op = pythonqtslot_free_list;
  if (op != NULL) {
    pythonqtslot_free_list = (PythonQtSlotFunctionObject *)(op->m_self);
    PyObject_INIT(op, &PythonQtSlotFunction_Type);
  }
  else {
    op = PyObject_GC_New(PythonQtSlotFunctionObject, &PythonQtSlotFunction_Type);
    if (op == NULL)
      return NULL;
  }
  op->m_ml = ml;
  Py_XINCREF(self);
  op->m_self = self;
  Py_XINCREF(module);
  op->m_module = module;
  PyObject_GC_Track(op);
  return (PyObject *)op;
}

PythonQtSlotInfo*
PythonQtSlotFunction_GetSlotInfo(PyObject *op)
{
  if (!PythonQtSlotFunction_Check(op)) {
    PyErr_BadInternalCall();
    return NULL;
  }
  return ((PythonQtSlotFunctionObject *)op) -> m_ml;
}

PyObject *
PythonQtSlotFunction_GetSelf(PyObject *op)
{
  if (!PythonQtSlotFunction_Check(op)) {
    PyErr_BadInternalCall();
    return NULL;
  }
  return ((PythonQtSlotFunctionObject *)op) -> m_self;
}

/* Methods (the standard built-in methods, that is) */

static void
meth_dealloc(PythonQtSlotFunctionObject *m)
{
  PyObject_GC_UnTrack(m);
  Py_XDECREF(m->m_self);
  Py_XDECREF(m->m_module);
  m->m_self = (PyObject *)pythonqtslot_free_list;
  pythonqtslot_free_list = m;
}

static PyObject *
meth_get__doc__(PythonQtSlotFunctionObject * /*m*/, void * /*closure*/)
{
  Py_INCREF(Py_None);
  return Py_None;
}

static PyObject *
meth_get__name__(PythonQtSlotFunctionObject *m, void * /*closure*/)
{
  return PyString_FromString(m->m_ml->metaMethod()->signature());
}

static int
meth_traverse(PythonQtSlotFunctionObject *m, visitproc visit, void *arg)
{
  int err;
  if (m->m_self != NULL) {
    err = visit(m->m_self, arg);
    if (err)
      return err;
  }
  if (m->m_module != NULL) {
    err = visit(m->m_module, arg);
    if (err)
      return err;
  }
  return 0;
}

static PyObject *
meth_get__self__(PythonQtSlotFunctionObject *m, void * /*closure*/)
{
  PyObject *self;
  if (PyEval_GetRestricted()) {
    PyErr_SetString(PyExc_RuntimeError,
      "method.__self__ not accessible in restricted mode");
    return NULL;
  }
  self = m->m_self;
  if (self == NULL)
    self = Py_None;
  Py_INCREF(self);
  return self;
}

static PyGetSetDef meth_getsets [] = {
  {"__doc__",  (getter)meth_get__doc__,  NULL, NULL},
  {"__name__", (getter)meth_get__name__, NULL, NULL},
  {"__self__", (getter)meth_get__self__, NULL, NULL},
  {NULL, NULL, NULL,NULL},
};

#if PY_MAJOR_VERSION == 2 && PY_MINOR_VERSION < 6
#define PY_WRITE_RESTRICTED WRITE_RESTRICTED
#endif

#define OFF(x) offsetof(PythonQtSlotFunctionObject, x)

static PyMemberDef meth_members[] = {
  {"__module__",    T_OBJECT,     OFF(m_module), PY_WRITE_RESTRICTED},
  {NULL}
};

static PyObject *
meth_repr(PythonQtSlotFunctionObject *f)
{
  if (f->m_self->ob_type == &PythonQtClassWrapper_Type) {
    PythonQtClassWrapper* self = (PythonQtClassWrapper*) f->m_self;
    return PyString_FromFormat("<unbound qt slot %s of %s type>",
      f->m_ml->slotName().data(),
      self->classInfo()->className());
  } else {
    return PyString_FromFormat("<qt slot %s of %s instance at %p>",
      f->m_ml->slotName().data(),
      f->m_self->ob_type->tp_name,
      f->m_self);
  }
}

static int
meth_compare(PythonQtSlotFunctionObject *a, PythonQtSlotFunctionObject *b)
{
  if (a->m_self != b->m_self)
    return (a->m_self < b->m_self) ? -1 : 1;
  if (a->m_ml == b->m_ml)
    return 0;
  if (strcmp(a->m_ml->metaMethod()->signature(), b->m_ml->metaMethod()->signature()) < 0)
    return -1;
  else
    return 1;
}

static long
meth_hash(PythonQtSlotFunctionObject *a)
{
  long x,y;
  if (a->m_self == NULL)
    x = 0;
  else {
    x = PyObject_Hash(a->m_self);
    if (x == -1)
      return -1;
  }
  y = _Py_HashPointer((void*)(a->m_ml));
  if (y == -1)
    return -1;
  x ^= y;
  if (x == -1)
    x = -2;
  return x;
}


PyTypeObject PythonQtSlotFunction_Type = {
  PyObject_HEAD_INIT(&PyType_Type)
    0,
    "builtin_qt_slot",
    sizeof(PythonQtSlotFunctionObject),
    0,
    (destructor)meth_dealloc,     /* tp_dealloc */
    0,          /* tp_print */
    0,          /* tp_getattr */
    0,          /* tp_setattr */
    (cmpfunc)meth_compare,      /* tp_compare */
    (reprfunc)meth_repr,      /* tp_repr */
    0,          /* tp_as_number */
    0,          /* tp_as_sequence */
    0,          /* tp_as_mapping */
    (hashfunc)meth_hash,      /* tp_hash */
    PythonQtSlotFunction_Call,      /* tp_call */
    0,          /* tp_str */
    PyObject_GenericGetAttr,    /* tp_getattro */
    0,          /* tp_setattro */
    0,          /* tp_as_buffer */
    Py_TPFLAGS_DEFAULT | Py_TPFLAGS_HAVE_GC,/* tp_flags */
    0,          /* tp_doc */
    (traverseproc)meth_traverse,    /* tp_traverse */
    0,          /* tp_clear */
    0,          /* tp_richcompare */
    0,          /* tp_weaklistoffset */
    0,          /* tp_iter */
    0,          /* tp_iternext */
    0,          /* tp_methods */
    meth_members,       /* tp_members */
    meth_getsets,       /* tp_getset */
    0,          /* tp_base */
    0,          /* tp_dict */
};

/* Clear out the free list */

void
PythonQtSlotFunction_Fini(void)
{
  while (pythonqtslot_free_list) {
    PythonQtSlotFunctionObject *v = pythonqtslot_free_list;
    pythonqtslot_free_list = (PythonQtSlotFunctionObject *)(v->m_self);
    PyObject_GC_Del(v);
  }
}

