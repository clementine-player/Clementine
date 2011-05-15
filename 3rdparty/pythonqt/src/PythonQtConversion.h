#ifndef _PYTHONQTCONVERSION_H
#define _PYTHONQTCONVERSION_H

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
// \file    PythonQtConversion.h
// \author  Florian Link
// \author  Last changed by $Author: florian $
// \date    2006-05
*/
//----------------------------------------------------------------------------------

#include "PythonQt.h"
#include "PythonQtMisc.h"
#include "PythonQtClassInfo.h"
#include "PythonQtMethodInfo.h"

#include <QWidget>
#include <QList>
#include <vector>

typedef PyObject* PythonQtConvertMetaTypeToPythonCB(const void* inObject, int metaTypeId);
typedef bool PythonQtConvertPythonToMetaTypeCB(PyObject* inObject, void* outObject, int metaTypeId, bool strict);

#define PythonQtRegisterListTemplateConverter(type, innertype) \
{ int typeId = qRegisterMetaType<type<innertype> >(#type"<"#innertype">"); \
PythonQtConv::registerPythonToMetaTypeConverter(typeId, PythonQtConvertPythonListToListOfValueType<type<innertype>, innertype>); \
PythonQtConv::registerMetaTypeToPythonConverter(typeId, PythonQtConvertListOfValueTypeToPythonList<type<innertype>, innertype>); \
}

#define PythonQtRegisterToolClassesTemplateConverter(innertype) \
  PythonQtRegisterListTemplateConverter(QList, innertype); \
  PythonQtRegisterListTemplateConverter(QVector, innertype); \
  PythonQtRegisterListTemplateConverter(std::vector, innertype);
// TODO: add QHash etc. here!

//! a static class that offers methods for type conversion
class PYTHONQT_EXPORT PythonQtConv {

public:

  //! get a ref counted True or False Python object
  static PyObject* GetPyBool(bool val);

  //! converts the Qt parameter given in \c data, interpreting it as a \c info parameter, into a Python object,
  static PyObject* ConvertQtValueToPython(const PythonQtMethodInfo::ParameterInfo& info, const void* data);

  //! convert python object to Qt (according to the given parameter) and if the conversion should be strict (classInfo is currently not used anymore)
  static void* ConvertPythonToQt(const PythonQtMethodInfo::ParameterInfo& info, PyObject* obj, bool strict, PythonQtClassInfo* classInfo, void* alreadyAllocatedCPPObject = NULL);

  //! creates a data storage for the passed parameter type and returns a void pointer to be set as arg[0] of qt_metacall
  static void* CreateQtReturnValue(const PythonQtMethodInfo::ParameterInfo& info);

  //! converts QString to Python string (unicode!)
  static PyObject* QStringToPyObject(const QString& str);

  //! converts QStringList to Python tuple
  static PyObject* QStringListToPyObject(const QStringList& list);

  //! converts QStringList to Python list
  static PyObject* QStringListToPyList(const QStringList& list);

    //! get string representation of py object
  static QString PyObjGetRepresentation(PyObject* val);

  //! get string value from py object
  static QString PyObjGetString(PyObject* val) { bool ok; QString s = PyObjGetString(val, false, ok); return s; }
  //! get string value from py object
  static QString PyObjGetString(PyObject* val, bool strict, bool &ok);
  //! get bytes from py object
  static QByteArray PyObjGetBytes(PyObject* val, bool strict, bool &ok);
  //! get int from py object
  static int     PyObjGetInt(PyObject* val, bool strict, bool &ok);
  //! get int64 from py object
  static qint64  PyObjGetLongLong(PyObject* val, bool strict, bool &ok);
  //! get int64 from py object
  static quint64  PyObjGetULongLong(PyObject* val, bool strict, bool &ok);
  //! get double from py object
  static double  PyObjGetDouble(PyObject* val, bool strict, bool &ok);
  //! get bool from py object
  static bool    PyObjGetBool(PyObject* val, bool strict, bool &ok);

  //! create a string list from python sequence
  static QStringList PyObjToStringList(PyObject* val, bool strict, bool& ok);

  //! convert python object to qvariant, if type is given it will try to create a qvariant of that type, otherwise
  //! it will guess from the python type
  static QVariant PyObjToQVariant(PyObject* val, int type = -1);

  //! convert QVariant from PyObject
  static PyObject* QVariantToPyObject(const QVariant& v);

  static PyObject* QVariantMapToPyObject(const QVariantMap& m);
  static PyObject* QVariantListToPyObject(const QVariantList& l);
  
  //! get human readable string from CPP object (when the metatype is known)
  static QString CPPObjectToString(int type, const void* data);
    
  //! register a converter callback from python to cpp for given metatype
  static void registerPythonToMetaTypeConverter(int metaTypeId, PythonQtConvertPythonToMetaTypeCB* cb) { _pythonToMetaTypeConverters.insert(metaTypeId, cb); }

  //! register a converter callback from cpp to python for given metatype
  static void registerMetaTypeToPythonConverter(int metaTypeId, PythonQtConvertMetaTypeToPythonCB* cb) { _metaTypeToPythonConverters.insert(metaTypeId, cb); }

  //! returns the inner type id of a simple template of the form SomeObject<InnerType>
  static int getInnerTemplateMetaType(const QByteArray& typeName);

  //! converts the Qt parameter given in \c data, interpreting it as a \c type registered qvariant/meta type, into a Python object,
  static PyObject* ConvertQtValueToPythonInternal(int type, const void* data);

public:

  static PythonQtValueStorage<qint64, 128>  global_valueStorage;
  static PythonQtValueStorage<void*, 128>   global_ptrStorage;
  static PythonQtValueStorage<QVariant, 32> global_variantStorage;

protected:
  static QHash<int, PythonQtConvertMetaTypeToPythonCB*> _metaTypeToPythonConverters; 
  static QHash<int, PythonQtConvertPythonToMetaTypeCB*> _pythonToMetaTypeConverters; 
 
  //! handle automatic conversion of some special types (QColor, QBrush, ...)
  static void* handlePythonToQtAutoConversion(int typeId, PyObject* obj, void* alreadyAllocatedCPPObject);

  //! converts the list of pointers of given type to Python
  static PyObject* ConvertQListOfPointerTypeToPythonList(QList<void*>* list, const QByteArray& type);
  //! tries to convert the python object to a QList of pointers to \c type objects, returns true on success
  static bool      ConvertPythonListToQListOfPointerType(PyObject* obj, QList<void*>* list, const QByteArray& type, bool strict);

  //! cast wrapper to given className if possible
  static void* castWrapperTo(PythonQtInstanceWrapper* wrapper, const QByteArray& className, bool& ok);
};

template<class ListType, class T>
PyObject* PythonQtConvertListOfValueTypeToPythonList(const void* /*QList<T>* */ inList, int metaTypeId)
{
  ListType* list = (ListType*)inList; 
  static const int innerType = PythonQtConv::getInnerTemplateMetaType(QByteArray(QMetaType::typeName(metaTypeId)));
  if (innerType == QVariant::Invalid) {
    std::cerr << "PythonQtConvertListOfValueTypeToPythonList: unknown inner type " << QMetaType::typeName(metaTypeId) << std::endl;
  }
  PyObject* result = PyTuple_New(list->size());
  int i = 0;
  foreach (const T& value, *list) {
    PyTuple_SET_ITEM(result, i, PythonQtConv::ConvertQtValueToPythonInternal(innerType, &value));
    i++;
  }
  return result;
}

template<class ListType, class T>
bool PythonQtConvertPythonListToListOfValueType(PyObject* obj, void* /*QList<T>* */ outList, int metaTypeId, bool /*strict*/)
{
  ListType* list = (ListType*)outList; 
  static const int innerType = PythonQtConv::getInnerTemplateMetaType(QByteArray(QMetaType::typeName(metaTypeId)));
  if (innerType == QVariant::Invalid) {
    std::cerr << "PythonQtConvertPythonListToListOfValueType: unknown inner type " << QMetaType::typeName(metaTypeId) << std::endl;
  }
  bool result = false;
  if (PySequence_Check(obj)) {
    result = true;
    int count = PySequence_Size(obj);
    PyObject* value;
    for (int i = 0;i<count;i++) {
      value = PySequence_GetItem(obj,i);
      // this is quite some overhead, but it avoids having another large switch...
      QVariant v = PythonQtConv::PyObjToQVariant(value, innerType);
      if (v.isValid()) {
        list->push_back(qVariantValue<T>(v));
      } else {
        result = false;
        break;
      }
    }
  }
  return result;
}

#endif
