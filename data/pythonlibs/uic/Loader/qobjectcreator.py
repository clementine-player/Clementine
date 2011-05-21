import sys

try:
    set()
except NameError:
    from sets import Set as set

from PythonQt import QtGui


class _QtGuiWrapper(object):
    def search(cls):
        return getattr(QtGui, cls, None)
    search = staticmethod(search)


class _ModuleWrapper(object):
    def __init__(self, moduleName, classes):
        self._moduleName = moduleName
        self._module = None
        self._classes = set(classes)

    def search(self, cls):
        if cls in self._classes:
            if self._module is None:
                self._module = __import__(self._moduleName, {}, {}, self._classes)
            # Allow for namespaces.
            obj = self._module
            for attr in cls.split('.'):
                obj = getattr(obj, attr)

            return obj
        else:
            return None


class _CustomWidgetLoader(object):
    def __init__(self):
        # should it stay this way?
        sys.path.append(".")
        self._widgets = {}
        self._modules = {}
        
    def addCustomWidget(self, widgetClass, baseClass, module):
        assert widgetClass not in self._widgets
        self._widgets[widgetClass] = module
    
    def search(self, cls):
        try:
            module = self._widgets[cls]
            if module not in self._modules:
                self._modules[module] = __import__(module, {}, {}, (cls,))
            
            return getattr(self._modules[module], cls)
        except KeyError:
            pass
        return None


class LoaderCreatorPolicy(object):
    def createQtGuiWrapper(self):
        return _QtGuiWrapper
    
    def createModuleWrapper(self, moduleName, classes):
        return _ModuleWrapper(moduleName, classes)
    
    def createCustomWidgetLoader(self):
        return _CustomWidgetLoader()

    def instantiate(self, clsObject, objectName, ctor_args, is_attribute=True):
        # Create a new type so setattr() can work
        new_type = type("__WidgetWrapper_" + clsObject.__name__, (clsObject,), {})
        return new_type(*ctor_args)

    def invoke(self, rname, method, args):
        return method(*args)

    def getSlot(self, object, slotname):
        # Rename slots that correspond to Python keyword arguments.
        if slotname == 'raise':
            slotname += '_'

        return getattr(object, slotname)
