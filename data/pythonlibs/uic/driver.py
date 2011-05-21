import sys
import logging

from uic import compileUi, loadUi


class Driver(object):
    """ This encapsulates access to the pyuic functionality so that it can be
    called by code that is Python v2/v3 specific.
    """

    LOGGER_NAME = 'uic'

    def __init__(self, opts, ui_file):
        """ Initialise the object.  opts is the parsed options.  ui_file is the
        name of the .ui file.
        """

        if opts.debug:
            logger = logging.getLogger(self.LOGGER_NAME)
            handler = logging.StreamHandler()
            handler.setFormatter(logging.Formatter("%(name)s: %(message)s"))
            logger.addHandler(handler)
            logger.setLevel(logging.DEBUG)

        self._opts = opts
        self._ui_file = ui_file

    def invoke(self):
        """ Invoke the action as specified by the parsed options.  Returns 0 if
        there was no error.
        """

        if self._opts.preview:
            return self._preview()

        self._generate()

        return 0

    def _preview(self):
        """ Preview the .ui file.  Return the exit status to be passed back to
        the parent process.
        """

        from PythonQt import QtGui

        app = QtGui.QApplication([self._ui_file])
        widget = loadUi(self._ui_file)
        widget.show()

        return app.exec_()

    def _generate(self):
        """ Generate the Python code. """

        if self._opts.output == "-":
            pyfile = sys.stdout
        else:
            pyfile = open(self._opts.output, 'wt')

        compileUi(self._ui_file, pyfile, self._opts.execute, self._opts.indent,
                self._opts.pyqt3_wrapper)

    def on_IOError(self, e):
        """ Handle an IOError exception. """

        sys.stderr.write("Error: %s: \"%s\"\n" % (e.strerror, e.filename))

    def on_SyntaxError(self, e):
        """ Handle a SyntaxError exception. """

        sys.stderr.write("Error in input file: %s\n" % e)

    def on_NoSuchWidgetError(self, e):
        """ Handle a NoSuchWidgetError exception. """

        if e.args[0].startswith("Q3"):
            sys.stderr.write("Error: Q3Support widgets are not supported by PythonQt.\n")
        else:
            sys.stderr.write(str(e) + "\n")

    def on_Exception(self, e):
        """ Handle a generic exception. """

        if logging.getLogger(self.LOGGER_NAME).level == logging.DEBUG:
            import traceback

            traceback.print_exception(*sys.exc_info())
        else:
            sys.stderr.write("An unexpected error occurred")
