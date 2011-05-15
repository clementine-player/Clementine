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

#include "PythonQtClassInfo.h"
#include "PythonQtMethodInfo.h"
#include "PythonQt.h"
#include <QMetaMethod>
#include <QMetaObject>
#include <QMetaEnum>

QHash<QByteArray, int> PythonQtMethodInfo::_parameterTypeDict;

PythonQtClassInfo::PythonQtClassInfo() {
  _meta = NULL;
  _constructors = NULL;
  _destructor = NULL;
  _decoratorProvider = NULL;
  _decoratorProviderCB = NULL;
  _pythonQtClassWrapper = NULL;
  _shellSetInstanceWrapperCB = NULL;
  _metaTypeId = -1;
  _typeSlots = 0;
  _isQObject = false;
  _enumsCreated = false;
}

PythonQtClassInfo::~PythonQtClassInfo()
{
  clearCachedMembers();
  
  if (_constructors) {
    _constructors->deleteOverloadsAndThis();
  }
  if (_destructor) {
    _destructor->deleteOverloadsAndThis();
  }
  foreach(PythonQtSlotInfo* info, _decoratorSlots) {
    info->deleteOverloadsAndThis();
  }
}

void PythonQtClassInfo::setupQObject(const QMetaObject* meta)
{
  // _wrappedClassName is already set earlier in the class setup
  _isQObject = true;
  _meta = meta;
}

void PythonQtClassInfo::setupCPPObject(const QByteArray& classname)
{
  _isQObject = false;
  _wrappedClassName = classname;
  _metaTypeId = QMetaType::type(classname);
}

void PythonQtClassInfo::clearCachedMembers()
{
  QHashIterator<QByteArray, PythonQtMemberInfo> i(_cachedMembers);
  while (i.hasNext()) {
    PythonQtMemberInfo member = i.next().value();
    if (member._type== PythonQtMemberInfo::Slot) {
      PythonQtSlotInfo* info = member._slot;
      while (info) {
        PythonQtSlotInfo* next = info->nextInfo();
        delete info;
        info = next;
      }
    }
  }
}

int PythonQtClassInfo::findCharOffset(const char* sigStart, char someChar)
{
  const char* sigEnd = sigStart;
  char c;
  do {
    c = *sigEnd++;
  } while (c!=someChar && c!=0);
  return sigEnd-sigStart-1;
}
          
bool PythonQtClassInfo::lookForPropertyAndCache(const char* memberName)
{
  if (!_meta) return false;
  
  bool found = false;
  bool nameMapped = false;
  const char* attributeName = memberName;
  // look for properties
  int i = _meta->indexOfProperty(attributeName);
  if (i==-1) {
    // try to map name to objectName
    if (qstrcmp(attributeName, "name")==0) {
      attributeName = "objectName";
      nameMapped = true;
      i = _meta->indexOfProperty(attributeName);
    }
  }
  if (i!=-1) {
    PythonQtMemberInfo newInfo(_meta->property(i));
    _cachedMembers.insert(attributeName, newInfo);
    if (nameMapped) {
      _cachedMembers.insert(memberName, newInfo);
    }
  #ifdef PYTHONQT_DEBUG
    std::cout << "caching property " << memberName << " on " << _meta->className() << std::endl;
  #endif
    found = true;
  }
  return found;
}

PythonQtSlotInfo* PythonQtClassInfo::recursiveFindDecoratorSlotsFromDecoratorProvider(const char* memberName, PythonQtSlotInfo* inputInfo, bool &found, QHash<QByteArray, PythonQtMemberInfo>& memberCache, int upcastingOffset)
{
  inputInfo = findDecoratorSlotsFromDecoratorProvider(memberName, inputInfo, found, memberCache, upcastingOffset);
  foreach(const ParentClassInfo& info, _parentClasses) {
    inputInfo = info._parent->recursiveFindDecoratorSlotsFromDecoratorProvider(memberName, inputInfo, found, memberCache, upcastingOffset+info._upcastingOffset);
  }
  return inputInfo;
}

PythonQtSlotInfo* PythonQtClassInfo::findDecoratorSlotsFromDecoratorProvider(const char* memberName, PythonQtSlotInfo* tail, bool &found, QHash<QByteArray, PythonQtMemberInfo>& memberCache, int upcastingOffset) {
  QObject* decoratorProvider = decorator();
  int memberNameLen = strlen(memberName);
  if (decoratorProvider) {
    //qDebug()<< "looking " << decoratorProvider->metaObject()->className() << " " << memberName << " " << upcastingOffset;
    const QMetaObject* meta = decoratorProvider->metaObject();
    int numMethods = meta->methodCount();
    int startFrom = QObject::staticMetaObject.methodCount();
    for (int i = startFrom; i < numMethods; i++) {
      QMetaMethod m = meta->method(i);
      if ((m.methodType() == QMetaMethod::Method ||
           m.methodType() == QMetaMethod::Slot) && m.access() == QMetaMethod::Public) {
        
        const char* sigStart = m.signature();
        bool isClassDeco = false;
        if (qstrncmp(sigStart, "static_", 7)==0) {
          // skip the static_classname_ part of the string
          sigStart += 7 + 1 + strlen(className());
          isClassDeco = true;
        } else if (qstrncmp(sigStart, "new_", 4)==0) {
          isClassDeco = true;
        } else if (qstrncmp(sigStart, "delete_", 7)==0) {
          isClassDeco = true;
        }
        // find the first '('
        int offset = findCharOffset(sigStart, '(');

        // XXX no checking is currently done if the slots have correct first argument or not...
        
        // check if same length and same name
        if (memberNameLen == offset && qstrncmp(memberName, sigStart, offset)==0) {
          found = true;
          PythonQtSlotInfo* info = new PythonQtSlotInfo(this, m, i, decoratorProvider, isClassDeco?PythonQtSlotInfo::ClassDecorator:PythonQtSlotInfo::InstanceDecorator);
          info->setUpcastingOffset(upcastingOffset);
          //qDebug()<< "adding " << decoratorProvider->metaObject()->className() << " " << memberName << " " << upcastingOffset;
          if (tail) {
            tail->setNextInfo(info);
          } else {
            PythonQtMemberInfo newInfo(info);
            memberCache.insert(memberName, newInfo);
          }
          tail = info;
        }
      }
    }
  }

  tail = findDecoratorSlots(memberName, memberNameLen, tail, found, memberCache, upcastingOffset);
  
  return tail;
}

bool PythonQtClassInfo::lookForMethodAndCache(const char* memberName)
{
  bool found = false;
  int memberNameLen = strlen(memberName);
  PythonQtSlotInfo* tail = NULL;
  if (_meta) {
    int numMethods = _meta->methodCount();
    for (int i = 0; i < numMethods; i++) {
      QMetaMethod m = _meta->method(i);
      if (((m.methodType() == QMetaMethod::Method ||
           m.methodType() == QMetaMethod::Slot) && m.access() == QMetaMethod::Public)
        || m.methodType()==QMetaMethod::Signal) {
        
        const char* sigStart = m.signature();
        // find the first '('
        int offset = findCharOffset(sigStart, '(');
        
        // check if same length and same name
        if (memberNameLen == offset && qstrncmp(memberName, sigStart, offset)==0) {
          found = true;
          PythonQtSlotInfo* info = new PythonQtSlotInfo(this, m, i);
          if (tail) {
            tail->setNextInfo(info);
          } else {
            PythonQtMemberInfo newInfo(info);
            _cachedMembers.insert(memberName, newInfo);
          }
          tail = info;
        }
      }
    }
  }
  
  // look for dynamic decorators in this class and in derived classes
  tail = recursiveFindDecoratorSlotsFromDecoratorProvider(memberName, tail, found, _cachedMembers, 0);

  return found;
}

bool PythonQtClassInfo::lookForEnumAndCache(const QMetaObject* meta, const char* memberName)
{
  bool found = false;
  // look for enum values
  int enumCount = meta->enumeratorCount();
  for (int i=0;i<enumCount; i++) {
    QMetaEnum e = meta->enumerator(i);
    // we do not want flags, they will cause our values to appear two times
    if (e.isFlag()) continue;
    
    for (int j=0; j < e.keyCount(); j++) {
      if (qstrcmp(e.key(j), memberName)==0) {
        PyObject* enumType = findEnumWrapper(e.name());
        if (enumType) {
          PythonQtObjectPtr enumValuePtr;
          enumValuePtr.setNewRef(PythonQtPrivate::createEnumValueInstance(enumType, e.value(j)));
          PythonQtMemberInfo newInfo(enumValuePtr);
          _cachedMembers.insert(memberName, newInfo);
  #ifdef PYTHONQT_DEBUG
          std::cout << "caching enum " << memberName << " on " << meta->className() << std::endl;
  #endif
          found = true;
          break;
        } else {
          std::cout << "enum " << e.name() << " not found on " << className() << std::endl;
        }
      }
    }
  }
  return found;
}

PythonQtMemberInfo PythonQtClassInfo::member(const char* memberName)
{
  PythonQtMemberInfo info = _cachedMembers.value(memberName);
  if (info._type != PythonQtMemberInfo::Invalid) {
    return info;
  } else {
    bool found = false;
  
    found = lookForPropertyAndCache(memberName);
    if (!found) {
      found = lookForMethodAndCache(memberName);
    }
    if (!found) {
      if (_meta) {
        // check enums in our meta object directly
        found = lookForEnumAndCache(_meta, memberName);
      }
      if (!found) {
        // check enums in the class hierachy of CPP classes
        // look for dynamic decorators in this class and in derived classes
        QList<QObject*> decoObjects;
        recursiveCollectDecoratorObjects(decoObjects);
        foreach(QObject* deco, decoObjects) {
          // call on ourself for caching, but with different metaObject():
          found = lookForEnumAndCache(deco->metaObject(), memberName);
          if (found) {
            break;
          }
        }
      }
    }
    if (!found) {
      // maybe it is an enum wrapper?
      PyObject* p = findEnumWrapper(memberName);
      if (p) {
        info._type = PythonQtMemberInfo::EnumWrapper;
        info._enumWrapper = p;
        _cachedMembers.insert(memberName, info);
        found = true;
      }
    }
    if (!found) {
      // since python keywords can not be looked up, we check if the name contains a single trailing _
      // and remove that and look again, so that we e.g. find exec on an exec_ lookup
      QByteArray mbrName(memberName);
      if ((mbrName.length()>2) && 
          (mbrName.at(mbrName.length()-1) == '_') &&
          (mbrName.at(mbrName.length()-2) != '_')) {
        mbrName = mbrName.mid(0,mbrName.length()-1);
        found = lookForMethodAndCache(mbrName.constData());
        if (found) {
          return _cachedMembers.value(mbrName);
        }
      }
    }
    if (!found) {
      // we store a NotFound member, so that we get a quick result for non existing members (e.g. operator_equal lookup)
      info._type = PythonQtMemberInfo::NotFound;
      _cachedMembers.insert(memberName, info);
    }
  }

  return _cachedMembers.value(memberName);
}

void PythonQtClassInfo::recursiveCollectDecoratorObjects(QList<QObject*>& decoratorObjects) {
  QObject* deco = decorator();
  if (deco) {
    decoratorObjects.append(deco);
  }
  foreach(const ParentClassInfo& info, _parentClasses) {
    info._parent->recursiveCollectDecoratorObjects(decoratorObjects);
  }
}

void PythonQtClassInfo::recursiveCollectClassInfos(QList<PythonQtClassInfo*>& classInfoObjects) {
  classInfoObjects.append(this);
  foreach(const ParentClassInfo& info, _parentClasses) {
    info._parent->recursiveCollectClassInfos(classInfoObjects);
  }
}

PythonQtSlotInfo* PythonQtClassInfo::findDecoratorSlots(const char* memberName, int memberNameLen, PythonQtSlotInfo* tail, bool &found, QHash<QByteArray, PythonQtMemberInfo>& memberCache, int upcastingOffset)
{
  QListIterator<PythonQtSlotInfo*> it(_decoratorSlots);
  while (it.hasNext()) {

    PythonQtSlotInfo* infoOrig = it.next();
          
    const char* sigStart = infoOrig->metaMethod()->signature();
    if (qstrncmp("static_", sigStart, 7)==0) {
      sigStart += 7;
      sigStart += findCharOffset(sigStart, '_')+1;
    }
    int offset = findCharOffset(sigStart, '(');
    if (memberNameLen == offset && qstrncmp(memberName, sigStart, offset)==0) {
      //make a copy, otherwise we will have trouble on overloads!
      PythonQtSlotInfo* info = new PythonQtSlotInfo(*infoOrig);
      info->setUpcastingOffset(upcastingOffset);
      found = true;
      if (tail) {
        tail->setNextInfo(info);
      } else {
        PythonQtMemberInfo newInfo(info);
        memberCache.insert(memberName, newInfo);
      }
      tail = info;
    }
  }
  return tail;
}

void PythonQtClassInfo::listDecoratorSlotsFromDecoratorProvider(QStringList& list, bool metaOnly) {
  QObject* decoratorProvider = decorator();
  if (decoratorProvider) {
    const QMetaObject* meta = decoratorProvider->metaObject();
    int numMethods = meta->methodCount();
    int startFrom = QObject::staticMetaObject.methodCount();
    for (int i = startFrom; i < numMethods; i++) {
      QMetaMethod m = meta->method(i);
      if ((m.methodType() == QMetaMethod::Method ||
           m.methodType() == QMetaMethod::Slot) && m.access() == QMetaMethod::Public) {
        
        const char* sigStart = m.signature();
        bool isClassDeco = false;
        if (qstrncmp(sigStart, "static_", 7)==0) {
          // skip the static_classname_ part of the string
          sigStart += 7 + 1 + strlen(className());
          isClassDeco = true;
        } else if (qstrncmp(sigStart, "new_", 4)==0) {
          continue;
        } else if (qstrncmp(sigStart, "delete_", 7)==0) {
          continue;
        } else if (qstrncmp(sigStart, "py_", 3)==0) {
          // hide everything that starts with py_
          continue;
        }
        // find the first '('
        int offset = findCharOffset(sigStart, '(');
        
        // XXX no checking is currently done if the slots have correct first argument or not...
        if (!metaOnly || isClassDeco) {
          list << QString::fromLatin1(sigStart, offset); 
        }
      }
    }
  }

  // look for global decorator slots
  QListIterator<PythonQtSlotInfo*> it(_decoratorSlots);
  while (it.hasNext()) {
    PythonQtSlotInfo* slot = it.next();
    if (metaOnly) {
      if (slot->isClassDecorator()) {
        QByteArray first = slot->slotName();
        if (first.startsWith("static_")) {
          int idx = first.indexOf('_');
          idx = first.indexOf('_', idx+1);
          first = first.mid(idx+1);
        }
        list << first;
      }
    } else {
      list << slot->slotName();
    }
  }
}

QStringList PythonQtClassInfo::propertyList()
{
  QStringList l;
  if (_isQObject && _meta) {
    int i;
    int numProperties = _meta->propertyCount();
    for (i = 0; i < numProperties; i++) {
      QMetaProperty p = _meta->property(i);
      l << QString(p.name());
    }
  }
  return l;
}

QStringList PythonQtClassInfo::memberList(bool metaOnly)
{
  decorator();

  QStringList l;
  QString h;
  if (_isQObject && _meta && !metaOnly) {
    l = propertyList();
  }
  
  // normal slots of QObject (or wrapper QObject)
  if (!metaOnly && _meta) {
    int numMethods = _meta->methodCount();
    bool skipQObj = !_isQObject;
    for (int i = skipQObj?QObject::staticMetaObject.methodCount():0; i < numMethods; i++) {
      QMetaMethod m = _meta->method(i);
      if (((m.methodType() == QMetaMethod::Method ||
        m.methodType() == QMetaMethod::Slot) && m.access() == QMetaMethod::Public)
          || m.methodType()==QMetaMethod::Signal) {
        QByteArray signa(m.signature());
        signa = signa.left(signa.indexOf('('));
        l << signa;
      }
    }
  }

  {
    // look for dynamic decorators in this class and in derived classes
    QList<PythonQtClassInfo*> infos;
    recursiveCollectClassInfos(infos);
    foreach(PythonQtClassInfo* info, infos) {
      info->listDecoratorSlotsFromDecoratorProvider(l, metaOnly);
    }
  }
  
  // List enumerator keys...
  QList<const QMetaObject*> enumMetaObjects;
  if (_meta) {
    enumMetaObjects << _meta;
  }
  // check enums in the class hierachy of CPP classes
  QList<QObject*> decoObjects;
  recursiveCollectDecoratorObjects(decoObjects);
  foreach(QObject* deco, decoObjects) {
    enumMetaObjects << deco->metaObject();
  }
  
  foreach(const QMetaObject* meta, enumMetaObjects) {
    for (int i = 0; i<meta->enumeratorCount(); i++) {
      QMetaEnum e = meta->enumerator(i);
      l << e.name();
      // we do not want flags, they will cause our values to appear two times
      if (e.isFlag()) continue;

      for (int j=0; j < e.keyCount(); j++) {
        l << QString(e.key(j));
      }
    }
  }

  return QSet<QString>::fromList(l).toList();
}

const char* PythonQtClassInfo::className()
{
  return _wrappedClassName.constData();
}

void* PythonQtClassInfo::castTo(void* ptr, const char* classname)
{
  if (ptr==NULL) {
    return NULL;
  }
  if (_wrappedClassName == classname) {
    return ptr;
  }
  foreach(const ParentClassInfo& info, _parentClasses) {
    void* result = info._parent->castTo((char*)ptr + info._upcastingOffset, classname);
    if (result) {
      return result;
    }
  }
  return NULL;
}

bool PythonQtClassInfo::inherits(const char* name)
{
  if (_wrappedClassName == name) {
    return true;
  }
  foreach(const ParentClassInfo& info, _parentClasses) {
    if (info._parent->inherits(name)) {
      return true;
    }
  }
  return false;
}

bool PythonQtClassInfo::inherits(PythonQtClassInfo* classInfo)
{
  if (classInfo == this) {
    return true;
  }
  foreach(const ParentClassInfo& info, _parentClasses) {
    if (info._parent->inherits(classInfo)) {
      return true;
    }
  }
  return false;
}

QString PythonQtClassInfo::help()
{
  decorator();
  QString h;
  h += QString("--- ") + QString(className()) + QString(" ---\n");
  
  if (_isQObject) {
    h += "Properties:\n";
  
    int i;
    int numProperties = _meta->propertyCount();
    for (i = 0; i < numProperties; i++) {
      QMetaProperty p = _meta->property(i);
      h += QString(p.name()) + " (" + QString(p.typeName()) + " )\n";
    }
  }
  
  if (constructors()) {
    h += "Constructors:\n";
    PythonQtSlotInfo* constr = constructors();
    while (constr) {
      h += constr->fullSignature() + "\n";
      constr = constr->nextInfo();
    }
  }

  h += "Slots:\n";
  h += "QString help()\n";
  h += "QString className()\n";

  if (_meta) {
    int numMethods = _meta->methodCount();
    for (int i = 0; i < numMethods; i++) {
      QMetaMethod m = _meta->method(i);
      if ((m.methodType() == QMetaMethod::Method ||
        m.methodType() == QMetaMethod::Slot) && m.access() == QMetaMethod::Public) {
        PythonQtSlotInfo slot(this, m, i);
        h += slot.fullSignature()+ "\n";
      }
    }
  }
  
  // TODO xxx : decorators and enums from decorator() are missing...
  // maybe we can reuse memberlist()?
  
  if (_meta && _meta->enumeratorCount()) {
    h += "Enums:\n";
    for (int i = 0; i<_meta->enumeratorCount(); i++) {
      QMetaEnum e = _meta->enumerator(i);
      h += QString(e.name()) + " {";
      for (int j=0; j < e.keyCount(); j++) {
        if (j) { h+= ", "; }
        h += e.key(j);
      }
      h += " }\n";
    }
  }

  if (_isQObject && _meta) {
    int numMethods = _meta->methodCount();
    if (numMethods>0) {
      h += "Signals:\n";
      for (int i = 0; i < numMethods; i++) {
        QMetaMethod m = _meta->method(i);
        if (m.methodType() == QMetaMethod::Signal) {
          h += QString(m.signature()) + "\n";
        }
      }
    }
  }
  return h;
}

PythonQtSlotInfo* PythonQtClassInfo::constructors()
{
  if (!_constructors) {
    // force creation of lazy decorator, which will register the decorators
    decorator();
  }
  return _constructors;
}

PythonQtSlotInfo* PythonQtClassInfo::destructor()
{
  if (!_destructor) {
    // force creation of lazy decorator, which will register the decorators
    decorator();
  }
  return _destructor;
}

void PythonQtClassInfo::addConstructor(PythonQtSlotInfo* info)
{
  PythonQtSlotInfo* prev = constructors();
  if (prev) {
    info->setNextInfo(prev->nextInfo());
    prev->setNextInfo(info);
  } else {
    _constructors = info;
  }
}

void PythonQtClassInfo::addDecoratorSlot(PythonQtSlotInfo* info)
{
  _decoratorSlots.append(info);
}

void PythonQtClassInfo::setDestructor(PythonQtSlotInfo* info)
{
  if (_destructor) {
    _destructor->deleteOverloadsAndThis();
  }
  _destructor = info;
}

void PythonQtClassInfo::setMetaObject(const QMetaObject* meta)
{
  _meta = meta;
  clearCachedMembers();
}

QObject* PythonQtClassInfo::decorator()
{
  if (!_decoratorProvider && _decoratorProviderCB) {
    _decoratorProvider = (*_decoratorProviderCB)();
    if (_decoratorProvider) {
      _decoratorProvider->setParent(PythonQt::priv());
      // setup enums early, since they might be needed by the constructor decorators:
      if (!_enumsCreated) {
        createEnumWrappers();
      }
      PythonQt::priv()->addDecorators(_decoratorProvider, PythonQtPrivate::ConstructorDecorator | PythonQtPrivate::DestructorDecorator);
    }
  }
  // check if enums need to be created and create them if they are not yet created
  if (!_enumsCreated) {
    createEnumWrappers();
  }
  return _decoratorProvider;
}

bool PythonQtClassInfo::hasOwnerMethodButNoOwner(void* object)
{
  PythonQtMemberInfo info = member("py_hasOwner");
  if (info._type == PythonQtMemberInfo::Slot) {
    void* obj = object;
    bool result = false;
    void* args[2];
    args[0] = &result;
    args[1] = &obj;
    info._slot->decorator()->qt_metacall(QMetaObject::InvokeMetaMethod, info._slot->slotIndex(), args);
    return !result;
  } else {
    return false;
  }
}

void* PythonQtClassInfo::recursiveCastDownIfPossible(void* ptr, char** resultClassName)
{
  if (!_polymorphicHandlers.isEmpty()) {
    foreach(PythonQtPolymorphicHandlerCB* cb, _polymorphicHandlers) {
      void* resultPtr = (*cb)(ptr, resultClassName);
      if (resultPtr) {
        return resultPtr;
      }
    }
  }
  foreach(const ParentClassInfo& info, _parentClasses) {
    if (!info._parent->isQObject()) {
      void* resultPtr = info._parent->recursiveCastDownIfPossible((char*)ptr + info._upcastingOffset, resultClassName);
      if (resultPtr) {
        return resultPtr;
      }
    }
  }
  return NULL;
}

void* PythonQtClassInfo::castDownIfPossible(void* ptr, PythonQtClassInfo** resultClassInfo)
{
  char* className;
  // this would do downcasting recursively...
  // void* resultPtr = recursiveCastDownIfPossible(ptr, &className);

  // we only do downcasting on the base object, not on the whole inheritance tree...
  void* resultPtr = NULL;
  if (!_polymorphicHandlers.isEmpty()) {
    foreach(PythonQtPolymorphicHandlerCB* cb, _polymorphicHandlers) {
      resultPtr = (*cb)(ptr, &className);
      if (resultPtr) {
        break;
      }
    }
  }
  if (resultPtr) {
    *resultClassInfo = PythonQt::priv()->getClassInfo(className);
  } else {
    *resultClassInfo = this;
    resultPtr = ptr;
  }
  return resultPtr;
}

PyObject* PythonQtClassInfo::findEnumWrapper(const QByteArray& name, PythonQtClassInfo* localScope, bool* isLocalEnum)
{
  if (isLocalEnum) {
    *isLocalEnum = true;
  }
  int scopePos = name.lastIndexOf("::");
  if (scopePos != -1) {
    if (isLocalEnum) {
      *isLocalEnum = false;
    }
    // split into scope and enum name
    QByteArray enumScope = name.mid(0,scopePos);
    QByteArray enumName = name.mid(scopePos+2);
    PythonQtClassInfo* info = PythonQt::priv()->getClassInfo(enumScope);
    if (info) {
      return info->findEnumWrapper(enumName);
    } else{
      return NULL;
    }
  }
  if (localScope) {
    return localScope->findEnumWrapper(name);
  } else {
    return NULL;
  }
}

void PythonQtClassInfo::createEnumWrappers(const QMetaObject* meta)
{
  for (int i = meta->enumeratorOffset();i<meta->enumeratorCount();i++) {
    QMetaEnum e = meta->enumerator(i);
    PythonQtObjectPtr p;
    p.setNewRef(PythonQtPrivate::createNewPythonQtEnumWrapper(e.name(), _pythonQtClassWrapper));
    _enumWrappers.append(p);
  }
}

void PythonQtClassInfo::createEnumWrappers()
{
  if (!_enumsCreated) {
    _enumsCreated = true;
    if (_meta) {
      createEnumWrappers(_meta);
    }
    if (decorator()) {
      createEnumWrappers(decorator()->metaObject());
    }
    foreach(const ParentClassInfo& info, _parentClasses) {
      info._parent->createEnumWrappers();
    }
  }
}

PyObject* PythonQtClassInfo::findEnumWrapper(const char* name) {
  // force enum creation
  if (!_enumsCreated) {
    createEnumWrappers();
  }
  foreach(const PythonQtObjectPtr& p, _enumWrappers) {
    const char* className = ((PyTypeObject*)p.object())->tp_name;
    if (qstrcmp(className, name)==0) {
      return p.object();
    }
  }
  foreach(const ParentClassInfo& info, _parentClasses) {
    PyObject* p = info._parent->findEnumWrapper(name);
    if (p) return p;
  }
  return NULL;
}

