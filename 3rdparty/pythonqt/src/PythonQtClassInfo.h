#ifndef _PYTHONQTCLASSINFO_H
#define _PYTHONQTCLASSINFO_H

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

#include <QMetaObject>
#include <QMetaMethod>
#include <QHash>
#include <QByteArray>
#include <QList>
#include "PythonQt.h"

class PythonQtSlotInfo;

struct PythonQtMemberInfo {
  enum Type {
    Invalid, Slot, EnumValue, EnumWrapper, Property, NotFound 
  };

  PythonQtMemberInfo():_type(Invalid),_slot(NULL),_enumWrapper(NULL),_enumValue(0) { }
  
  PythonQtMemberInfo(PythonQtSlotInfo* info) {
    _type = Slot;
    _slot = info;
    _enumValue = NULL;
  }

  PythonQtMemberInfo(const PythonQtObjectPtr& enumValue) {
    _type = EnumValue;
    _slot = NULL;
    _enumValue = enumValue;
    _enumWrapper = NULL;
  }

  PythonQtMemberInfo(const QMetaProperty& prop) {
    _type = Property;
    _slot = NULL;
    _enumValue = NULL;
    _property = prop;
    _enumWrapper = NULL;
  }

  Type              _type;

  // TODO: this could be a union...
  PythonQtSlotInfo* _slot;
  PyObject*         _enumWrapper;
  PythonQtObjectPtr _enumValue;
  QMetaProperty     _property;
};

//! a class that stores all required information about a Qt object (and an optional associated C++ class name)
/*! for fast lookup of slots when calling the object from Python
*/
class PYTHONQT_EXPORT PythonQtClassInfo {

public:
  PythonQtClassInfo();
  ~PythonQtClassInfo();

  //! store information about parent classes
  struct ParentClassInfo {
    ParentClassInfo(PythonQtClassInfo* parent, int upcastingOffset=0):_parent(parent),_upcastingOffset(upcastingOffset)
    {};

    PythonQtClassInfo* _parent;
    int                _upcastingOffset;
  };


  //! setup as a QObject, taking the meta object as meta information about the QObject
  void setupQObject(const QMetaObject* meta);

  //! setup as a CPP (non-QObject), taking the classname
  void setupCPPObject(const QByteArray& classname);

  //! set the type capabilities
  void setTypeSlots(int typeSlots) { _typeSlots = typeSlots; }
  //! get the type capabilities
  int typeSlots() const { return _typeSlots; }

  //! get the Python method definition for a given slot name (without return type and signature)
  PythonQtMemberInfo member(const char* member);

  //! get access to the constructor slot (which may be overloaded if there are multiple constructors)
  PythonQtSlotInfo* constructors();
  
  //! get access to the destructor slot
  PythonQtSlotInfo* destructor();

  //! add a constructor, ownership is passed to classinfo
  void addConstructor(PythonQtSlotInfo* info);

  //! set a destructor, ownership is passed to classinfo
  void setDestructor(PythonQtSlotInfo* info);

  //! add a decorator slot, ownership is passed to classinfo
  void addDecoratorSlot(PythonQtSlotInfo* info);

  //! get the classname (either of the QObject or of the wrapped CPP object)
  const char* className();

  //! returns if the QObject
  bool isQObject() { return _isQObject; }

  //! returns if the class is a CPP wrapper
  bool isCPPWrapper() { return !_isQObject; }

  //! get the meta object
  const QMetaObject* metaObject() { return _meta; }

  //! set the meta object, this will reset the caching
  void setMetaObject(const QMetaObject* meta);

  //! returns if this class inherits from the given classname
  bool inherits(const char* classname);
  
  //! returns if this class inherits from the given classinfo
  bool inherits(PythonQtClassInfo* info);

  //! casts the given \c ptr to an object of type \c classname, returns the new pointer
  //! which might be different to \c ptr due to C++ multiple inheritance
  //! (if the cast is not possible or if ptr is NULL, NULL is returned)
  void* castTo(void* ptr, const char* classname);

  //! get help string for the metaobject
  QString help();

  //! get list of all properties (on QObjects only, otherwise the list is empty)
  QStringList propertyList();

  //! get list of all members
  QStringList memberList(bool metaOnly = false);

  //! get the meta type id of this class (only valid for isCPPWrapper() == true)
  int metaTypeId() { return _metaTypeId; }

  //! set an additional decorator provider that offers additional decorator slots for this class 
  void setDecoratorProvider(PythonQtQObjectCreatorFunctionCB* cb) { _decoratorProviderCB = cb; _decoratorProvider = NULL; }

  //! get the decorator qobject instance
  QObject* decorator();
  
  //! add the parent class info of a CPP object
  void addParentClass(const ParentClassInfo& info) { _parentClasses.append(info); }

  //! check if the special method "py_hasOwner" is implemented and if it returns false, which means that the object may be destroyed
  bool hasOwnerMethodButNoOwner(void* object);

  //! set the associated PythonQtClassWrapper (which handles instance creation of this type)
  void setPythonQtClassWrapper(PyObject* obj) { _pythonQtClassWrapper = obj; }

  //! get the associated PythonQtClassWrapper (which handles instance creation of this type)
  PyObject* pythonQtClassWrapper() { return _pythonQtClassWrapper; }

  //! set the shell set instance wrapper cb
  void setShellSetInstanceWrapperCB(PythonQtShellSetInstanceWrapperCB* cb) {
    _shellSetInstanceWrapperCB = cb;
  }

  //! get the shell set instance wrapper cb
  PythonQtShellSetInstanceWrapperCB* shellSetInstanceWrapperCB() {
    return _shellSetInstanceWrapperCB;
  }

  //! add a handler for polymorphic downcasting
  void addPolymorphicHandler(PythonQtPolymorphicHandlerCB* cb) { _polymorphicHandlers.append(cb); }

  //! cast the pointer down in the class hierarchy if a polymorphic handler allows to do that
  void* castDownIfPossible(void* ptr, PythonQtClassInfo** resultClassInfo);

  //! returns if the localScope has an enum of that type name or if the enum contains a :: scope, if that class contails the enum
  static PyObject* findEnumWrapper(const QByteArray& name, PythonQtClassInfo* localScope, bool* isLocalEnum = NULL);
  
private:
  void createEnumWrappers();
  void createEnumWrappers(const QMetaObject* meta);
  PyObject* findEnumWrapper(const char* name);

  //! clear all cached members
  void clearCachedMembers();

  void* recursiveCastDownIfPossible(void* ptr, char** resultClassName);

  PythonQtSlotInfo* findDecoratorSlotsFromDecoratorProvider(const char* memberName, PythonQtSlotInfo* inputInfo, bool &found, QHash<QByteArray, PythonQtMemberInfo>& memberCache, int upcastingOffset);
  void listDecoratorSlotsFromDecoratorProvider(QStringList& list, bool metaOnly);
  PythonQtSlotInfo* recursiveFindDecoratorSlotsFromDecoratorProvider(const char* memberName, PythonQtSlotInfo* inputInfo, bool &found, QHash<QByteArray, PythonQtMemberInfo>& memberCache, int upcastingOffset);

  void recursiveCollectClassInfos(QList<PythonQtClassInfo*>& classInfoObjects);
  void recursiveCollectDecoratorObjects(QList<QObject*>& decoratorObjects);

  bool lookForPropertyAndCache(const char* memberName);
  bool lookForMethodAndCache(const char* memberName);
  bool lookForEnumAndCache(const QMetaObject* m, const char* memberName);

  PythonQtSlotInfo* findDecoratorSlots(const char* memberName, int memberNameLen, PythonQtSlotInfo* tail, bool &found, QHash<QByteArray, PythonQtMemberInfo>& memberCache, int upcastingOffset);
  int findCharOffset(const char* sigStart, char someChar);

  QHash<QByteArray, PythonQtMemberInfo> _cachedMembers;

  PythonQtSlotInfo*                    _constructors;
  PythonQtSlotInfo*                    _destructor;
  QList<PythonQtSlotInfo*>             _decoratorSlots;

  QList<PythonQtObjectPtr>             _enumWrappers;

  const QMetaObject*                   _meta;

  QByteArray                           _wrappedClassName;
  QList<ParentClassInfo>               _parentClasses;

  QList<PythonQtPolymorphicHandlerCB*> _polymorphicHandlers;

  QObject*                             _decoratorProvider;
  PythonQtQObjectCreatorFunctionCB*    _decoratorProviderCB;
  
  PyObject*                            _pythonQtClassWrapper;
  
  PythonQtShellSetInstanceWrapperCB*   _shellSetInstanceWrapperCB;
  
  int                                  _metaTypeId;
  int                                  _typeSlots;

  bool                                 _isQObject;
  bool                                 _enumsCreated;
  
};

//---------------------------------------------------------------


#endif
