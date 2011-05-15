#ifndef _PYTHONQTCLASSWRAPPER_H
#define _PYTHONQTCLASSWRAPPER_H

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
// \file    PythonQtClassWrapper.h
// \author  Florian Link
// \author  Last changed by $Author: florian $
// \date    2006-05
*/
//----------------------------------------------------------------------------------

#include <Python.h>

#include "PythonQtSystem.h"

#include "structmember.h"
#include "methodobject.h"
#include "compile.h"
#include "eval.h"
#include <QString>

class PythonQtClassInfo;

//! the type of the PythonQt class wrapper objects
extern PYTHONQT_EXPORT PyTypeObject PythonQtClassWrapper_Type;

//---------------------------------------------------------------
//! a Python wrapper object for PythonQt wrapped classes
//! which inherits from the Python type object to allow
//! deriving of wrapped CPP classes from Python.
typedef struct {
  PyHeapTypeObject _base;

  //! the additional class information that PythonQt stores for the CPP class
  PythonQtClassInfo* _classInfo;

  //! get the class info
  PythonQtClassInfo* classInfo() { return _classInfo; }

} PythonQtClassWrapper;

//---------------------------------------------------------------

#endif
