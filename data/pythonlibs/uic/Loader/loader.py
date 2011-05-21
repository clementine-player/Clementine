from PythonQt import QtGui, QtCore
from uic.uiparser import UIParser
from uic.Loader.qobjectcreator import LoaderCreatorPolicy


class DynamicUILoader(UIParser):
    def __init__(self):
        UIParser.__init__(self, QtCore, QtGui, LoaderCreatorPolicy())

    def createToplevelWidget(self, classname, widgetname):
        if self.toplevelInst is not None:
            if not isinstance(self.toplevelInst, self.factory.findQObjectType(classname)):
                raise TypeError(("Wrong base class of toplevel widget",
                                  (type(self.toplevelInst), classname)))
            return self.toplevelInst
        else:
            return self.factory.createQObject(classname, widgetname, ())

    def loadUi(self, filename, toplevelInst = None):
        self.toplevelInst = toplevelInst
        return self.parse(filename)
