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
// \file    PythonQtClassWrapper.cpp
// \author  Florian Link
// \author  Last changed by $Author: florian $
// \date    2006-05
*/
//----------------------------------------------------------------------------------

#include "PythonQtClassWrapper.h"
#include <QObject>

#include "PythonQt.h"
#include "PythonQtSlot.h"
#include "PythonQtClassInfo.h"
#include "PythonQtConversion.h"
#include "PythonQtInstanceWrapper.h"

static PyObject* PythonQtInstanceWrapper_invert(PythonQtInstanceWrapper* wrapper)
{
  PyObject* result = NULL;
  static QByteArray memberName = "__invert__";
  PythonQtMemberInfo opSlot = wrapper->classInfo()->member(memberName);
  if (opSlot._type == PythonQtMemberInfo::Slot) {
    result = PythonQtSlotFunction_CallImpl(wrapper->classInfo(), wrapper->_obj, opSlot._slot, NULL, NULL, wrapper->_wrappedPtr);
  }
  return result;
}

static int PythonQtInstanceWrapper_nonzero(PythonQtInstanceWrapper* wrapper)
{
  int result = (wrapper->_wrappedPtr == NULL && wrapper->_obj == NULL)?0:1;
  if (result) {
    static QByteArray memberName = "__nonzero__";
    PythonQtMemberInfo opSlot = wrapper->classInfo()->member(memberName);
    if (opSlot._type == PythonQtMemberInfo::Slot) {
      PyObject* resultObj = PythonQtSlotFunction_CallImpl(wrapper->classInfo(), wrapper->_obj, opSlot._slot, NULL, NULL, wrapper->_wrappedPtr);
      if (resultObj == Py_False) {
        result = 0;
      }
      Py_XDECREF(resultObj);
    }
  }
  return result;
}


static PyObject* PythonQtInstanceWrapper_binaryfunc(PyObject* self, PyObject* other, const QByteArray& opName, const QByteArray& fallbackOpName = QByteArray())
{
  // since we disabled type checking, we can receive any object as self, but we currently only support
  // different objects on the right. Otherwise we would need to generate __radd__ etc. methods.
  if (!PyObject_TypeCheck(self, &PythonQtInstanceWrapper_Type)) {
    QString error = "Unsupported operation " + opName + "(" + self->ob_type->tp_name + ", " +  other->ob_type->tp_name + ")";
    PyErr_SetString(PyExc_ArithmeticError, error.toLatin1().data());
    return NULL;
  }
  PythonQtInstanceWrapper* wrapper = (PythonQtInstanceWrapper*)self;
  PyObject* result = NULL;
  PythonQtMemberInfo opSlot = wrapper->classInfo()->member(opName);
  if (opSlot._type == PythonQtMemberInfo::Slot) {
    // TODO get rid of tuple
    PyObject* args = PyTuple_New(1);
    Py_INCREF(other);
    PyTuple_SET_ITEM(args, 0, other);
    result = PythonQtSlotFunction_CallImpl(wrapper->classInfo(), wrapper->_obj, opSlot._slot, args, NULL, wrapper->_wrappedPtr);
    Py_DECREF(args);
    if (!result && !fallbackOpName.isEmpty()) {
      // try fallback if we did not get a result
      result = PythonQtInstanceWrapper_binaryfunc(self, other, fallbackOpName);
    }
  }
  return result;
}

#define BINARY_OP(NAME) \
static PyObject* PythonQtInstanceWrapper_ ## NAME(PyObject* self, PyObject* other) \
{ \
  static const QByteArray opName("__" #NAME "__"); \
  return PythonQtInstanceWrapper_binaryfunc(self, other, opName); \
}

#define BINARY_OP_INPLACE(NAME) \
  static PyObject* PythonQtInstanceWrapper_i ## NAME(PyObject* self, PyObject* other) \
{ \
  static const QByteArray opName("__i" #NAME "__"); \
  static const QByteArray fallbackName("__" #NAME "__"); \
  return PythonQtInstanceWrapper_binaryfunc(self, other, opName, fallbackName); \
}

BINARY_OP(add)
BINARY_OP(sub)
BINARY_OP(mul)
BINARY_OP(div)
BINARY_OP(and)
BINARY_OP(or)
BINARY_OP(xor)
BINARY_OP(mod)
BINARY_OP(lshift)
BINARY_OP(rshift)

BINARY_OP_INPLACE(add)
BINARY_OP_INPLACE(sub)
BINARY_OP_INPLACE(mul)
BINARY_OP_INPLACE(div)
BINARY_OP_INPLACE(and)
BINARY_OP_INPLACE(or)
BINARY_OP_INPLACE(xor)
BINARY_OP_INPLACE(mod)
BINARY_OP_INPLACE(lshift)
BINARY_OP_INPLACE(rshift)

static void initializeSlots(PythonQtClassWrapper* wrap)
{
  int typeSlots = wrap->classInfo()->typeSlots();
  if (typeSlots) {
    if (typeSlots & PythonQt::Type_Add) {
      wrap->_base.as_number.nb_add = (binaryfunc)PythonQtInstanceWrapper_add;
    }
    if (typeSlots & PythonQt::Type_Subtract) {
      wrap->_base.as_number.nb_subtract = (binaryfunc)PythonQtInstanceWrapper_sub;
    }
    if (typeSlots & PythonQt::Type_Multiply) {
      wrap->_base.as_number.nb_multiply = (binaryfunc)PythonQtInstanceWrapper_mul;
    }
    if (typeSlots & PythonQt::Type_Divide) {
      wrap->_base.as_number.nb_divide = (binaryfunc)PythonQtInstanceWrapper_div;
      wrap->_base.as_number.nb_true_divide = (binaryfunc)PythonQtInstanceWrapper_div;
    }
    if (typeSlots & PythonQt::Type_And) {
      wrap->_base.as_number.nb_and = (binaryfunc)PythonQtInstanceWrapper_and;
    }
    if (typeSlots & PythonQt::Type_Or) {
      wrap->_base.as_number.nb_or = (binaryfunc)PythonQtInstanceWrapper_or;
    }
    if (typeSlots & PythonQt::Type_Xor) {
      wrap->_base.as_number.nb_xor = (binaryfunc)PythonQtInstanceWrapper_xor;
    }
    if (typeSlots & PythonQt::Type_Mod) {
      wrap->_base.as_number.nb_remainder = (binaryfunc)PythonQtInstanceWrapper_mod;
    }
    if (typeSlots & PythonQt::Type_LShift) {
      wrap->_base.as_number.nb_lshift = (binaryfunc)PythonQtInstanceWrapper_lshift;
    }
    if (typeSlots & PythonQt::Type_RShift) {
      wrap->_base.as_number.nb_rshift = (binaryfunc)PythonQtInstanceWrapper_rshift;
    }

    if (typeSlots & PythonQt::Type_InplaceAdd) {
      wrap->_base.as_number.nb_add = (binaryfunc)PythonQtInstanceWrapper_iadd;
    }
    if (typeSlots & PythonQt::Type_InplaceSubtract) {
      wrap->_base.as_number.nb_subtract = (binaryfunc)PythonQtInstanceWrapper_isub;
    }
    if (typeSlots & PythonQt::Type_InplaceMultiply) {
      wrap->_base.as_number.nb_multiply = (binaryfunc)PythonQtInstanceWrapper_imul;
    }
    if (typeSlots & PythonQt::Type_InplaceDivide) {
      wrap->_base.as_number.nb_inplace_divide = (binaryfunc)PythonQtInstanceWrapper_idiv;
      wrap->_base.as_number.nb_inplace_true_divide = (binaryfunc)PythonQtInstanceWrapper_idiv;
    }
    if (typeSlots & PythonQt::Type_InplaceAnd) {
      wrap->_base.as_number.nb_inplace_and = (binaryfunc)PythonQtInstanceWrapper_iand;
    }
    if (typeSlots & PythonQt::Type_InplaceOr) {
      wrap->_base.as_number.nb_inplace_or = (binaryfunc)PythonQtInstanceWrapper_ior;
    }
    if (typeSlots & PythonQt::Type_InplaceXor) {
      wrap->_base.as_number.nb_inplace_xor = (binaryfunc)PythonQtInstanceWrapper_ixor;
    }
    if (typeSlots & PythonQt::Type_InplaceMod) {
      wrap->_base.as_number.nb_inplace_remainder = (binaryfunc)PythonQtInstanceWrapper_imod;
    }
    if (typeSlots & PythonQt::Type_InplaceLShift) {
      wrap->_base.as_number.nb_inplace_lshift = (binaryfunc)PythonQtInstanceWrapper_ilshift;
    }
    if (typeSlots & PythonQt::Type_InplaceRShift) {
      wrap->_base.as_number.nb_inplace_rshift = (binaryfunc)PythonQtInstanceWrapper_irshift;
    }
    if (typeSlots & PythonQt::Type_Invert) {
      wrap->_base.as_number.nb_invert = (unaryfunc)PythonQtInstanceWrapper_invert;
    }
    if (typeSlots & PythonQt::Type_NonZero) {
      wrap->_base.as_number.nb_nonzero = (inquiry)PythonQtInstanceWrapper_nonzero;
    }
  }
}

static PyObject* PythonQtClassWrapper_alloc(PyTypeObject *self, Py_ssize_t nitems)
{
  // call the default type alloc
  PyObject* obj = PyType_Type.tp_alloc(self, nitems);

  // take current class type, if we are called via newPythonQtClassWrapper()
  PythonQtClassWrapper* wrap = (PythonQtClassWrapper*)obj;
  wrap->_classInfo = PythonQt::priv()->currentClassInfoForClassWrapperCreation();
  if (wrap->_classInfo) {
    initializeSlots(wrap);
  }

  return obj;
}


static int PythonQtClassWrapper_init(PythonQtClassWrapper* self, PyObject* args, PyObject* kwds)
{
  // call the default type init
  if (PyType_Type.tp_init((PyObject *)self, args, kwds) < 0) {
    return -1;
  }

  // if we have no CPP class information, try our base class
  if (!self->classInfo()) {
    PyTypeObject*  superType = ((PyTypeObject *)self)->tp_base;

    if (!superType || (superType->ob_type != &PythonQtClassWrapper_Type)) {
      PyErr_Format(PyExc_TypeError, "type %s is not derived from PythonQtClassWrapper", ((PyTypeObject*)self)->tp_name);
      return -1;
    }

    // take the class info from the superType
    self->_classInfo = ((PythonQtClassWrapper*)superType)->classInfo();
  }

  return 0;
}

static PyObject *PythonQtClassWrapper_classname(PythonQtClassWrapper* type)
{
  return PyString_FromString((QString("Class_") + type->classInfo()->className()).toLatin1().data());
}

static PyObject *PythonQtClassWrapper_help(PythonQtClassWrapper* type)
{
  return PythonQt::self()->helpCalled(type->classInfo());
}

PyObject *PythonQtClassWrapper__init__(PythonQtClassWrapper *type, PyObject *args)
{
  Py_ssize_t argc = PyTuple_Size(args);
  if (argc>0) {
    // we need to call __init__ of the instance
    PyObject* self = PyTuple_GET_ITEM(args, 0);
    if (PyObject_TypeCheck(self, (PyTypeObject*)type->classInfo()->pythonQtClassWrapper())) {
      PyObject* newargs = PyTuple_New(argc-1);
      for (int i = 0;i<argc-1; i++) {
        PyTuple_SET_ITEM(newargs, i,PyTuple_GET_ITEM(args, i+1));
      }
      PythonQtInstanceWrapper* wrapper = (PythonQtInstanceWrapper*)self;
      int result = PythonQtInstanceWrapper_init(wrapper, newargs, NULL);
      Py_DECREF(newargs);
      if (result==0) {
        Py_INCREF(Py_None);
        return Py_None;
      } else {
        // init failed!
      }
    } else {
      // self not of correct type!
    }
  } else {
    // wrong number of args
  }
  return NULL;
}

static PyMethodDef PythonQtClassWrapper_methods[] = {
    {"__init__", (PyCFunction)PythonQtClassWrapper__init__, METH_VARARGS,
    "Return the classname of the object"
    },
    {"className", (PyCFunction)PythonQtClassWrapper_classname, METH_NOARGS,
     "Return the classname of the object"
    },
    {"help", (PyCFunction)PythonQtClassWrapper_help, METH_NOARGS,
    "Shows the help of available methods for this class"
    },
    {NULL, NULL, 0 , NULL}  /* Sentinel */
};


static PyObject *PythonQtClassWrapper_getattro(PyObject *obj, PyObject *name)
{
  const char *attributeName;
  PythonQtClassWrapper *wrapper = (PythonQtClassWrapper *)obj;
  
  if ((attributeName = PyString_AsString(name)) == NULL) {
    return NULL;
  }
  if (obj == (PyObject*)&PythonQtInstanceWrapper_Type) {
    return NULL;
  }

  if (qstrcmp(attributeName, "__dict__")==0) {
    PyObject* dict = ((PyTypeObject *)wrapper)->tp_dict;
    if (!wrapper->classInfo()) {
      Py_INCREF(dict);
      return dict;
    }
    dict = PyDict_Copy(dict);
    
    QStringList l = wrapper->classInfo()->memberList(false);
    foreach (QString name, l) {
      PyObject* o = PyObject_GetAttrString(obj, name.toLatin1().data());
      if (o) {
        PyDict_SetItemString(dict, name.toLatin1().data(), o);
        Py_DECREF(o);
      } else {
        // it must have been a property or child, which we do not know as a class object...
      }
    }
    if (wrapper->classInfo()->constructors()) {
      PyObject* func = PyCFunction_New(&PythonQtClassWrapper_methods[0], obj);
      PyDict_SetItemString(dict, "__init__", func);
      Py_DECREF(func);
    }
    for (int i = 1;i<3;i++) {
      PyObject* func = PyCFunction_New(&PythonQtClassWrapper_methods[i], obj);
      PyDict_SetItemString(dict, PythonQtClassWrapper_methods[i].ml_name, func);
      Py_DECREF(func);
    }
    return dict;
  }

  if (wrapper->classInfo()) {
    PythonQtMemberInfo member = wrapper->classInfo()->member(attributeName);
    if (member._type == PythonQtMemberInfo::EnumValue) {
      PyObject* enumValue = member._enumValue;
      Py_INCREF(enumValue);
      return enumValue;
    } else if (member._type == PythonQtMemberInfo::EnumWrapper) {
      PyObject* enumWrapper = member._enumWrapper;
      Py_INCREF(enumWrapper);
      return enumWrapper;
    } else if (member._type == PythonQtMemberInfo::Slot) {
      // we return all slots, even the instance slots, since they are callable as unbound slots with self argument
      return PythonQtSlotFunction_New(member._slot, obj, NULL);
    }
  }

  // look for the interal methods (className(), help())
  PyObject* internalMethod = Py_FindMethod( PythonQtClassWrapper_methods, obj, (char*)attributeName);
  if (internalMethod) {
    return internalMethod;
  }
  PyErr_Clear();

  // look in super
  PyObject* superAttr = PyType_Type.tp_getattro(obj, name);
  if (superAttr) {
    return superAttr;
  }

  QString error = QString(wrapper->classInfo()->className()) + " has no attribute named '" + QString(attributeName) + "'";
  PyErr_SetString(PyExc_AttributeError, error.toLatin1().data());
  return NULL;
}

static int PythonQtClassWrapper_setattro(PyObject *obj,PyObject *name,PyObject *value)
{
  return PyType_Type.tp_setattro(obj,name,value);
}

/*
static PyObject * PythonQtClassWrapper_repr(PyObject * obj)
{
  PythonQtClassWrapper* wrapper = (PythonQtClassWrapper*)obj;
  if (wrapper->classInfo()->isCPPWrapper()) {
    const QMetaObject* meta = wrapper->classInfo()->metaObject(); 
    if (!meta) {
      QObject* decorator = wrapper->classInfo()->decorator();
      if (decorator) {
        meta = decorator->metaObject();
      }
    }
    if (meta) {
      return PyString_FromFormat("%s Class (C++ wrapped by %s)", wrapper->classInfo()->className(), meta->className());
    } else {
      return PyString_FromFormat("%s Class (C++ unwrapped)", wrapper->classInfo()->className());
    }
  } else {
    return PyString_FromFormat("%s Class", wrapper->classInfo()->className());
  }
}

*/

PyTypeObject PythonQtClassWrapper_Type = {
    PyObject_HEAD_INIT(NULL)
    0,                         /*ob_size*/
    "PythonQt.PythonQtClassWrapper",             /*tp_name*/
    sizeof(PythonQtClassWrapper),             /*tp_basicsize*/
    0,                         /*tp_itemsize*/
    0,                         /*tp_dealloc*/
    0,                         /*tp_print*/
    0,                         /*tp_getattr*/
    0,                         /*tp_setattr*/
    0,                         /*tp_compare*/
    0, //PythonQtClassWrapper_repr,            /*tp_repr*/
    0,                         /*tp_as_number*/
    0,                         /*tp_as_sequence*/
    0,                         /*tp_as_mapping*/
    0,                         /*tp_hash */
    0,                         /*tp_call*/
    0,                         /*tp_str*/
    PythonQtClassWrapper_getattro,                         /*tp_getattro*/
    PythonQtClassWrapper_setattro,                         /*tp_setattro*/
    0,                         /*tp_as_buffer*/
    Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE, /*tp_flags*/
    0,           /* tp_doc */
    0,                   /* tp_traverse */
    0,                   /* tp_clear */
    0,                   /* tp_richcompare */
    0,                   /* tp_weaklistoffset */
    0,                   /* tp_iter */
    0,                   /* tp_iternext */
    0,                   /* tp_methods */
    0,                   /* tp_members */
    0,                         /* tp_getset */
    0,                         /* tp_base */
    0,                         /* tp_dict */
    0,                         /* tp_descr_get */
    0,                         /* tp_descr_set */
    0,                         /* tp_dictoffset */
    (initproc)PythonQtClassWrapper_init,      /* tp_init */
    PythonQtClassWrapper_alloc,                         /* tp_alloc */
    0,                         /* tp_new */
    0,                         /* tp_free */
};

//-------------------------------------------------------

