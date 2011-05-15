#ifndef _PYTHONQTSIGNALRECEIVER_H
#define _PYTHONQTSIGNALRECEIVER_H

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
// \file    PythonQtSignalReceiver.h
// \author  Florian Link
// \author  Last changed by $Author: florian $
// \date    2006-05
*/
//----------------------------------------------------------------------------------

#include <Python.h>
#include "PythonQtSystem.h"
#include "PythonQtObjectPtr.h"

class PythonQtMethodInfo;
class PythonQtClassInfo;

//! stores information about a signal target
/*! copy construction and assignment works fine with the C++ standard behaviour and are thus not implemented
*/
class PYTHONQT_EXPORT PythonQtSignalTarget {
public:
  PythonQtSignalTarget() {
    _signalId = -1;
    _methodInfo = NULL;
    _slotId = -1;
  }

  PythonQtSignalTarget(int signalId,const PythonQtMethodInfo* methodInfo, int slotId, PyObject* callable)
  {
    _signalId = signalId;
    _slotId = slotId;
    _methodInfo = methodInfo;
    _callable = callable;
  };

  ~PythonQtSignalTarget() {
  };

  //! get the id of the original signal
  int signalId() const { return _signalId; }

  //! get the id that was assigned to this simulated slot
  int slotId()  const { return _slotId; }

  //! get the signals parameter info
  const PythonQtMethodInfo* methodInfo()  const { return _methodInfo; }

  //! call the python callable with the given arguments (as defined in methodInfo)
  void call(void **arguments) const;

  //! check if it is the same signal target
  bool isSame(int signalId, PyObject* callable) const { return callable==_callable && signalId==_signalId; }

  //! call the given callable with arguments described by PythonQtMethodInfo, returns a new reference as result value (or NULL)
  static PyObject* call(PyObject* callable, const PythonQtMethodInfo* methodInfo, void **arguments, bool skipFirstArgumentOfMethodInfo = false);

private:
  int       _signalId;
  int       _slotId;
  const PythonQtMethodInfo* _methodInfo;
  PythonQtObjectPtr _callable;
};

//! base class for signal receivers
/*!
*/
class PythonQtSignalReceiverBase : public QObject {
  Q_OBJECT
public:
  PythonQtSignalReceiverBase(QObject* obj):QObject(obj) {};
};

//! receives all signals for one QObject
/*! we derive from our base but do not declare the QObject macro because we want to reimplement qt_metacall only.
*/
class PythonQtSignalReceiver : public PythonQtSignalReceiverBase {

public:
  PythonQtSignalReceiver(QObject* obj);
  ~PythonQtSignalReceiver();

  //! add a signal handler
  bool addSignalHandler(const char* signal, PyObject* callable);

  //! remove a signal handler
  bool removeSignalHandler(const char* signal, PyObject* callable);

  //! remove all signal handlers
  void removeSignalHandlers();

  //! we implement this method to simulate a number of slots that match the ids in _targets
  virtual int qt_metacall(QMetaObject::Call c, int id, void **arguments);

private:
  //! get the index of the signal
  int getSignalIndex(const char* signal);

  QObject* _obj;
  PythonQtClassInfo* _objClassInfo;
  int _slotCount;
  // linear list may get slow on multiple targets, but I think typically we have many objects and just a few signals
  QList<PythonQtSignalTarget> _targets;
};


#endif
