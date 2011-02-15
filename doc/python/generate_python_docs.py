import epydoc.cli
import epydoc.docintrospecter
import PyQt4.QtCore
import sys

import inspect

def is_pyqt_wrapper_class(thing):
  return epydoc.docintrospecter.isclass(thing) and \
         isinstance(thing, PyQt4.QtCore.pyqtWrapperType)

def introspect_pyqt_wrapper_class(thing, doc, module_name=None):
  doc = epydoc.docintrospecter.introspect_class(thing, doc, module_name=module_name)

  for name in thing.__dict__.keys():
    if name in doc.variables and hasattr(thing, name):
      actual_var = getattr(thing, name)
      val_doc = epydoc.docintrospecter.introspect_docs(
          actual_var, context=doc, module_name=module_name)
      var_doc = epydoc.docintrospecter.VariableDoc(
          name=name, value=val_doc, container=doc, docs_extracted_by='introspecter')
      doc.variables[name] = var_doc

  return doc

epydoc.docintrospecter.register_introspecter(is_pyqt_wrapper_class, introspect_pyqt_wrapper_class)


sys.argv = [
  "epydoc",
  "--html",
  "-o", "output",
  "-v",
  "--name", "clementine",
  "--url", "http://www.clementine-player.org",
  "clementine",
]

print "Running '%s'" % ' '.join(sys.argv)
epydoc.cli.cli()
