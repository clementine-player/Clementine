from uic.exceptions import WidgetPluginError


def load_plugin(plugin, plugin_globals, plugin_locals):
    """ Load the given plugin (which is an open file).  Return True if the
    plugin was loaded, or False if it wanted to be ignored.  Raise an exception
    if there was an error.
    """

    try:
        exec(plugin.read(), plugin_globals, plugin_locals)
    except ImportError:
        return False
    except Exception, e:
        raise WidgetPluginError("%s: %s" % (e.__class__, str(e)))

    return True
