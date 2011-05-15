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
// \file    PythonQtStdOut.cpp
// \author  Florian Link
// \author  Last changed by $Author: florian $
// \date    2006-05
*/
//----------------------------------------------------------------------------------

#include "PythonQtStdOut.h"

static PyObject *PythonQtStdOutRedirect_new(PyTypeObject *type, PyObject * /*args*/, PyObject * /*kwds*/)
{
  PythonQtStdOutRedirect *self;
  self = (PythonQtStdOutRedirect *)type->tp_alloc(type, 0);

  self->softspace = 0;
  self->_cb = NULL;

  return (PyObject *)self;
}

static PyObject *PythonQtStdOutRedirect_write(PyObject *self, PyObject *args)
{
  PythonQtStdOutRedirect*  s = (PythonQtStdOutRedirect*)self;
  if (s->_cb) {
    char *string;
    if (!PyArg_ParseTuple(args, "s", &string))
      return NULL;

    if (s->softspace > 0) {
      (*s->_cb)(QString(""));
      s->softspace = 0;
    }

    (*s->_cb)(QString(string));
  }
  return Py_BuildValue("");
}

static PyObject *PythonQtStdOutRedirect_flush(PyObject * /*self*/, PyObject * /*args*/)
{
  return Py_BuildValue("");
}



static PyMethodDef PythonQtStdOutRedirect_methods[] = {
  {"write", (PyCFunction)PythonQtStdOutRedirect_write, METH_VARARGS,
  "redirect the writing to a callback"},
  {"flush", (PyCFunction)PythonQtStdOutRedirect_flush, METH_VARARGS,
  "flush the output, currently not implemented but needed for logging framework"
  },
  {NULL,    NULL, 0 , NULL} /* sentinel */
};

static PyMemberDef PythonQtStdOutRedirect_members[] = {
  {"softspace", T_INT, offsetof(PythonQtStdOutRedirect, softspace), 0,
    "soft space flag"
  },
  {NULL}  /* Sentinel */
};

PyTypeObject PythonQtStdOutRedirectType = {
    PyObject_HEAD_INIT(NULL)
    0,                         /*ob_size*/
    "PythonQtStdOutRedirect",             /*tp_name*/
    sizeof(PythonQtStdOutRedirect),             /*tp_basicsize*/
    0,                         /*tp_itemsize*/
    0, /*tp_dealloc*/
    0,                         /*tp_print*/
    0,                         /*tp_getattr*/
    0,                         /*tp_setattr*/
    0,           /*tp_compare*/
    0,              /*tp_repr*/
    0,                         /*tp_as_number*/
    0,                         /*tp_as_sequence*/
    0,                         /*tp_as_mapping*/
    0,                         /*tp_hash */
    0,                         /*tp_call*/
    0,                         /*tp_str*/
    0,                         /*tp_getattro*/
    0,                         /*tp_setattro*/
    0,                         /*tp_as_buffer*/
    Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE, /*tp_flags*/
    "PythonQtStdOutRedirect",           /* tp_doc */
    0,                   /* tp_traverse */
    0,                   /* tp_clear */
    0,                   /* tp_richcompare */
    0,                   /* tp_weaklistoffset */
    0,                   /* tp_iter */
    0,                   /* tp_iternext */
    PythonQtStdOutRedirect_methods,                   /* tp_methods */
    PythonQtStdOutRedirect_members,                   /* tp_members */
    0,                   /* tp_getset */
    0,                         /* tp_base */
    0,                         /* tp_dict */
    0,                         /* tp_descr_get */
    0,                         /* tp_descr_set */
    0,                         /* tp_dictoffset */
    0,                         /* tp_init */
    0,                         /* tp_alloc */
    PythonQtStdOutRedirect_new,                 /* tp_new */
};
