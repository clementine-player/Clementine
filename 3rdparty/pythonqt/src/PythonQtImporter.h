#ifndef _PYTHONQTIMPORTER_
#define _PYTHONQTIMPORTER_

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
// \file    PythonQtImporter.h
// \author  Florian Link
// \author  Last changed by $Author: stk $
// \date    2004-06
*/
//----------------------------------------------------------------------------------

#include "Python.h"
#include "structmember.h"
#include "osdefs.h"
#include "marshal.h"
#include "compile.h"
#include <time.h>

#include <qobject.h>
#include <qstring.h>


//! defines a python object that stores a Qt slot info
typedef struct _PythonQtImporter {
  PyObject_HEAD
  QString* _path;
} PythonQtImporter;


//! implements importing of python files into PythonQt
/*! also compiles/marshalls/unmarshalls py/pyc files and handles time stamps correctly
*/
class PythonQtImport
{
public:

  enum ModuleType {
    MI_NOT_FOUND,
    MI_MODULE,
    MI_PACKAGE,
    MI_SHAREDLIBRARY
  };

  struct ModuleInfo {
    ModuleInfo() {
      type  = MI_NOT_FOUND;
    }
    QString    fullPath;   //!< the full path to the found file
    QString    moduleName; //!< the module name without the package prefix
    ModuleType type;
  };

  //! initialize
  static void init();

  //! writes the python code to disk, marshalling and writing the time stamp
  static void writeCompiledModule(PyCodeObject *co, const QString& filename, long mtime);

  /*! Given the contents of a .py[co] file in a buffer, unmarshal the data
   and return the code object. Return None if it the magic word doesn't
   match (we do this instead of raising an exception as we fall back
   to .py if available and we don't want to mask other errors).
   Returns a new reference. */
  static PyObject *unmarshalCode(const QString& path, const QByteArray& data, time_t mtime);

  //! Given a string buffer containing Python source code, compile it
  //! return and return a code object as a new reference.
  static PyObject *compileSource(const QString& path, const QByteArray& data);

  //! Return the code object for the module named by 'fullname' from the
  //! Zip archive as a new reference.
  static PyObject *getCodeFromData(const QString& path, int isbytecode = 0, int ispackage = 0,
                                      time_t mtime = 0);

  //! Get the code object associated with the module specified by
  //! 'fullname'.
  static PyObject * getModuleCode(PythonQtImporter *self,
                                  const char* fullname, QString& modpath);


  //! gets the compiled code for the given *.py file if there is a valid pyc file, otherwise compiles the file and writes the pyc
  static PyObject* getCodeFromPyc(const QString& file);

  //! Return if module exists and is a package or a module
  static ModuleInfo getModuleInfo(PythonQtImporter* self, const QString& fullname);

  //! get the last name of a dot chain (first.second.last)
  static QString getSubName(const QString& str);

  //! Given a buffer, return the long that is represented by the first
  //!   4 bytes, encoded as little endian. This partially reimplements
  //!   marshal.c:r_long()
  static long getLong(unsigned char *buf);

  //! get time stamp of file
  static time_t getMTimeOfSource(const QString& path);

  //! replace extension of file
  static QString replaceExtension(const QString& str, const QString& ext);

};

#endif

