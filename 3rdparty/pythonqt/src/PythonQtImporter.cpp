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
// \author  Last changed by $Author: florian $
// \date    2006-05
*/
// This module was inspired by the zipimport.c module of the original
// Python distribution. Most of the functions are identical or slightly
// modified to do all the loading of Python files via an external file interface.
// In contrast to zipimport.c, this module also writes *.pyc files
// automatically if it has write access/is not inside of a zip file.
//----------------------------------------------------------------------------------

#include "PythonQtImporter.h"
#include "PythonQtImportFileInterface.h"
#include "PythonQt.h"
#include "PythonQtConversion.h"
#include <QFile>
#include <QFileInfo>

#define IS_SOURCE   0x0
#define IS_BYTECODE 0x1
#define IS_PACKAGE  0x2

struct st_mlab_searchorder {
  char suffix[14];
  int type;
};

/* mlab_searchorder defines how we search for a module in the Zip
   archive: we first search for a package __init__, then for
   non-package .pyc, .pyo and .py entries. The .pyc and .pyo entries
   are swapped by initmlabimport() if we run in optimized mode. Also,
   '/' is replaced by SEP there. */
 struct st_mlab_searchorder mlab_searchorder[] = {
  {"/__init__.pyc", IS_PACKAGE | IS_BYTECODE},
  {"/__init__.pyo", IS_PACKAGE | IS_BYTECODE},
  {"/__init__.py", IS_PACKAGE | IS_SOURCE},
  {".pyc", IS_BYTECODE},
  {".pyo", IS_BYTECODE},
  {".py", IS_SOURCE},
  {"", 0}
};

extern PyTypeObject PythonQtImporter_Type;
PyObject *PythonQtImportError;

QString PythonQtImport::getSubName(const QString& str)
{
  int idx = str.lastIndexOf('.');
  if (idx!=-1) {
    return str.mid(idx+1);
  } else {
    return str;
  }
}

PythonQtImport::ModuleInfo PythonQtImport::getModuleInfo(PythonQtImporter* self, const QString& fullname)
{
  ModuleInfo info;
  QString subname;
  struct st_mlab_searchorder *zso;

  subname = getSubName(fullname);
  QString path = *self->_path + "/" + subname;

  QString test;
  for (zso = mlab_searchorder; *zso->suffix; zso++) {
    test = path + zso->suffix;
    if (PythonQt::importInterface()->exists(test)) {
      info.fullPath = test;
      info.moduleName = subname;
      info.type = (zso->type & IS_PACKAGE)?MI_PACKAGE:MI_MODULE;
      return info;
    }
  }
  // test if it is a shared library
  foreach(const QString& suffix, PythonQt::priv()->sharedLibrarySuffixes()) {
    test = path+suffix;
    if (PythonQt::importInterface()->exists(test)) {
      info.fullPath = test;
      info.moduleName = subname;
      info.type = MI_SHAREDLIBRARY;
    }
  }
  return info;
}


/* PythonQtImporter.__init__
  Just store the path argument (or reject if it is in the ignorePaths list
*/
int PythonQtImporter_init(PythonQtImporter *self, PyObject *args, PyObject * /*kwds*/)
{
  self->_path = NULL;

  const char* cpath;
  if (!PyArg_ParseTuple(args, "s",
    &cpath))
    return -1;

  QString path(cpath);
  if (PythonQt::importInterface()->exists(path)) {
    const QStringList& ignorePaths = PythonQt::self()->getImporterIgnorePaths();
    foreach(QString ignorePath, ignorePaths) {
      if (path.startsWith(ignorePath)) {
        PyErr_SetString(PythonQtImportError,
          "path ignored");
        return -1;
      }
    }

    self->_path = new QString(path);
    return 0;
  } else {
    PyErr_SetString(PythonQtImportError,
        "path does not exist error");
    return -1;
  }
}

void
PythonQtImporter_dealloc(PythonQtImporter *self)
{
  // free the stored path
  if (self->_path) delete self->_path;
  // free ourself
  self->ob_type->tp_free((PyObject *)self);
}


/* Check whether we can satisfy the import of the module named by
   'fullname'. Return self if we can, None if we can't. */
PyObject *
PythonQtImporter_find_module(PyObject *obj, PyObject *args)
{
  PythonQtImporter *self = (PythonQtImporter *)obj;
  PyObject *path = NULL;
  char *fullname;

  if (!PyArg_ParseTuple(args, "s|O:PythonQtImporter.find_module",
            &fullname, &path))
    return NULL;

  //qDebug() << "looking for " << fullname << " at " << *self->_path;

  PythonQtImport::ModuleInfo info = PythonQtImport::getModuleInfo(self, fullname);
  if (info.type != PythonQtImport::MI_NOT_FOUND) {
    Py_INCREF(self);
    return (PyObject *)self;
  } else {
    Py_INCREF(Py_None);
    return Py_None;
  }
}

/* Load and return the module named by 'fullname'. */
PyObject *
PythonQtImporter_load_module(PyObject *obj, PyObject *args)
{
  PythonQtImporter *self = (PythonQtImporter *)obj;
  PyObject *code = NULL, *mod = NULL, *dict = NULL;
  char *fullname;

  if (!PyArg_ParseTuple(args, "s:PythonQtImporter.load_module",
            &fullname))
    return NULL;

  PythonQtImport::ModuleInfo info = PythonQtImport::getModuleInfo(self, fullname);
  if (info.type == PythonQtImport::MI_NOT_FOUND) {
    return NULL;
  }

  if (info.type == PythonQtImport::MI_PACKAGE || info.type == PythonQtImport::MI_MODULE) {
    QString fullPath;
    code = PythonQtImport::getModuleCode(self, fullname, fullPath);
    if (code == NULL) {
      return NULL;
    }

    mod = PyImport_AddModule(fullname);
    if (mod == NULL) {
      Py_DECREF(code);
      return NULL;
    }
    dict = PyModule_GetDict(mod);

    if (PyDict_SetItemString(dict, "__loader__", (PyObject *)self) != 0) {
      Py_DECREF(code);
      Py_DECREF(mod);
      return NULL;
    }

    if (info.type == PythonQtImport::MI_PACKAGE) {
      PyObject *pkgpath, *fullpath;
      QString subname = info.moduleName;
      int err;

      fullpath = PyString_FromFormat("%s%c%s",
        self->_path->toLatin1().constData(),
        SEP,
        subname.toLatin1().constData());
      if (fullpath == NULL) {
        Py_DECREF(code);
        Py_DECREF(mod);
        return NULL;
      }

      pkgpath = Py_BuildValue("[O]", fullpath);
      Py_DECREF(fullpath);
      if (pkgpath == NULL) {
        Py_DECREF(code);
        Py_DECREF(mod);
        return NULL;
      }
      err = PyDict_SetItemString(dict, "__path__", pkgpath);
      Py_DECREF(pkgpath);
      if (err != 0) {
        Py_DECREF(code);
        Py_DECREF(mod);
        return NULL;
      }
    }
    mod = PyImport_ExecCodeModuleEx(fullname, code, fullPath.toLatin1().data());
    Py_DECREF(code);
    if (Py_VerboseFlag) {
      PySys_WriteStderr("import %s # loaded from %s\n",
        fullname, fullPath.toLatin1().constData());
    }
  } else {
    PythonQtObjectPtr imp;
    imp.setNewRef(PyImport_ImportModule("imp"));
    
    // Create a PyList with the current path as its single element,
    // which is required for find_module (it won't accept a tuple...)
    PythonQtObjectPtr pathList;
    pathList.setNewRef(PythonQtConv::QStringListToPyList(QStringList() << *self->_path));

    QVariantList args;
    // Pass the module name without the package prefix
    args.append(info.moduleName);
    // And the path where we know that the shared library is
    args.append(QVariant::fromValue(pathList));
    QVariant result = imp.call("find_module", args);
    if (result.isValid()) {
      // This will return a tuple with (file, pathname, description)
      QVariantList list = result.toList();
      if (list.count()==3) {
        // We prepend the full module name (including package prefix)
        list.prepend(fullname);
        // And call "load_module" with (fullname, file, pathname, description)
        PythonQtObjectPtr module = imp.call("load_module", list);
        mod = module.object();
        if (mod) {
          Py_INCREF(mod);
        }

        // Finally, we need to close the file again, which find_module opened for us
        PythonQtObjectPtr file = list.at(1);
        file.call("close");
      }
    }
  }
  return mod;
}


PyObject *
PythonQtImporter_get_data(PyObject* /*obj*/, PyObject* /*args*/)
{
  // EXTRA, NOT YET IMPLEMENTED
  return NULL;
}

PyObject *
PythonQtImporter_get_code(PyObject *obj, PyObject *args)
{
  PythonQtImporter *self = (PythonQtImporter *)obj;
  char *fullname;

  if (!PyArg_ParseTuple(args, "s:PythonQtImporter.get_code", &fullname))
    return NULL;

  QString notused;
  return PythonQtImport::getModuleCode(self, fullname, notused);
}

PyObject *
PythonQtImporter_get_source(PyObject * /*obj*/, PyObject * /*args*/)
{
  // EXTRA, NOT YET IMPLEMENTED
  return NULL;
}

PyDoc_STRVAR(doc_find_module,
"find_module(fullname, path=None) -> self or None.\n\
\n\
Search for a module specified by 'fullname'. 'fullname' must be the\n\
fully qualified (dotted) module name. It returns the PythonQtImporter\n\
instance itself if the module was found, or None if it wasn't.\n\
The optional 'path' argument is ignored -- it's there for compatibility\n\
with the importer protocol.");

PyDoc_STRVAR(doc_load_module,
"load_module(fullname) -> module.\n\
\n\
Load the module specified by 'fullname'. 'fullname' must be the\n\
fully qualified (dotted) module name. It returns the imported\n\
module, or raises PythonQtImportError if it wasn't found.");

PyDoc_STRVAR(doc_get_data,
"get_data(pathname) -> string with file data.\n\
\n\
Return the data associated with 'pathname'. Raise IOError if\n\
the file wasn't found.");

PyDoc_STRVAR(doc_get_code,
"get_code(fullname) -> code object.\n\
\n\
Return the code object for the specified module. Raise PythonQtImportError\n\
is the module couldn't be found.");

PyDoc_STRVAR(doc_get_source,
"get_source(fullname) -> source string.\n\
\n\
Return the source code for the specified module. Raise PythonQtImportError\n\
is the module couldn't be found, return None if the archive does\n\
contain the module, but has no source for it.");

PyMethodDef PythonQtImporter_methods[] = {
  {"find_module", PythonQtImporter_find_module, METH_VARARGS,
   doc_find_module},
  {"load_module", PythonQtImporter_load_module, METH_VARARGS,
   doc_load_module},
  {"get_data", PythonQtImporter_get_data, METH_VARARGS,
   doc_get_data},
  {"get_code", PythonQtImporter_get_code, METH_VARARGS,
   doc_get_code},
  {"get_source", PythonQtImporter_get_source, METH_VARARGS,
   doc_get_source},
  {NULL,    NULL, 0 , NULL} /* sentinel */
};


PyDoc_STRVAR(PythonQtImporter_doc,
"PythonQtImporter(path) -> PythonQtImporter object\n\
\n\
Create a new PythonQtImporter instance. 'path' must be a valid path on disk/or inside of a zip file known to MeVisLab\n\
. Every path is accepted.");

#define DEFERRED_ADDRESS(ADDR) 0

PyTypeObject PythonQtImporter_Type = {
  PyObject_HEAD_INIT(DEFERRED_ADDRESS(&PyType_Type))
  0,
  "PythonQtImport.PythonQtImporter",
  sizeof(PythonQtImporter),
  0,          /* tp_itemsize */
  (destructor)PythonQtImporter_dealloc, /* tp_dealloc */
  0,          /* tp_print */
  0,          /* tp_getattr */
  0,          /* tp_setattr */
  0,          /* tp_compare */
  0,    /* tp_repr */
  0,          /* tp_as_number */
  0,          /* tp_as_sequence */
  0,          /* tp_as_mapping */
  0,          /* tp_hash */
  0,          /* tp_call */
  0,          /* tp_str */
  PyObject_GenericGetAttr,    /* tp_getattro */
  0,          /* tp_setattro */
  0,          /* tp_as_buffer */
  Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE ,    /* tp_flags */
  PythonQtImporter_doc,     /* tp_doc */
  0,      /* tp_traverse */
  0,          /* tp_clear */
  0,          /* tp_richcompare */
  0,          /* tp_weaklistoffset */
  0,          /* tp_iter */
  0,          /* tp_iternext */
  PythonQtImporter_methods,     /* tp_methods */
  0,          /* tp_members */
  0,          /* tp_getset */
  0,          /* tp_base */
  0,          /* tp_dict */
  0,          /* tp_descr_get */
  0,          /* tp_descr_set */
  0,          /* tp_dictoffset */
  (initproc)PythonQtImporter_init,    /* tp_init */
  PyType_GenericAlloc,      /* tp_alloc */
  PyType_GenericNew,      /* tp_new */
  PyObject_Del,     /* tp_free */
};


/* Given a buffer, return the long that is represented by the first
   4 bytes, encoded as little endian. This partially reimplements
   marshal.c:r_long() */
long
PythonQtImport::getLong(unsigned char *buf)
{
  long x;
  x =  buf[0];
  x |= (long)buf[1] <<  8;
  x |= (long)buf[2] << 16;
  x |= (long)buf[3] << 24;
#if SIZEOF_LONG > 4
  /* Sign extension for 64-bit machines */
  x |= -(x & 0x80000000L);
#endif
  return x;
}

void PythonQtImport::writeCompiledModule(PyCodeObject *co, const QString& filename, long mtime)
{
  // we do not want to write Qt resources to disk, do we?
  if (filename.startsWith(":")) {
    return;
  }

#if PY_VERSION_HEX < 0x02040000
  PyObject* data = PyMarshal_WriteObjectToString((PyObject*) co);
#else
  PyObject* data = PyMarshal_WriteObjectToString((PyObject*) co, Py_MARSHAL_VERSION);
#endif

  if (!data) {
    return;
  }

  QFile file(filename);
  if (!file.open(QIODevice::WriteOnly)) {
    Py_DECREF(data);
    return;
  }

  char* buffer = NULL;
  Py_ssize_t length = 0;
  PyString_AsStringAndSize(data, &buffer, &length);

  const quint32 magic_number = PyImport_GetMagicNumber();
  const quint32 mtime_32 = mtime;

  file.write(reinterpret_cast<const char*>(&magic_number), sizeof(magic_number));
  file.write(reinterpret_cast<const char*>(&mtime_32), sizeof(mtime_32));
  file.write(buffer, length);

  Py_DECREF(data);
}

/* Given the contents of a .py[co] file in a buffer, unmarshal the data
   and return the code object. Return None if it the magic word doesn't
   match (we do this instead of raising an exception as we fall back
   to .py if available and we don't want to mask other errors).
   Returns a new reference. */
PyObject *
PythonQtImport::unmarshalCode(const QString& path, const QByteArray& data, time_t mtime)
{
  PyObject *code;
  // ugly cast, but Python API is not const safe
  char *buf = (char*) data.constData();
  int size = data.size();

  if (size <= 9) {
    PySys_WriteStderr("# %s has bad pyc data\n",
            path.toLatin1().constData());
    Py_INCREF(Py_None);
    return Py_None;
  }

  if (getLong((unsigned char *)buf) != PyImport_GetMagicNumber()) {
    if (Py_VerboseFlag)
      PySys_WriteStderr("# %s has bad magic\n",
            path.toLatin1().constData());
    Py_INCREF(Py_None);
    return Py_None;
  }

  if (mtime != 0 && !(getLong((unsigned char *)buf + 4) == mtime)) {
    if (Py_VerboseFlag)
      PySys_WriteStderr("# %s has bad mtime\n",
            path.toLatin1().constData());
    Py_INCREF(Py_None);
    return Py_None;
  }

  code = PyMarshal_ReadObjectFromString(buf + 8, size - 8);
  if (code == NULL)
    return NULL;
  if (!PyCode_Check(code)) {
    Py_DECREF(code);
    PyErr_Format(PyExc_TypeError,
         "compiled module %.200s is not a code object",
         path.toLatin1().constData());
    return NULL;
  }
  return code;
}


/* Given a string buffer containing Python source code, compile it
   return and return a code object as a new reference. */
PyObject *
PythonQtImport::compileSource(const QString& path, const QByteArray& data)
{
  PyObject *code;
  QByteArray data1 = data;
// in qt4, data is null terminated
//  data1.resize(data.size()+1);
//  data1.data()[data.size()-1] = 0;
  code = Py_CompileString(data.data(), path.toLatin1().constData(),
        Py_file_input);
  return code;
}


/* Return the code object for the module named by 'fullname' from the
   Zip archive as a new reference. */
PyObject *
PythonQtImport::getCodeFromData(const QString& path, int isbytecode,int /*ispackage*/, time_t mtime)
{
  PyObject *code;

  QByteArray qdata;
    if (!isbytecode) {
      //    mlabDebugConst("MLABPython", "reading source " << path);
      bool ok;
      qdata = PythonQt::importInterface()->readSourceFile(path, ok);
      if (!ok) {
        //    mlabErrorConst("PythonQtImporter","File could not be verified" << path);
        return NULL;
      }
      if (qdata == " ") {
        qdata.clear();
      }
    } else {
      qdata = PythonQt::importInterface()->readFileAsBytes(path);
    }

  if (isbytecode) {
//    mlabDebugConst("MLABPython", "reading bytecode " << path);
    code = unmarshalCode(path, qdata, mtime);
  }
  else {
  //  mlabDebugConst("MLABPython", "compiling source " << path);
    code = compileSource(path, qdata);
    if (code) {
      // save a pyc file if possible
      QDateTime time;
      time = PythonQt::importInterface()->lastModifiedDate(path);
      writeCompiledModule((PyCodeObject*)code, path+"c", time.toTime_t());
    }
  }
  return code;
}

time_t
PythonQtImport::getMTimeOfSource(const QString& path)
{
  time_t mtime = 0;
  QString path2 = path;
  path2.truncate(path.length()-1);

  if (PythonQt::importInterface()->exists(path2)) {
    mtime = PythonQt::importInterface()->lastModifiedDate(path2).toTime_t();
  }

  return mtime;
}

/* Get the code object associated with the module specified by
   'fullname'. */
PyObject *
PythonQtImport::getModuleCode(PythonQtImporter *self, const char* fullname, QString& modpath)
{
  QString subname;
  struct st_mlab_searchorder *zso;

  subname = getSubName(fullname);
  QString path = *self->_path + "/" + subname;

  QString test;
  for (zso = mlab_searchorder; *zso->suffix; zso++) {
    PyObject *code = NULL;
    test = path + zso->suffix;

    if (Py_VerboseFlag > 1)
      PySys_WriteStderr("# trying %s\n",
            test.toLatin1().constData());
    if (PythonQt::importInterface()->exists(test)) {
      time_t mtime = 0;
      int ispackage = zso->type & IS_PACKAGE;
      int isbytecode = zso->type & IS_BYTECODE;

      if (isbytecode) {
        mtime = getMTimeOfSource(test);
      }
      code = getCodeFromData(test, isbytecode, ispackage, mtime);
      if (code == Py_None) {
        Py_DECREF(code);
        continue;
      }
      if (code != NULL) {
        modpath = test;
      }
      return code;
    }
  }
  PyErr_Format(PythonQtImportError, "can't find module '%.200s'", fullname);

  return NULL;
}

QString PythonQtImport::replaceExtension(const QString& str, const QString& ext)
{
 QString r;
 int i = str.lastIndexOf('.');
 if (i!=-1) {
   r = str.mid(0,i) + "." + ext;
 } else {
   r = str + "." + ext;
 }
 return r;
}

PyObject* PythonQtImport::getCodeFromPyc(const QString& file)
{
  PyObject* code;
  const static QString pycStr("pyc");
  QString pyc = replaceExtension(file, pycStr);
  if (PythonQt::importInterface()->exists(pyc)) {
    time_t mtime = 0;
    mtime = getMTimeOfSource(pyc);
    code = getCodeFromData(pyc, true, false, mtime);
    if (code != Py_None && code != NULL) {
      return code;
    }
    if (code) {
      Py_DECREF(code);
    }
  }
  code = getCodeFromData(file,false,false,0);
  return code;
}

/* Module init */

PyDoc_STRVAR(mlabimport_doc,
"Imports python files into PythonQt, completely replaces internal python import");

void PythonQtImport::init()
{
  static bool first = true;
  if (!first) {
    return;
  }
  first = false;

  PyObject *mod;

  if (PyType_Ready(&PythonQtImporter_Type) < 0)
    return;

  /* Correct directory separator */
  mlab_searchorder[0].suffix[0] = SEP;
  mlab_searchorder[1].suffix[0] = SEP;
  mlab_searchorder[2].suffix[0] = SEP;
  if (Py_OptimizeFlag) {
    /* Reverse *.pyc and *.pyo */
    struct st_mlab_searchorder tmp;
    tmp = mlab_searchorder[0];
    mlab_searchorder[0] = mlab_searchorder[1];
    mlab_searchorder[1] = tmp;
    tmp = mlab_searchorder[3];
    mlab_searchorder[3] = mlab_searchorder[4];
    mlab_searchorder[4] = tmp;
  }

  mod = Py_InitModule4("PythonQtImport", NULL, mlabimport_doc,
           NULL, PYTHON_API_VERSION);

  PythonQtImportError = PyErr_NewException("PythonQtImport.PythonQtImportError",
              PyExc_ImportError, NULL);
  if (PythonQtImportError == NULL)
    return;

  Py_INCREF(PythonQtImportError);
  if (PyModule_AddObject(mod, "PythonQtImportError",
             PythonQtImportError) < 0)
    return;

  Py_INCREF(&PythonQtImporter_Type);
  if (PyModule_AddObject(mod, "PythonQtImporter",
             (PyObject *)&PythonQtImporter_Type) < 0)
    return;

  // set our importer into the path_hooks to handle all path on sys.path
  PyObject* classobj = PyDict_GetItemString(PyModule_GetDict(mod), "PythonQtImporter");
  PyObject* path_hooks = PySys_GetObject("path_hooks");
  PyList_Append(path_hooks, classobj);

#ifndef WIN32
  // reload the encodings module, because it might fail to custom import requirements (e.g. encryption).
  PyObject* modules         = PyImport_GetModuleDict();
  PyObject* encodingsModule = PyDict_GetItemString(modules, "encodings");
  if (encodingsModule != NULL) {
    PyImport_ReloadModule(encodingsModule);
  }
#endif
}
