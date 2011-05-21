__all__ = ("loadUi")


def loadUi(uifile, baseinstance=None):
    """loadUi(uifile, baseinstance=None) -> widget

    Load a Qt Designer .ui file and return an instance of the user interface.

    uifile is a file name or file-like object containing the .ui file.
    baseinstance is an optional instance of the Qt base class.  If specified
    then the user interface is created in it.  Otherwise a new instance of the
    base class is automatically created.
    """

    from uic.Loader.loader import DynamicUILoader

    return DynamicUILoader().loadUi(uifile, baseinstance)


# The list of directories that are searched for widget plugins.
from uic.objcreator import widgetPluginPath
