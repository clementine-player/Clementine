"""Maintain a cache of icons.

If an icon is used more than once by a GUI then ensure that only one copy is
created.
"""


class IconCache(object):
    """A cache of icons."""

    def __init__(self, object_factory, qtgui_module):
        """Initialise the cache."""

        self._object_factory = object_factory
        self._qtgui_module = qtgui_module
        self._cache = []

    def get_icon(self, iconset):
        """Return an icon described by the given iconset tag."""

        iset = _IconSet(iconset)

        try:
            idx = self._cache.index(iset)
        except ValueError:
            idx = -1

        if idx >= 0:
            # Return the icon from the cache.
            iset = self._cache[idx]
        else:
            # Follow uic's naming convention.
            name = 'icon'
            idx = len(self._cache)

            if idx > 0:
                name += str(idx)

            icon = self._object_factory.createQObject("QIcon", name, (),
                    is_attribute=False)
            iset.set_icon(icon, self._qtgui_module)
            self._cache.append(iset)

        return iset.icon


class _IconSet(object):
    """An icon set, ie. the mode and state and the pixmap used for each."""

    def __init__(self, iconset):
        """Initialise the icon set from an XML tag."""

        # Set the pre-Qt v4.4 fallback (ie. with no roles).
        self._fallback = iconset.text.replace("\\", "\\\\")
        self._use_fallback = True

        # Parse the icon set.
        self._roles = {}

        for i in iconset:
            file_name = i.text
            if file_name is not None:
                file_name = file_name.replace("\\", "\\\\")

            self._roles[i.tag] = file_name
            self._use_fallback = False

        # There is no real icon yet.
        self.icon = None

    def set_icon(self, icon, qtgui_module):
        """Save the icon and set its attributes."""

        if self._use_fallback:
            icon.addFile(self._fallback)
        else:
            for role, pixmap in self._roles.items():
                if role.endswith("off"):
                    mode = role[:-3]
                    state = qtgui_module.QIcon.Off
                elif role.endswith("on"):
                    mode = role[:-2]
                    state = qtgui_module.QIcon.On
                else:
                    continue

                mode = getattr(qtgui_module.QIcon, mode.title())

                if pixmap:
                    icon.addPixmap(qtgui_module.QPixmap(pixmap), mode, state)
                else:
                    icon.addPixmap(qtgui_module.QPixmap(), mode, state)

        self.icon = icon

    def __eq__(self, other):
        """Compare two icon sets for equality."""

        if not isinstance(other, type(self)):
            return NotImplemented

        if self._use_fallback:
            if other._use_fallback:
                return self._fallback == other._fallback

            return False

        if other._use_fallback:
            return False

        return self._roles == other._roles
