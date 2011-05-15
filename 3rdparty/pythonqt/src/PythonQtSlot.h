#ifndef _PYTHONQTSLOT_H
#define _PYTHONQTSLOT_H

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
// \file    PythonQtSlot.h
// \author  Florian Link
// \author  Last changed by $Author: florian $
// \date    2006-05
*/
//----------------------------------------------------------------------------------

#include "Python.h"
#include "PythonQtSystem.h"
#include "structmember.h"

class PythonQtSlotInfo;

extern PYTHONQT_EXPORT PyTypeObject PythonQtSlotFunction_Type;

#define PythonQtSlotFunction_Check(op) ((op)->ob_type == &PythonQtSlotFunction_Type)

PythonQtSlotInfo* PythonQtSlotFunction_GetSlotInfo(PyObject *);
PyObject* PythonQtSlotFunction_GetSelf(PyObject *);

/* Macros for direct access to these values. Type checks are *not*
   done, so use with care. */
#define PythonQtSlotFunction_GET_SELF(func) \
  (((PythonQtSlotFunctionObject *)func) -> m_self)

PyObject* PythonQtSlotFunction_Call(PyObject *, PyObject *, PyObject *);

PyObject *PythonQtSlotFunction_CallImpl(PythonQtClassInfo* classInfo, QObject* objectToCall, PythonQtSlotInfo* info, PyObject *args, PyObject *kw, void* firstArg=NULL,  void** directReturnValuePointer=NULL);


PyObject* PythonQtSlotFunction_New(PythonQtSlotInfo *, PyObject *,
           PyObject *);

//! defines a python object that stores a Qt slot info
typedef struct {
    PyObject_HEAD
    PythonQtSlotInfo *m_ml; /* Description of the C function to call */
    PyObject    *m_self; /* Passed as 'self' arg to the C func, can be NULL */
    PyObject    *m_module; /* The __module__ attribute, can be anything */
} PythonQtSlotFunctionObject;


#endif
