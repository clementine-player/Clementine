class NoSuchWidgetError(Exception):
    def __str__(self):
        return "Unknown Qt widget: %s" % (self.args[0],)

class UnsupportedPropertyError(Exception):
    pass

class WidgetPluginError(Exception):
    pass
