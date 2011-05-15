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
// \file    PythonQtMethodInfo.cpp
// \author  Florian Link
// \author  Last changed by $Author: florian $
// \date    2006-05
*/
//----------------------------------------------------------------------------------

#include "PythonQtMethodInfo.h"
#include "PythonQtClassInfo.h"
#include <iostream>

QHash<QByteArray, PythonQtMethodInfo*> PythonQtMethodInfo::_cachedSignatures;
QHash<QByteArray, QByteArray> PythonQtMethodInfo::_parameterNameAliases;

PythonQtMethodInfo::PythonQtMethodInfo(const QMetaMethod& meta, PythonQtClassInfo* classInfo)
{
#ifdef PYTHONQT_DEBUG
  QByteArray sig(meta.signature());
  sig = sig.mid(sig.indexOf('('));
  QByteArray fullSig = QByteArray(meta.typeName()) + " " + sig;
  std::cout << "caching " << fullSig.data() << std::endl;
#endif

  ParameterInfo type;
  fillParameterInfo(type, QByteArray(meta.typeName()), classInfo);
  _parameters.append(type);
  QList<QByteArray> names = meta.parameterTypes();
  foreach (const QByteArray& name, names) {
    fillParameterInfo(type, name, classInfo);
    _parameters.append(type);
  }
}

PythonQtMethodInfo::PythonQtMethodInfo(const QByteArray& typeName, const QList<QByteArray>& args)
{
  ParameterInfo type;
  fillParameterInfo(type, typeName, NULL);
  _parameters.append(type);
  foreach (const QByteArray& name, args) {
    fillParameterInfo(type, name, NULL);
    _parameters.append(type);
  }
}

const PythonQtMethodInfo* PythonQtMethodInfo::getCachedMethodInfo(const QMetaMethod& signal, PythonQtClassInfo* classInfo)
{
  QByteArray sig(signal.signature());
  sig = sig.mid(sig.indexOf('('));
  QByteArray fullSig = QByteArray(signal.typeName()) + " " + sig;
  PythonQtMethodInfo* result = _cachedSignatures.value(fullSig);
  if (!result) {
    result = new PythonQtMethodInfo(signal, classInfo);
    _cachedSignatures.insert(fullSig, result);
  }
  return result;
}

const PythonQtMethodInfo* PythonQtMethodInfo::getCachedMethodInfoFromArgumentList(int numArgs, const char** args)
{
  QByteArray typeName = args[0];
  QList<QByteArray> arguments;
  QByteArray fullSig = typeName;
  fullSig += "(";
  for (int i =1;i<numArgs; i++) {
    if (i>1) {
      fullSig += ",";
    }
    arguments << QByteArray(args[i]);
  }
  fullSig += ")";
  PythonQtMethodInfo* result = _cachedSignatures.value(fullSig);
  if (!result) {
    result = new PythonQtMethodInfo(typeName, arguments);
    _cachedSignatures.insert(fullSig, result);
  }
  return result;
}

void PythonQtMethodInfo::fillParameterInfo(ParameterInfo& type, const QByteArray& orgName, PythonQtClassInfo* classInfo)
{
  QByteArray name = orgName;

  type.enumWrapper = NULL;
  
  int len = name.length();
  if (len>0) {
    if (strncmp(name.constData(), "const ", 6)==0) {
      name = name.mid(6);
      len -= 6;
      type.isConst = true;
    } else {
      type.isConst = false;
    }
    char pointerCount = 0;
    bool hadReference = false;
    // remove * and & from the end of the string, handle & and * the same way
    while (name.at(len-1) == '*') {
      len--;
      pointerCount++;
    }
    while (name.at(len-1) == '&') {
      len--;
      hadReference = true;
    }
    if (len!=name.length()) {
      name = name.left(len);
    }
    type.pointerCount = pointerCount;

    QByteArray alias = _parameterNameAliases.value(name);
    if (!alias.isEmpty()) {
      name = alias;
    }

    type.typeId = nameToType(name);
    if ((type.pointerCount == 0) && type.typeId == Unknown) {
      type.typeId = QMetaType::type(name.constData());
      if (type.typeId == QMetaType::Void) {
        type.typeId = Unknown;
      }
    }
    type.name = name;

    if (type.typeId == PythonQtMethodInfo::Unknown || type.typeId >= QMetaType::User) {
      bool isLocalEnum;
      // TODOXXX: make use of this flag!
      type.enumWrapper = PythonQtClassInfo::findEnumWrapper(type.name, classInfo, &isLocalEnum);
    }
  } else {
    type.typeId = QMetaType::Void;
    type.pointerCount = 0;
    type.isConst = false;
  }
}

int PythonQtMethodInfo::nameToType(const char* name)
{
  if (_parameterTypeDict.isEmpty()) {
    // we could also use QMetaType::nameToType, but that does a string compare search
    // and does not support QVariant

    // QMetaType names
    _parameterTypeDict.insert("long", QMetaType::Long);
    _parameterTypeDict.insert("int", QMetaType::Int);
    _parameterTypeDict.insert("short", QMetaType::Short);
    _parameterTypeDict.insert("char", QMetaType::Char);
    _parameterTypeDict.insert("ulong", QMetaType::ULong);
    _parameterTypeDict.insert("unsigned long", QMetaType::ULong);
    _parameterTypeDict.insert("uint", QMetaType::UInt);
    _parameterTypeDict.insert("unsigned int", QMetaType::UInt);
    _parameterTypeDict.insert("ushort", QMetaType::UShort);
    _parameterTypeDict.insert("unsigned short", QMetaType::UShort);
    _parameterTypeDict.insert("uchar", QMetaType::UChar);
    _parameterTypeDict.insert("unsigned char", QMetaType::UChar);
    _parameterTypeDict.insert("bool", QMetaType::Bool);
    _parameterTypeDict.insert("float", QMetaType::Float);
    _parameterTypeDict.insert("double", QMetaType::Double);
    _parameterTypeDict.insert("qreal", QMetaType::Double);
    _parameterTypeDict.insert("QChar", QMetaType::QChar);
    _parameterTypeDict.insert("QByteArray", QMetaType::QByteArray);
    _parameterTypeDict.insert("Q3CString", QMetaType::QByteArray);
    _parameterTypeDict.insert("QString", QMetaType::QString);
    _parameterTypeDict.insert("", QMetaType::Void);
    _parameterTypeDict.insert("void", QMetaType::Void);
    // QVariant names
    _parameterTypeDict.insert("Q_LLONG", QMetaType::LongLong);
    _parameterTypeDict.insert("Q_ULLONG", QMetaType::ULongLong);
    _parameterTypeDict.insert("qlonglong", QMetaType::LongLong);
    _parameterTypeDict.insert("qulonglong", QMetaType::ULongLong);
    _parameterTypeDict.insert("qint64", QMetaType::LongLong);
    _parameterTypeDict.insert("quint64", QMetaType::ULongLong);
    _parameterTypeDict.insert("QIconSet", QMetaType::QIcon);
    _parameterTypeDict.insert("QVariantMap", QMetaType::QVariantMap);
    _parameterTypeDict.insert("QVariantList", QMetaType::QVariantList);
    _parameterTypeDict.insert("QMap<QString,QVariant>", QMetaType::QVariantMap);
    _parameterTypeDict.insert("QList<QVariant>", QMetaType::QVariantList);
    _parameterTypeDict.insert("QStringList", QMetaType::QStringList);
    _parameterTypeDict.insert("QBitArray", QMetaType::QBitArray);
    _parameterTypeDict.insert("QDate", QMetaType::QDate);
    _parameterTypeDict.insert("QTime", QMetaType::QTime);
    _parameterTypeDict.insert("QDateTime", QMetaType::QDateTime);
    _parameterTypeDict.insert("QUrl", QMetaType::QUrl);
    _parameterTypeDict.insert("QLocale", QMetaType::QLocale);
    _parameterTypeDict.insert("QRect", QMetaType::QRect);
    _parameterTypeDict.insert("QRectf", QMetaType::QRectF);
    _parameterTypeDict.insert("QSize", QMetaType::QSize);
    _parameterTypeDict.insert("QSizef", QMetaType::QSizeF);
    _parameterTypeDict.insert("QLine", QMetaType::QLine);
    _parameterTypeDict.insert("QLinef", QMetaType::QLineF);
    _parameterTypeDict.insert("QPoint", QMetaType::QPoint);
    _parameterTypeDict.insert("QPointf", QMetaType::QPointF);
    _parameterTypeDict.insert("QRegExp", QMetaType::QRegExp);
//    _parameterTypeDict.insert("QColorGroup", QMetaType::QColorGroup);
    _parameterTypeDict.insert("QFont", QMetaType::QFont);
    _parameterTypeDict.insert("QPixmap", QMetaType::QPixmap);
    _parameterTypeDict.insert("QBrush", QMetaType::QBrush);
    _parameterTypeDict.insert("QColor", QMetaType::QColor);
    _parameterTypeDict.insert("QCursor", QMetaType::QCursor);
    _parameterTypeDict.insert("QPalette", QMetaType::QPalette);
    _parameterTypeDict.insert("QIcon", QMetaType::QIcon);
    _parameterTypeDict.insert("QImage", QMetaType::QPolygon);
    _parameterTypeDict.insert("QRegion", QMetaType::QRegion);
    _parameterTypeDict.insert("QBitmap", QMetaType::QBitmap);
    _parameterTypeDict.insert("QSizePolicy", QMetaType::QSizePolicy);
    _parameterTypeDict.insert("QKeySequence", QMetaType::QKeySequence);
    _parameterTypeDict.insert("QPen", QMetaType::QPen);
    _parameterTypeDict.insert("QTextLength", QMetaType::QTextLength);
    _parameterTypeDict.insert("QTextFormat", QMetaType::QTextFormat);
    _parameterTypeDict.insert("QMatrix", QMetaType::QMatrix);
    _parameterTypeDict.insert("QVariant", PythonQtMethodInfo::Variant);
    // own special types... (none so far, could be e.g. ObjectList
  }
  QHash<QByteArray, int>::const_iterator it = _parameterTypeDict.find(name);
  if (it!=_parameterTypeDict.end()) {
    return it.value();
  } else {
    return PythonQtMethodInfo::Unknown;
  }
}

void PythonQtMethodInfo::cleanupCachedMethodInfos()
{
  QHashIterator<QByteArray, PythonQtMethodInfo *> i(_cachedSignatures);
  while (i.hasNext()) {
    delete i.next().value();
  }
}

void PythonQtMethodInfo::addParameterTypeAlias(const QByteArray& alias, const QByteArray& name)
{
  _parameterNameAliases.insert(alias, name);
}

//-------------------------------------------------------------------------------------------------

void PythonQtSlotInfo::deleteOverloadsAndThis()
{
  PythonQtSlotInfo* cur = this;
  while(cur->nextInfo()) {
    PythonQtSlotInfo* next = cur->nextInfo();
    delete cur;
    cur = next;
  }
}


QString PythonQtSlotInfo::fullSignature()
{ 
  bool skipFirstArg = isInstanceDecorator();
  QString result = _meta.typeName();
  QByteArray sig = slotName();
  QList<QByteArray> names = _meta.parameterNames();

  bool isStatic = false;
  bool isConstructor = false;
  bool isDestructor = false;

  if (_type == ClassDecorator) {
    if (sig.startsWith("new_")) {
      sig = sig.mid(strlen("new_"));
      isConstructor = true;
    } else if (sig.startsWith("delete_")) {
      sig = sig.mid(strlen("delete_"));
      isDestructor = true;
    } else if(sig.startsWith("static_")) {
      isStatic = true;
      sig = sig.mid(strlen("static_"));
      int idx = sig.indexOf("_");
      if (idx>=0) {
        sig = sig.mid(idx+1);
      }
    }
  }

  result += QByteArray(" ") + sig;
  result += "(";

  int lastEntry = _parameters.count()-1;
  for (int i = skipFirstArg?2:1; i<_parameters.count(); i++) {
    if (_parameters.at(i).isConst) {
      result += "const ";
    }
    result += _parameters.at(i).name;
    if (_parameters.at(i).pointerCount) {
      QByteArray stars;
      stars.fill('*', _parameters.at(i).pointerCount);
      result += stars;
    }
    if (!names.at(i-1).isEmpty()) {
      result += " ";
      result += names.at(i-1);
    }
    if (i!=lastEntry) {
      result += ", ";
    }
  }
  result += ")";

  if (isStatic) {
    result = QString("static ") + result;
  } 
  if (isConstructor) {
//    result = QString("constructor ") + result;
  } 
  if (isDestructor) {
    result = QString("~") + result;
  } 
  return result;
}


QByteArray PythonQtSlotInfo::slotName()
{
  QByteArray sig(_meta.signature());
  int idx = sig.indexOf('(');
  sig = sig.left(idx);
  return sig;
}

