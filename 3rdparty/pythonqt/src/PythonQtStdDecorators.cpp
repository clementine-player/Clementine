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
// \file    PythonQtStdDecorators.cpp
// \author  Florian Link
// \author  Last changed by $Author: florian $
// \date    2007-04
*/
//----------------------------------------------------------------------------------

#include "PythonQtStdDecorators.h"
#include "PythonQt.h"
#include "PythonQtClassInfo.h"
#include <QCoreApplication>

bool PythonQtStdDecorators::connect(QObject* sender, const QByteArray& signal, PyObject* callable)
{
  QByteArray signalTmp;
  char first = signal.at(0);
  if (first>='0' && first<='9') {
    signalTmp = signal;
  } else {
    signalTmp = "2" + signal;
  }
    
  if (sender) {
    return PythonQt::self()->addSignalHandler(sender, signalTmp, callable);
  } else {
    return false;
  }
}

bool PythonQtStdDecorators::connect(QObject* sender, const QByteArray& signal, QObject* receiver, const QByteArray& slot)
{
  bool r = false;
  if (sender && receiver) {
    QByteArray signalTmp;
    char first = signal.at(0);
    if (first>='0' && first<='9') {
      signalTmp = signal;
    } else {
      signalTmp = "2" + signal;
    }

    QByteArray slotTmp;
    first = slot.at(0);
    if (first>='0' && first<='9') {
      slotTmp = slot;
    } else {
      slotTmp = "1" + slot;
    }
    r = QObject::connect(sender, signalTmp, receiver, slotTmp);
  }
  return r;
}

bool PythonQtStdDecorators::disconnect(QObject* sender, const QByteArray& signal, PyObject* callable)
{
  QByteArray signalTmp;
  char first = signal.at(0);
  if (first>='0' && first<='9') {
    signalTmp = signal;
  } else {
    signalTmp = "2" + signal;
  }
  if (sender) {
    return PythonQt::self()->removeSignalHandler(sender, signalTmp, callable);
  } else {
    return false;
  }
}

bool PythonQtStdDecorators::disconnect(QObject* sender, const QByteArray& signal, QObject* receiver, const QByteArray& slot)
{
  bool r = false;
  if (sender && receiver) {
    QByteArray signalTmp;
    char first = signal.at(0);
    if (first>='0' && first<='9') {
      signalTmp = signal;
    } else {
      signalTmp = "2" + signal;
    }
    
    QByteArray slotTmp;
    first = slot.at(0);
    if (first>='0' && first<='9') {
      slotTmp = slot;
    } else {
      slotTmp = "1" + slot;
    }
    
    r = QObject::disconnect(sender, signalTmp, receiver, slotTmp);
  }
  return r;
}

#undef emit
void PythonQtStdDecorators::emit(QObject* sender, const QByteArray& signal, PyObject* arg1 ,PyObject* arg2 ,
          PyObject* arg3 ,PyObject* arg4 ,PyObject* arg5 ,PyObject* arg6 ,PyObject* arg7 )
{
  // TODO xxx
  // use normal PythonQtSlot calling code, add "allowSignal" to member lookup?!
}
#define emit

QObject* PythonQtStdDecorators::parent(QObject* o) {
  return o->parent();
}

void PythonQtStdDecorators::setParent(QObject* o, QObject* parent)
{
  o->setParent(parent);
}

const QObjectList* PythonQtStdDecorators::children(QObject* o)
{
  return &o->children();
}

bool PythonQtStdDecorators::setProperty(QObject* o, const char* name, const QVariant& value)
{
  return o->setProperty(name, value);
}

QVariant PythonQtStdDecorators::property(QObject* o, const char* name)
{
  return o->property(name);
}

QString PythonQtStdDecorators::tr(QObject* obj, const QByteArray& text, const QByteArray& ambig, int n)
{
  return QCoreApplication::translate(obj->metaObject()->className(), text.constData(), ambig.constData(), QCoreApplication::CodecForTr, n);
}

QObject* PythonQtStdDecorators::findChild(QObject* parent, PyObject* type, const QString& name)
{
  const QMetaObject* meta = NULL;
  const char* typeName = NULL;

  if (PyObject_TypeCheck(type, &PythonQtClassWrapper_Type)) {
    meta = ((PythonQtClassWrapper*)type)->classInfo()->metaObject();
  } else if (PyObject_TypeCheck(type, &PythonQtInstanceWrapper_Type)) {
    meta = ((PythonQtInstanceWrapper*)type)->classInfo()->metaObject();
  } else if (PyString_Check(type)) {
    typeName = PyString_AsString(type);
  }

  if (!typeName && !meta)
    return NULL;

  return findChild(parent, typeName, meta, name);
}

QList<QObject*> PythonQtStdDecorators::findChildren(QObject* parent, PyObject* type, const QString& name)
{
  const QMetaObject* meta = NULL;
  const char* typeName = NULL;

  if (PyObject_TypeCheck(type, &PythonQtClassWrapper_Type)) {
    meta = ((PythonQtClassWrapper*)type)->classInfo()->metaObject();
  } else if (PyObject_TypeCheck(type, &PythonQtInstanceWrapper_Type)) {
    meta = ((PythonQtInstanceWrapper*)type)->classInfo()->metaObject();
  } else if (PyString_Check(type)) {
    typeName = PyString_AsString(type);
  }

  QList<QObject*> list;

  if (!typeName && !meta)
    return list;

  findChildren(parent, typeName, meta, name, list);

  return list;
}

QList<QObject*> PythonQtStdDecorators::findChildren(QObject* parent, PyObject* type, const QRegExp& regExp)
{
  const QMetaObject* meta = NULL;
  const char* typeName = NULL;

  if (PyObject_TypeCheck(type, &PythonQtClassWrapper_Type)) {
    meta = ((PythonQtClassWrapper*)type)->classInfo()->metaObject();
  } else if (PyObject_TypeCheck(type, &PythonQtInstanceWrapper_Type)) {
    meta = ((PythonQtInstanceWrapper*)type)->classInfo()->metaObject();
  } else if (PyString_Check(type)) {
    typeName = PyString_AsString(type);
  }

  QList<QObject*> list;

  if (!typeName && !meta)
    return list;

  findChildren(parent, typeName, meta, regExp, list);

  return list;
}

QObject* PythonQtStdDecorators::findChild(QObject* parent, const char* typeName, const QMetaObject* meta, const QString& name)
{
  const QObjectList &children = parent->children();

  int i;
  for (i = 0; i < children.size(); ++i) {
    QObject* obj = children.at(i);

    if (!obj)
      return NULL;

    // Skip if the name doesn't match.
    if (!name.isNull() && obj->objectName() != name)
      continue;

    if ((typeName && obj->inherits(typeName)) ||        
      (meta && meta->cast(obj)))
      return obj;
  }

  for (i = 0; i < children.size(); ++i) {
    QObject* obj = findChild(children.at(i), typeName, meta, name);

    if (obj != NULL)
      return obj;
  }

  return NULL;
}

int PythonQtStdDecorators::findChildren(QObject* parent, const char* typeName, const QMetaObject* meta, const QString& name, QList<QObject*>& list)
{
  const QObjectList& children = parent->children();
  int i;

  for (i = 0; i < children.size(); ++i) {
    QObject* obj = children.at(i);

    if (!obj)
      return -1;

    // Skip if the name doesn't match.
    if (!name.isNull() && obj->objectName() != name)
      continue;

    if ((typeName && obj->inherits(typeName)) ||        
      (meta && meta->cast(obj))) {
        list += obj;
    }

    if (findChildren(obj, typeName, meta, name, list) < 0)
      return -1;
  }

  return 0;
}

int PythonQtStdDecorators::findChildren(QObject* parent, const char* typeName, const QMetaObject* meta, const QRegExp& regExp, QList<QObject*>& list)
{
  const QObjectList& children = parent->children();
  int i;

  for (i = 0; i < children.size(); ++i) {
    QObject* obj = children.at(i);

    if (!obj)
      return -1;

    // Skip if the name doesn't match.
    if (regExp.indexIn(obj->objectName()) == -1)
      continue;

    if ((typeName && obj->inherits(typeName)) ||        
      (meta && meta->cast(obj))) {
        list += obj;
    }

    if (findChildren(obj, typeName, meta, regExp, list) < 0)
      return -1;
  }

  return 0;
}
