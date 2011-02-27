import epydoc
import epydoc.apidoc
import epydoc.cli
import epydoc.docbuilder
import epydoc.docintrospecter
import epydoc.docwriter.html
import epydoc.markup.epytext
import inspect
import PyQt4.QtCore
import sys
import types

DOC_PAGES = [
  ("Hello world!", "doc-hello-world"),
]

OUTPUT_DIR = "output"


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
# down the side of each page, instead of in a separate frame, and to do external
# API links.
original_write_header = epydoc.docwriter.html.HTMLWriter.write_header
def my_write_header(self, out, title):
  original_write_header(self, out, title)

  out('<div class="sidebar">')

  # General doc pages
  out('<h2 class="toc">%s</h2>\n' % "Documentation")
  for (title, filename) in DOC_PAGES:
    out('<a href="%s.html">%s</a><br/>' % (filename, title))

  # Classes
  self.write_toc_section(out, "Class reference", self.class_list)

  # Functions
  funcs = [d for d in self.routine_list
              if not isinstance(self.docindex.container(d),
                (epydoc.apidoc.ClassDoc, types.NoneType))]
  self.write_toc_section(out, "Function reference", funcs)

  # Variables
  vars = []
  for doc in self.module_list:
      vars += doc.select_variables(value_type='other',
                                      imported=False,
                                      public=self._public_filter)
  self.write_toc_section(out, "Variable reference", vars)

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

def qt_url(name):
  if not isinstance(name, str) and \
     not isinstance(name, epydoc.apidoc.DottedName) and \
     not isinstance(name, unicode):
    return None

  parts = str(name).split('.')
  if len(parts) >= 3 and parts[0] == "PyQt4":
    parts = parts[2:]

  if not parts or not parts[0].startswith("Q"):
    return None

  label = '.'.join(parts)
  url = "http://www.riverbankcomputing.co.uk/static/Docs/PyQt4/html/"
  url += "%s.html" % parts[0].lower()

  if len(parts) >= 2:
    url += "#%s" % parts[1].lower()

  return url

original_translate_identifier_xref = epydoc.docwriter.html._HTMLDocstringLinker.translate_identifier_xref
def my_translate_identifier_xref(self, identifier, label=None):
  url = qt_url(identifier)
  if url:
    label = '.'.join(identifier.split('.')[2:])
    return '<a href="%s" class="link">%s</a>' % (url, label)
  return original_translate_identifier_xref(self, identifier, label)

original_url = epydoc.docwriter.html.HTMLWriter.url
def my_url(self, obj):
  if isinstance(obj, epydoc.apidoc.ValueDoc):
    url = qt_url(obj.canonical_name)
    if url:
      return url
  return original_url(self, obj)

original__write = epydoc.docwriter.html.HTMLWriter._write
def my__write(self, write_func, directory, filename, *args):
  if filename.startswith("http://"):
    return
  original__write(self, write_func, directory, filename, *args)

epydoc.docwriter.html.HTMLWriter._write = my__write
epydoc.docwriter.html.HTMLWriter.write_header = my_write_header
epydoc.docwriter.html.HTMLWriter.write_footer = my_write_footer
epydoc.docwriter.html.HTMLWriter.write_navbar = my_write_navbar
epydoc.docwriter.html.HTMLWriter.write_toc_section = my_write_toc_section
epydoc.docwriter.html._HTMLDocstringLinker.translate_identifier_xref = my_translate_identifier_xref
epydoc.docwriter.html.HTMLWriter.url = my_url


sys.argv = [
  "epydoc",
  "--html",
  "-o", OUTPUT_DIR,
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

# Parse arguments
(options, names) = epydoc.cli.parse_arguments()

# Set up the logger
logger = epydoc.cli.ConsoleLogger(1, 'hide')
epydoc.log.register_logger(logger)

# Write the main docs - this is copied from cli()
epydoc.docstringparser.DEFAULT_DOCFORMAT = options.docformat
docindex = epydoc.docbuilder.build_doc_index(names,
    options.introspect, options.parse,
    add_submodules=True)
html_writer = epydoc.docwriter.html.HTMLWriter(docindex, **options.__dict__)
html_writer.write(options.target)

# Write extra pages
def write_extra_page(out, title, source):
  handle = open(source, 'r')
  parsed_docstring = epydoc.markup.epytext.parse_docstring(handle.read(), [])
  html_writer.write_header(out, title)
  out(html_writer.docstring_to_html(parsed_docstring))
  html_writer.write_footer(out)

for (title, filename) in DOC_PAGES:
  source = "%s.epydoc" % filename
  html   = "%s.html" % filename
  print "Generating '%s' from '%s'..." % (html, source)

  html_writer._write(write_extra_page, OUTPUT_DIR, html, title, source)
