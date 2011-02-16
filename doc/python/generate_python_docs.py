import epydoc.apidoc
import epydoc.cli
import epydoc.docintrospecter
import epydoc.docwriter.html
import inspect
import PyQt4.QtCore
import sys
import types

# SIP does some strange stuff with the __dict__ of wrapped C++ classes:
#   someclass.__dict__["function"] != someclass.function
# These little hacks make epydoc generate documentation for the actual functions
# instead of their sip.methoddescriptor wrappers.

def is_pyqt_wrapper_class(thing):
  return epydoc.docintrospecter.isclass(thing) and \
         isinstance(thing, PyQt4.QtCore.pyqtWrapperType)

def introspect_pyqt_wrapper_class(thing, doc, module_name=None):
  # Inspect the class as normal
  doc = epydoc.docintrospecter.introspect_class(thing, doc, module_name=module_name)

  # Re-inspect the actual member functions
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


# Monkey-patch some functions in the HTML docwriter to show a table of contents
# down the side of each page, instead of in a separate frame.
original_write_header = epydoc.docwriter.html.HTMLWriter.write_header
def my_write_header(self, out, title):
  original_write_header(self, out, title)

  out('<div class="sidebar">')

  self.write_toc_section(out, "All Classes", self.class_list)

  # List the functions.
  funcs = [d for d in self.routine_list
              if not isinstance(self.docindex.container(d),
                (epydoc.apidoc.ClassDoc, types.NoneType))]
  self.write_toc_section(out, "All Functions", funcs)

  # List the variables.
  vars = []
  for doc in self.module_list:
      vars += doc.select_variables(value_type='other',
                                      imported=False,
                                      public=self._public_filter)
  self.write_toc_section(out, "All Variables", vars)

  out('</div>')
  out('<div class="maincontent">')

def my_write_footer(self, out, short=False):
  out('</div></body></html>')

def my_write_navbar(self, out, context):
  pass

original_write_toc_section = epydoc.docwriter.html.HTMLWriter.write_toc_section
def my_write_toc_section(self, out, name, docs, fullname=True):
  docs = [x for x in docs if not str(x.canonical_name).startswith('PyQt4')]
  original_write_toc_section(self, out, name, docs, fullname=fullname)

epydoc.docwriter.html.HTMLWriter.write_header = my_write_header
epydoc.docwriter.html.HTMLWriter.write_footer = my_write_footer
epydoc.docwriter.html.HTMLWriter.write_navbar = my_write_navbar
epydoc.docwriter.html.HTMLWriter.write_toc_section = my_write_toc_section


sys.argv = [
  "epydoc",
  "--html",
  "-o", "output",
  "-v",
  "--name", "clementine",
  "--url", "http://www.clementine-player.org",
  "--css", "epydoc.css",
  "--no-sourcecode",
  "--no-private",
  "--no-frames",
  "clementine",
]

print "Running '%s'" % ' '.join(sys.argv)
epydoc.cli.cli()
