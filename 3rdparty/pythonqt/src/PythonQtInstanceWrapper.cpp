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
// \file    PythonQtInstanceWrapper.cpp
// \author  Florian Link
// \author  Last changed by $Author: florian $
// \date    2006-05
*/
//----------------------------------------------------------------------------------

#include "PythonQtInstanceWrapper.h"
#include <QObject>
#include "PythonQt.h"
#include "PythonQtSlot.h"
#include "PythonQtClassInfo.h"
#include "PythonQtConversion.h"
#include "PythonQtClassWrapper.h"

PythonQtClassInfo* PythonQtInstanceWrapperStruct::classInfo()
{
  // take the class info from our type object
  return ((PythonQtClassWrapper*)ob_type)->_classInfo;
}

static void PythonQtInstanceWrapper_deleteObject(PythonQtInstanceWrapper* self, bool force = false) {

  // is this a C++ wrapper?
  if (self->_wrappedPtr) {
    //mlabDebugConst("Python","c++ wrapper removed " << self->_wrappedPtr << " " << self->_obj->className() << " " << self->classInfo()->wrappedClassName().latin1());

    PythonQt::priv()->removeWrapperPointer(self->_wrappedPtr);
    // we own our qobject, so we delete it now:
    delete self->_obj;
    self->_obj = NULL;
    if (force || self->classInfo()->hasOwnerMethodButNoOwner(self->_wrappedPtr) || self->_ownedByPythonQt) {
      int type = self->classInfo()->metaTypeId();
      if (self->_useQMetaTypeDestroy && type>=0) {
        // use QMetaType to destroy the object
        QMetaType::destroy(type, self->_wrappedPtr);
      } else {
        PythonQtSlotInfo* slot = self->classInfo()->destructor();
        if (slot) {
          void* args[2];
          args[0] = NULL;
          args[1] = &self->_wrappedPtr;
          slot->decorator()->qt_metacall(QMetaObject::InvokeMetaMethod, slot->slotIndex(), args);
          self->_wrappedPtr = NULL;
        } else {
          if (type>=0) {
            // use QMetaType to destroy the object
            QMetaType::destroy(type, self->_wrappedPtr);
          } else {
            // TODO: warn about not being able to destroy the object?
          }
        }
      }
    }
  } else {
    //mlabDebugConst("Python","qobject wrapper removed " << self->_obj->className() << " " << self->classInfo()->wrappedClassName().latin1());
    if (self->_objPointerCopy) {
      PythonQt::priv()->removeWrapperPointer(self->_objPointerCopy);
    }
    if (self->_obj) {
      if (force || self->_ownedByPythonQt) {
        if (force || !self->_obj->parent()) {
          delete self->_obj;
        }
      } else {
        if (self->_obj->parent()==NULL) {
          // tell someone who is interested that the qobject is no longer wrapped, if it has no parent
          PythonQt::qObjectNoLongerWrappedCB(self->_obj);
        }
      }
    }
  }
  self->_obj = NULL;
}

static void PythonQtInstanceWrapper_dealloc(PythonQtInstanceWrapper* self)
{
  PythonQtInstanceWrapper_deleteObject(self);
  self->_obj.~QPointer<QObject>();
  self->ob_type->tp_free((PyObject*)self);
}

static PyObject* PythonQtInstanceWrapper_new(PyTypeObject *type, PyObject * /*args*/, PyObject * /*kwds*/)
{
  //PythonQtClassWrapper    *classType = (PythonQtClassWrapper*)type;
  PythonQtInstanceWrapper *self;
  static PyObject* emptyTuple = NULL;
  if (emptyTuple==NULL) {
    emptyTuple = PyTuple_New(0);
  }

  self = (PythonQtInstanceWrapper*)PyBaseObject_Type.tp_new(type, emptyTuple, NULL);

  if (self != NULL) {
    new (&self->_obj) QPointer<QObject>();
    self->_wrappedPtr = NULL;
    self->_ownedByPythonQt = false;
    self->_useQMetaTypeDestroy = false;
    self->_isShellInstance = false;
  }
  return (PyObject *)self;
}

int PythonQtInstanceWrapper_init(PythonQtInstanceWrapper * self, PyObject * args, PyObject * kwds)
{
  if (args == PythonQtPrivate::dummyTuple()) {
    // we are called from the internal PythonQt API, so our data will be filled later on...
    return 0;
  }

  // we are called from python, try to construct our object
  if (self->classInfo()->constructors()) {
    void* directCPPPointer = NULL;
    PythonQtSlotFunction_CallImpl(self->classInfo(), NULL, self->classInfo()->constructors(), args, kwds, NULL, &directCPPPointer);
    if (PyErr_Occurred()) {
      return -1;
    }
    if (directCPPPointer) {
      // change ownershipflag to be owned by PythonQt
      self->_ownedByPythonQt = true;
      self->_useQMetaTypeDestroy = false;
      if (self->classInfo()->isCPPWrapper()) {
        self->_wrappedPtr = directCPPPointer;
        // TODO xxx: if there is a wrapper factory, we might want to generate a wrapper for our class?!
      } else {
        self->setQObject((QObject*)directCPPPointer);
      }
      // register with PythonQt
      PythonQt::priv()->addWrapperPointer(directCPPPointer, self);

      PythonQtShellSetInstanceWrapperCB* cb = self->classInfo()->shellSetInstanceWrapperCB();
      if (cb) {
        // if we are a derived python class, we set the wrapper
        // to activate the shell class, otherwise we just ignore that it is a shell...
        // we detect it be checking if the type does not have PythonQtInstanceWrapper_Type as direct base class,
        // which is the case for all non-python derived types
        if (((PyObject*)self)->ob_type->tp_base != &PythonQtInstanceWrapper_Type) {
          // set the wrapper and remember that we have a shell instance!
          (*cb)(directCPPPointer, self);
          self->_isShellInstance = true;
        }
      }
    }
  } else {
    QString error = QString("No constructors available for ") + self->classInfo()->className();
    PyErr_SetString(PyExc_ValueError, error.toLatin1().data());
    return -1;
  }
  return 0;
}

static PyObject *PythonQtInstanceWrapper_richcompare(PythonQtInstanceWrapper* wrapper, PyObject* other, int code)
{
  bool validPtrs = false;
  bool areSamePtrs = false;
  if (PyObject_TypeCheck((PyObject*)wrapper, &PythonQtInstanceWrapper_Type)) {
    if (PyObject_TypeCheck(other, &PythonQtInstanceWrapper_Type)) {
      validPtrs = true;
      PythonQtInstanceWrapper* w1 = wrapper;
      PythonQtInstanceWrapper* w2 = (PythonQtInstanceWrapper*)other;
      // check pointers directly
      if (w1->_wrappedPtr != NULL) {
        if (w1->_wrappedPtr == w2->_wrappedPtr) {
          areSamePtrs = true;
        }
      } else if (w1->_obj == w2->_obj) {
        areSamePtrs = true;
      }
    } else if (other == Py_None) {
      validPtrs = true;
      if (wrapper->_obj || wrapper->_wrappedPtr) {
        areSamePtrs = false;
      } else {
        areSamePtrs = true;
      }
    }
  }

  if ((wrapper->classInfo()->typeSlots() & PythonQt::Type_RichCompare) == 0) {
    // shortcut if richcompare is not supported:
    if (validPtrs && code == Py_EQ) {
      return PythonQtConv::GetPyBool(areSamePtrs);
    } else if (validPtrs && code == Py_NE) {
      return PythonQtConv::GetPyBool(!areSamePtrs);
    }
    Py_INCREF(Py_NotImplemented);
    return Py_NotImplemented;
  }

  QByteArray memberName;
  switch (code) {
  case Py_LT:
    {
      static QByteArray name = "__lt__";
      memberName = name;
    }
    break;

  case Py_LE:
    {
      static QByteArray name = "__le__";
      memberName = name;
    }
    break;

  case Py_EQ:
    {
      static QByteArray name = "__eq__";
      memberName = name;
    }
    break;

  case Py_NE:
    {
      static QByteArray name = "__ne__";
      memberName = name;
    }
    break;

  case Py_GT:
    {
      static QByteArray name = "__gt__";
      memberName = name;
    }
    break;

  case Py_GE:
    {
      static QByteArray name = "__ge__";
      memberName = name;
    }
    break;
  }

  PythonQtMemberInfo opSlot = wrapper->classInfo()->member(memberName);
  if (opSlot._type == PythonQtMemberInfo::Slot) {
    // TODO get rid of tuple
    PyObject* args = PyTuple_New(1);
    Py_INCREF(other);
    PyTuple_SET_ITEM(args, 0, other);
    PyObject* result = PythonQtSlotFunction_CallImpl(wrapper->classInfo(), wrapper->_obj, opSlot._slot, args, NULL, wrapper->_wrappedPtr);
    Py_DECREF(args);
    return result;
  } else {
    // not implemented, let python try something else!
    Py_INCREF(Py_NotImplemented);
    return Py_NotImplemented;
  }
}


static PyObject *PythonQtInstanceWrapper_classname(PythonQtInstanceWrapper* obj)
{
  return PyString_FromString(obj->ob_type->tp_name);
}

static PyObject *PythonQtInstanceWrapper_help(PythonQtInstanceWrapper* obj)
{
  return PythonQt::self()->helpCalled(obj->classInfo());
}

static PyObject *PythonQtInstanceWrapper_delete(PythonQtInstanceWrapper * self)
{
  PythonQtInstanceWrapper_deleteObject(self, true);
  Py_INCREF(Py_None);
  return Py_None;
}


static PyMethodDef PythonQtInstanceWrapper_methods[] = {
    {"className", (PyCFunction)PythonQtInstanceWrapper_classname, METH_NOARGS,
     "Return the classname of the object"
    },
    {"help", (PyCFunction)PythonQtInstanceWrapper_help, METH_NOARGS,
    "Shows the help of available methods for this class"
    },
    {"delete", (PyCFunction)PythonQtInstanceWrapper_delete, METH_NOARGS,
    "Deletes the C++ object (at your own risk, my friend!)"
    },
{NULL, NULL, 0, NULL}  /* Sentinel */
};


static PyObject *PythonQtInstanceWrapper_getattro(PyObject *obj,PyObject *name)
{
  const char *attributeName;
  PythonQtInstanceWrapper *wrapper = (PythonQtInstanceWrapper *)obj;

  if ((attributeName = PyString_AsString(name)) == NULL) {
    return NULL;
  }

  if (qstrcmp(attributeName, "__dict__")==0) {
    PyObject* dict = PyBaseObject_Type.tp_getattro(obj, name);
    dict = PyDict_Copy(dict);

    if (wrapper->_obj) {
      // only the properties are missing, the rest is already available from
      // PythonQtClassWrapper...
      QStringList l = wrapper->classInfo()->propertyList();
      foreach (QString name, l) {
        PyObject* o = PyObject_GetAttrString(obj, name.toLatin1().data());
        if (o) {
          PyDict_SetItemString(dict, name.toLatin1().data(), o);
          Py_DECREF(o);
        } else {
          std::cerr << "PythonQtInstanceWrapper: something is wrong, could not get attribute " << name.toLatin1().data();
        }
      }

      QList<QByteArray> dynamicProps = wrapper->_obj->dynamicPropertyNames();
      foreach (QByteArray name, dynamicProps) {
        PyObject* o = PyObject_GetAttrString(obj, name.data());
        if (o) {
          PyDict_SetItemString(dict, name.data(), o);
          Py_DECREF(o);
        } else {
          std::cerr << "PythonQtInstanceWrapper: dynamic property could not be read " << name.data();
        }
      }
    }
    // Note: we do not put children into the dict, is would look confusing?!
    return dict;
  }

  // first look in super, to return derived methods from base object first
  PyObject* superAttr = PyBaseObject_Type.tp_getattro(obj, name);
  if (superAttr) {
    return superAttr;
  }
  PyErr_Clear();

  //  mlabDebugConst("Python","get " << attributeName);

  PythonQtMemberInfo member = wrapper->classInfo()->member(attributeName);
  switch (member._type) {
  case PythonQtMemberInfo::Property:
    if (wrapper->_obj) {
      if (member._property.userType() != QVariant::Invalid) {
        return PythonQtConv::QVariantToPyObject(member._property.read(wrapper->_obj));
      } else {
        Py_INCREF(Py_None);
        return Py_None;
      }
    } else {
      QString error = QString("Trying to read property '") + attributeName + "' from a destroyed " + wrapper->classInfo()->className() + " object";
      PyErr_SetString(PyExc_ValueError, error.toLatin1().data());
      return NULL;
    }
    break;
  case PythonQtMemberInfo::Slot:
    return PythonQtSlotFunction_New(member._slot, obj, NULL);
    break;
  case PythonQtMemberInfo::EnumValue:
    {
      PyObject* enumValue = member._enumValue;
      Py_INCREF(enumValue);
      return enumValue;
    }
    break;
  case PythonQtMemberInfo::EnumWrapper:
    {
      PyObject* enumWrapper = member._enumWrapper;
      Py_INCREF(enumWrapper);
      return enumWrapper;
    }
    break;
  case PythonQtMemberInfo::NotFound:
    {
      static const QByteArray getterString("py_get_");
      // check for a getter slot
      PythonQtMemberInfo member = wrapper->classInfo()->member(getterString + attributeName);
      if (member._type == PythonQtMemberInfo::Slot) {
        return PythonQtSlotFunction_CallImpl(wrapper->classInfo(), wrapper->_obj, member._slot, NULL, NULL, wrapper->_wrappedPtr);
      }

      // handle dynamic properties
      if (wrapper->_obj) {
        QVariant v = wrapper->_obj->property(attributeName);
        if (v.isValid()) {
          return PythonQtConv::QVariantToPyObject(v);
        }
      }
    }
    break;
  default:
    // is an invalid type, go on
    break;
  }

  // look for the internal methods (className(), help())
  PyObject* internalMethod = Py_FindMethod( PythonQtInstanceWrapper_methods, obj, (char*)attributeName);
  if (internalMethod) {
    return internalMethod;
  }
  PyErr_Clear();

  if (wrapper->_obj) {
    // look for a child
    QObjectList children = wrapper->_obj->children();
    for (int i = 0; i < children.count(); i++) {
      QObject *child = children.at(i);
      if (child->objectName() == attributeName) {
        return PythonQt::priv()->wrapQObject(child);
      }
    }
  }

  QString error = QString(wrapper->classInfo()->className()) + " has no attribute named '" + QString(attributeName) + "'";
  PyErr_SetString(PyExc_AttributeError, error.toLatin1().data());
  return NULL;
}

static int PythonQtInstanceWrapper_setattro(PyObject *obj,PyObject *name,PyObject *value)
{
  QString error;
  const char *attributeName;
  PythonQtInstanceWrapper *wrapper = (PythonQtInstanceWrapper *)obj;

  if ((attributeName = PyString_AsString(name)) == NULL)
    return -1;

  PythonQtMemberInfo member = wrapper->classInfo()->member(attributeName);
  if (member._type == PythonQtMemberInfo::Property) {

    if (!wrapper->_obj) {
      error = QString("Trying to set property '") + attributeName + "' on a destroyed " + wrapper->classInfo()->className() + " object";
      PyErr_SetString(PyExc_AttributeError, error.toLatin1().data());
      return -1;
    }

    QMetaProperty prop = member._property;
    if (prop.isWritable()) {
      QVariant v;
      if (prop.isEnumType()) {
        // this will give us either a string or an int, everything else will probably be an error
        v = PythonQtConv::PyObjToQVariant(value);
      } else {
        int t = prop.userType();
        v = PythonQtConv::PyObjToQVariant(value, t);
      }
      bool success = false;
      if (v.isValid()) {
        success = prop.write(wrapper->_obj, v);
      }
      if (success) {
        return 0;
      } else {
        error = QString("Property '") + attributeName + "' of type '" +
          prop.typeName() + "' does not accept an object of type "
          + QString(value->ob_type->tp_name) + " (" + PythonQtConv::PyObjGetRepresentation(value) + ")";
      }
    } else {
      error = QString("Property '") + attributeName + "' of " + obj->ob_type->tp_name + " object is not writable";
    }
  } else if (member._type == PythonQtMemberInfo::Slot) {
    error = QString("Slot '") + attributeName + "' can not be overwritten on " + obj->ob_type->tp_name + " object";
  } else if (member._type == PythonQtMemberInfo::EnumValue) {
    error = QString("EnumValue '") + attributeName + "' can not be overwritten on " + obj->ob_type->tp_name + " object";
  } else if (member._type == PythonQtMemberInfo::EnumWrapper) {
    error = QString("Enum '") + attributeName + "' can not be overwritten on " + obj->ob_type->tp_name + " object";
  } else if (member._type == PythonQtMemberInfo::NotFound) {
    // check for a setter slot
    static const QByteArray setterString("py_set_");
    PythonQtMemberInfo setter = wrapper->classInfo()->member(setterString + attributeName);
    if (setter._type == PythonQtMemberInfo::Slot) {
      // call the setter and ignore the result value
      void* result;
      PyObject* args = PyTuple_New(1);
      Py_INCREF(value);
      PyTuple_SET_ITEM(args, 0, value);
      PythonQtSlotFunction_CallImpl(wrapper->classInfo(), wrapper->_obj, setter._slot, args, NULL, wrapper->_wrappedPtr, &result);
      Py_DECREF(args);
      return 0;
    }

    // handle dynamic properties
    if (wrapper->_obj) {
      QVariant prop = wrapper->_obj->property(attributeName);
      if (prop.isValid()) {
        QVariant v = PythonQtConv::PyObjToQVariant(value);
        if (v.isValid()) {
          wrapper->_obj->setProperty(attributeName, v);
          return 0;
        } else {
          error = QString("Dynamic property '") + attributeName + "' does not accept an object of type "
          + QString(value->ob_type->tp_name) + " (" + PythonQtConv::PyObjGetRepresentation(value) + ")";
          PyErr_SetString(PyExc_AttributeError, error.toLatin1().data());
          return -1;
        }
      }
    }

    // if we are a derived python class, we allow setting attributes.
    // if we are a direct CPP wrapper, we do NOT allow it, since
    // it would be confusing to allow it because a wrapper will go away when it is not seen by python anymore
    // and when it is recreated from a CPP pointer the attributes are gone...
    if (obj->ob_type->tp_base != &PythonQtInstanceWrapper_Type) {
      return PyBaseObject_Type.tp_setattro(obj,name,value);
    } else {
      error = QString("'") + attributeName + "' does not exist on " + obj->ob_type->tp_name + " and creating new attributes on C++ objects is not allowed";
    }
  }

  PyErr_SetString(PyExc_AttributeError, error.toLatin1().data());
  return -1;
}

static QString getStringFromObject(PythonQtInstanceWrapper* wrapper) {
  QString result;
  if (wrapper->_wrappedPtr) {
    // first try some manually string conversions for some variants
    int metaid = wrapper->classInfo()->metaTypeId();
    result = PythonQtConv::CPPObjectToString(metaid, wrapper->_wrappedPtr);
    if (!result.isEmpty()) {
      return result;
    }
  }
  // next, try to call py_toString
  PythonQtMemberInfo info = wrapper->classInfo()->member("py_toString");
  if (info._type == PythonQtMemberInfo::Slot) {
    PyObject* resultObj = PythonQtSlotFunction_CallImpl(wrapper->classInfo(), wrapper->_obj, info._slot, NULL, NULL, wrapper->_wrappedPtr);
    if (resultObj) {
      // TODO this is one conversion too much, would be nicer to call the slot directly...
      result = PythonQtConv::PyObjGetString(resultObj);
      Py_DECREF(resultObj);
    }
  }
  return result;
}

static PyObject * PythonQtInstanceWrapper_str(PyObject * obj)
{
  PythonQtInstanceWrapper* wrapper = (PythonQtInstanceWrapper*)obj;

  // QByteArray should be directly returned as a str
  if (wrapper->classInfo()->metaTypeId()==QVariant::ByteArray) {
    QByteArray* b = (QByteArray*) wrapper->_wrappedPtr;
    if (b->data()) {
      return PyString_FromStringAndSize(b->data(), b->size());
    } else {
      return PyString_FromString("");
    }
  }

  const char* typeName = obj->ob_type->tp_name;
  QObject *qobj = wrapper->_obj;
  QString str = getStringFromObject(wrapper);
  if (!str.isEmpty()) {
    return PyString_FromFormat("%s", str.toLatin1().constData());
  }
  if (wrapper->_wrappedPtr) {
    if (wrapper->_obj) {
      return PyString_FromFormat("%s (C++ Object %p wrapped by %s %p))", typeName, wrapper->_wrappedPtr, wrapper->_obj->metaObject()->className(), qobj);
    } else {
      return PyString_FromFormat("%s (C++ Object %p)", typeName, wrapper->_wrappedPtr);
    }
  } else {
    return PyString_FromFormat("%s (QObject %p)", typeName, qobj);
  }
}

static PyObject * PythonQtInstanceWrapper_repr(PyObject * obj)
{
  PythonQtInstanceWrapper* wrapper = (PythonQtInstanceWrapper*)obj;
  const char* typeName = obj->ob_type->tp_name;

  QObject *qobj = wrapper->_obj;
  QString str = getStringFromObject(wrapper);
  if (!str.isEmpty()) {
    if (str.startsWith(typeName)) {
      return PyString_FromFormat("%s", str.toLatin1().constData());
    } else {
      return PyString_FromFormat("%s(%s, %p)", typeName, str.toLatin1().constData(), wrapper->_wrappedPtr);
    }
  }
  if (wrapper->_wrappedPtr) {
    if (wrapper->_obj) {
      return PyString_FromFormat("%s (C++ Object %p wrapped by %s %p))", typeName, wrapper->_wrappedPtr, wrapper->_obj->metaObject()->className(), qobj);
    } else {
      return PyString_FromFormat("%s (C++ Object %p)", typeName, wrapper->_wrappedPtr);
    }
  } else {
    return PyString_FromFormat("%s (%s %p)", typeName, wrapper->classInfo()->className(), qobj);
  }
}

static int PythonQtInstanceWrapper_builtin_nonzero(PyObject *obj)
{
  PythonQtInstanceWrapper* wrapper = (PythonQtInstanceWrapper*)obj;
  return (wrapper->_wrappedPtr == NULL && wrapper->_obj == NULL)?0:1;
}


static long PythonQtInstanceWrapper_hash(PythonQtInstanceWrapper *obj)
{
  if (obj->_wrappedPtr != NULL) {
    return reinterpret_cast<long>(obj->_wrappedPtr);
  } else {
    QObject* qobj = obj->_obj; // get pointer from QPointer wrapper
    return reinterpret_cast<long>(qobj);
  }
}



// we override nb_nonzero, so that one can do 'if' expressions to test for a NULL ptr
static PyNumberMethods PythonQtInstanceWrapper_as_number = {
  0,      /* nb_add */
    0,      /* nb_subtract */
    0,      /* nb_multiply */
    0,      /* nb_divide */
    0,      /* nb_remainder */
    0,      /* nb_divmod */
    0,      /* nb_power */
    0,      /* nb_negative */
    0,      /* nb_positive */
    0,      /* nb_absolute */
    PythonQtInstanceWrapper_builtin_nonzero,      /* nb_nonzero */
    0,      /* nb_invert */
    0,      /* nb_lshift */
    0,      /* nb_rshift */
    0,    /* nb_and */
    0,    /* nb_xor */
    0,    /* nb_or */
    0,      /* nb_coerce */
    0,      /* nb_int */
    0,      /* nb_long */
    0,      /* nb_float */
    0,      /* nb_oct */
    0,      /* nb_hex */
    0,      /* nb_inplace_add */
    0,      /* nb_inplace_subtract */
    0,      /* nb_inplace_multiply */
    0,      /* nb_inplace_divide */
    0,      /* nb_inplace_remainder */
    0,      /* nb_inplace_power */
    0,      /* nb_inplace_lshift */
    0,      /* nb_inplace_rshift */
    0,      /* nb_inplace_and */
    0,      /* nb_inplace_xor */
    0,      /* nb_inplace_or */
    0,      /* nb_floor_divide */
    0,      /* nb_true_divide */
    0,      /* nb_inplace_floor_divide */
    0,      /* nb_inplace_true_divide */
};

PyTypeObject PythonQtInstanceWrapper_Type = {
    PyObject_HEAD_INIT(&PythonQtClassWrapper_Type)
    0,                         /*ob_size*/
    "PythonQt.PythonQtInstanceWrapper",             /*tp_name*/
    sizeof(PythonQtInstanceWrapper),             /*tp_basicsize*/
    0,                         /*tp_itemsize*/
    (destructor)PythonQtInstanceWrapper_dealloc, /*tp_dealloc*/
    0,                         /*tp_print*/
    0,                         /*tp_getattr*/
    0,                         /*tp_setattr*/
    0,                         /*tp_compare*/
    PythonQtInstanceWrapper_repr,            /*tp_repr*/
    &PythonQtInstanceWrapper_as_number,                         /*tp_as_number*/
    0,                         /*tp_as_sequence*/
    0,                         /*tp_as_mapping*/
    (hashfunc)PythonQtInstanceWrapper_hash,                         /*tp_hash */
    0,                         /*tp_call*/
    PythonQtInstanceWrapper_str,      /*tp_str*/
    PythonQtInstanceWrapper_getattro,                         /*tp_getattro*/
    PythonQtInstanceWrapper_setattro,                         /*tp_setattro*/
    0,                         /*tp_as_buffer*/
    Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE | Py_TPFLAGS_CHECKTYPES, /*tp_flags*/
    "PythonQtInstanceWrapper object",           /* tp_doc */
    0,                   /* tp_traverse */
    0,                   /* tp_clear */
    (richcmpfunc)PythonQtInstanceWrapper_richcompare,                   /* tp_richcompare */
    0,                   /* tp_weaklistoffset */
    0,                   /* tp_iter */
    0,                   /* tp_iternext */
    0,             /* tp_methods */
    0,             /* tp_members */
    0,                         /* tp_getset */
    0,                         /* tp_base */
    0,                         /* tp_dict */
    0,                         /* tp_descr_get */
    0,                         /* tp_descr_set */
    0,                         /* tp_dictoffset */
    (initproc)PythonQtInstanceWrapper_init,      /* tp_init */
    0,                         /* tp_alloc */
    PythonQtInstanceWrapper_new,                 /* tp_new */
};

//-------------------------------------------------------

