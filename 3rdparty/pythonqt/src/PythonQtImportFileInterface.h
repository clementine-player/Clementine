#ifndef _PYTHONQTIMPORTFILEINTERFACE_H
#define _PYTHONQTIMPORTFILEINTERFACE_H

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
// \file    PythonQtImportFileInterface.h
// \author  Florian Link
// \author  Last changed by $Author: florian $
// \date    2006-05
*/
//----------------------------------------------------------------------------------

#include <QDateTime>
#include <QString>
#include <QByteArray>

//! Defines an abstract interface to file access for the Python import statement. 
//! see PythonQt::setImporter()
class PythonQtImportFileInterface {

public:
  // get rid of warnings
  virtual ~PythonQtImportFileInterface() {}

  //! read the given file as byte array, without doing any linefeed translations
  virtual QByteArray readFileAsBytes(const QString& filename) = 0;

  //! read a source file, expects a readable Python text file with translated line feeds.
  //! If the file can not be load OR it can not be verified, ok is set to false
  virtual QByteArray readSourceFile(const QString& filename, bool& ok) = 0;

  //! returns if the file exists
  virtual bool exists(const QString& filename) = 0;

  //! get the last modified data of a file
  virtual QDateTime lastModifiedDate(const QString& filename) = 0;

};

#endif