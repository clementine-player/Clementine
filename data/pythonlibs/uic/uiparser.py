import sys
import logging
import os.path
import re

try:
    from xml.etree.ElementTree import parse, SubElement
except ImportError:
    try:
        from ElementTree import parse, SubElement
    except ImportError:
        from elementtree.ElementTree import parse, SubElement

from uic.exceptions import NoSuchWidgetError
from uic.objcreator import QObjectCreator
from uic.properties import Properties


logger = logging.getLogger(__name__)
DEBUG = logger.debug

if sys.version_info < (2,4,0):
    def reversed(seq):
        for i in xrange(len(seq)-1, -1, -1):
            yield seq[i]

QtCore = None
QtGui = None


def gridPosition(elem):
    """gridPosition(elem) -> tuple

    Return the 4-tuple of (row, column, rowspan, colspan)
    for a widget element, or an empty tuple.
    """
    try:
        return (int(elem.attrib["row"]),
                int(elem.attrib["column"]),
                int(elem.attrib.get("rowspan", 1)),
                int(elem.attrib.get("colspan", 1)))
    except KeyError:
        return ()


class WidgetStack(list):
    topwidget = None
    def push(self, item):
        DEBUG("push %s %s" % (item.className(),
                              item.objectName))
        self.append(item)
        if item.inherits("QWidget"):
            self.topwidget = item

    def popLayout(self):
        layout = list.pop(self)
        DEBUG("pop layout %s %s" % (layout.className(),
                                    layout.objectName))
        return layout

    def popWidget(self):
        widget = list.pop(self)
        DEBUG("pop widget %s %s" % (widget.className(),
                                    widget.objectName))
        for item in reversed(self):
            if item.inherits("QWidget"):
                self.topwidget = item
                break
        else:
            self.topwidget = None
        DEBUG("new topwidget %s" % (self.topwidget,))
        return widget

    def peek(self):
        return self[-1]

    def topIsLayout(self):
        return self[-1].inherits("QLayout")


class UIParser(object):    
    def __init__(self, QtCoreModule, QtGuiModule, creatorPolicy):
        self.factory = QObjectCreator(creatorPolicy)
        self.wprops = Properties(self.factory, QtCoreModule, QtGuiModule)
        
        global QtCore, QtGui
        QtCore = QtCoreModule
        QtGui = QtGuiModule
        
        self.reset()

    def uniqueName(self, name):
        """UIParser.uniqueName(string) -> string

        Create a unique name from a string.
        >>> p = UIParser(QtCore, QtGui)
        >>> p.uniqueName("foo")
        'foo'
        >>> p.uniqueName("foo")
        'foo1'
        """
        try:
            suffix = self.name_suffixes[name]
        except KeyError:
            self.name_suffixes[name] = 0
            return name

        suffix += 1
        self.name_suffixes[name] = suffix

        return "%s%i" % (name, suffix)

    def reset(self):
        try: self.wprops.reset()
        except AttributeError: pass
        self.toplevelWidget = None
        self.stack = WidgetStack()
        self.name_suffixes = {}
        self.defaults = {"spacing": 6, "margin": 0}
        self.actions = []
        self.currentActionGroup = None
        self.resources = []
        self.button_groups = []
        self.item_nr = 0

    def setupObject(self, clsname, parent, branch, is_attribute = True):
        name = self.uniqueName(branch.attrib.get("name") or clsname[1:].lower())
        if parent is None:
            args = ()
        else:
            args = (parent, )
        obj =  self.factory.createQObject(clsname, name, args, is_attribute)
        self.wprops.setProperties(obj, branch)
        obj.setObjectName(name)
        if is_attribute:
            setattr(self.toplevelWidget, name, obj)
        return obj
    
    def createWidget(self, elem):
        def widgetClass(elem):
            cls = elem.attrib["class"].replace('::', '.')
            if cls == "Line":
                return "QFrame"
            else:
                return cls
            
        self.column_counter = 0
        self.row_counter = 0
        self.item_nr = 0
        self.itemstack = []
        self.sorting_enabled = None
        
        parent = self.stack.topwidget
        for class_name in ["QToolBox", "QTabWidget", "QStackedWidget",
                           "QDockWidget", "QWizard"]:
            if parent.inherits(class_name):
                parent = None
                break
        
        self.stack.push(self.setupObject(widgetClass(elem), parent, elem))

        if self.stack.topwidget.inherits("QTableWidget"):
            self.stack.topwidget.setColumnCount(len(elem.findall("column")))
            self.stack.topwidget.setRowCount(len(elem.findall("row")))

        self.traverseWidgetTree(elem)
        widget = self.stack.popWidget()

        if widget.inherits("QTreeView"):
            self.handleHeaderView(elem, "header", widget.header())

        elif widget.inherits("QTableView"):
            self.handleHeaderView(elem, "horizontalHeader",
                    widget.horizontalHeader())
            self.handleHeaderView(elem, "verticalHeader",
                    widget.verticalHeader())

        elif widget.inherits("QAbstractButton"):
            bg_i18n = self.wprops.getAttribute(elem, "buttonGroup")
            if bg_i18n is not None:
                bg_name = str(bg_i18n)

                for bg in self.button_groups:
                    if bg.objectName == bg_name:
                        break
                else:
                    bg = self.factory.createQObject("QButtonGroup", bg_name,
                            (self.toplevelWidget, ))
                    bg.setObjectName(bg_name)
                    self.button_groups.append(bg)

                bg.addButton(widget)

        if self.sorting_enabled is not None:
            widget.setSortingEnabled(self.sorting_enabled)
            self.sorting_enabled = None
        
        if self.stack.topIsLayout():
            lay = self.stack.peek()
            gp = elem.attrib["grid-position"]

            if lay.inherits("QFormLayout"):
                if gp[1]:
                    role = QtGui.QFormLayout.FieldRole
                else:
                    role = QtGui.QFormLayout.LabelRole

                lay.setWidget(gp[0], role, widget)
            else:
                lay.addWidget(widget, *gp)

        topwidget = self.stack.topwidget

        if topwidget.inherits("QToolBox"):
            icon = self.wprops.getAttribute(elem, "icon")
            if icon is not None:
                topwidget.addItem(widget, icon, self.wprops.getAttribute(elem, "label"))
            else:
                topwidget.addItem(widget, self.wprops.getAttribute(elem, "label"))

            tooltip = self.wprops.getAttribute(elem, "toolTip")
            if tooltip is not None:
                topwidget.setItemToolTip(topwidget.indexOf(widget), tooltip)
                
        elif topwidget.inherits("QTabWidget"):
            icon = self.wprops.getAttribute(elem, "icon")
            if icon is not None:
                topwidget.addTab(widget, icon, self.wprops.getAttribute(elem, "title"))
            else:
                topwidget.addTab(widget, self.wprops.getAttribute(elem, "title"))

            tooltip = self.wprops.getAttribute(elem, "toolTip")
            if tooltip is not None:
                topwidget.setTabToolTip(topwidget.indexOf(widget), tooltip)
            
        elif topwidget.inherits("QWizard"):
            topwidget.addPage(widget)
            
        elif topwidget.inherits("QStackedWidget"):
            topwidget.addWidget(widget)
            
        elif topwidget.inherits("QDockWidget") or topwidget.inherits("QScrollArea"):
            topwidget.setWidget(widget)
            
        elif topwidget.inherits("QMainWindow"):
            if type(widget) == QtGui.QWidget:
                topwidget.setCentralWidget(widget)
            elif widget.inherits("QToolBar"):
                tbArea = self.wprops.getAttribute(elem, "toolBarArea")

                if tbArea is None:
                    topwidget.addToolBar(widget)
                else:
                    if isinstance(tbArea, str) or isinstance(tbArea, unicode):
                        tbArea = getattr(QtCore.Qt, tbArea)
                    else:
                        tbArea = QtCore.Qt.ToolBarArea(tbArea)

                    topwidget.addToolBar(tbArea, widget)

                tbBreak = self.wprops.getAttribute(elem, "toolBarBreak")

                if tbBreak:
                    topwidget.insertToolBarBreak(widget)

            elif widget.inherits("QMenuBar"):
                topwidget.setMenuBar(widget)
            elif widget.inherits("QStatusBar"):
                topwidget.setStatusBar(widget)
            elif widget.inherits("QDockWidget"):
                dwArea = self.wprops.getAttribute(elem, "dockWidgetArea")
                topwidget.addDockWidget(QtCore.Qt.DockWidgetArea(dwArea),
                        widget)

    def handleHeaderView(self, elem, name, header):
        value = self.wprops.getAttribute(elem, name + "Visible")
        if value is not None:
            header.setVisible(value)

        value = self.wprops.getAttribute(elem, name + "CascadingSectionResizes")
        if value is not None:
            header.setCascadingSectionResizes(value)

        value = self.wprops.getAttribute(elem, name + "DefaultSectionSize")
        if value is not None:
            header.setDefaultSectionSize(value)

        value = self.wprops.getAttribute(elem, name + "HighlightSections")
        if value is not None:
            header.setHighlightSections(value)

        value = self.wprops.getAttribute(elem, name + "MinimumSectionSize")
        if value is not None:
            header.setMinimumSectionSize(value)

        value = self.wprops.getAttribute(elem, name + "ShowSortIndicator")
        if value is not None:
            header.setSortIndicatorShown(value)

        value = self.wprops.getAttribute(elem, name + "StretchLastSection")
        if value is not None:
            header.setStretchLastSection(value)

    def createSpacer(self, elem):
        width = int(elem.findtext("property/size/width"))
        height = int(elem.findtext("property/size/height"))

        sizeType = self.wprops.getProperty(elem, "sizeType",
                QtGui.QSizePolicy.Expanding)

        policy = (QtGui.QSizePolicy.Minimum, sizeType)

        if self.wprops.getProperty(elem, "orientation") == QtCore.Qt.Horizontal:
            policy = policy[1], policy[0]

        name = self.uniqueName("spacerItem")
        spacer = self.factory.createQObject("QSpacerItem",
                name, (width, height) + policy,
                is_attribute=False)

        if self.stack.topIsLayout():
            lay = self.stack.peek()
            gp = elem.attrib["grid-position"]

            if lay.inherits("QFormLayout"):
                if gp[1]:
                    role = QtGui.QFormLayout.FieldRole
                else:
                    role = QtGui.QFormLayout.LabelRole

                lay.setItem(gp[0], role, spacer)
            else:
                lay.addItem(spacer, *gp)

        # Must keep a reference to the spacer otherwise it'll go out of scope
        # and cause a crash.
        setattr(self.toplevelWidget, name, spacer)

    def createLayout(self, elem):
        # Qt v4.3 introduced setContentsMargins() and separate values for each
        # of the four margins which are specified as separate properties.  This
        # doesn't really fit the way we parse the tree (why aren't the values
        # passed as attributes of a single property?) so we create a new
        # property and inject it.  However, if we find that they have all been
        # specified and have the same value then we inject a different property
        # that is compatible with older versions of Qt.
        left = self.wprops.getProperty(elem, 'leftMargin', -1)
        top = self.wprops.getProperty(elem, 'topMargin', -1)
        right = self.wprops.getProperty(elem, 'rightMargin', -1)
        bottom = self.wprops.getProperty(elem, 'bottomMargin', -1)

        # Count the number of properties and if they had the same value.
        def comp_property(m, so_far=-2, nr=0):
            if m >= 0:
                nr += 1

                if so_far == -2:
                    so_far = m
                elif so_far != m:
                    so_far = -1

            return so_far, nr

        margin, nr_margins = comp_property(left)
        margin, nr_margins = comp_property(top, margin, nr_margins)
        margin, nr_margins = comp_property(right, margin, nr_margins)
        margin, nr_margins = comp_property(bottom, margin, nr_margins)

        if nr_margins > 0:
            if nr_margins == 4 and margin >= 0:
                # We can inject the old margin property.
                me = SubElement(elem, 'property', name='margin')
                SubElement(me, 'number').text = str(margin)
            else:
                # We have to inject the new internal property.
                cme = SubElement(elem, 'property', name='pyuicContentsMargins')
                SubElement(cme, 'number').text = str(left)
                SubElement(cme, 'number').text = str(top)
                SubElement(cme, 'number').text = str(right)
                SubElement(cme, 'number').text = str(bottom)

        # We do the same for setHorizontalSpacing() and setVerticalSpacing().
        horiz = self.wprops.getProperty(elem, 'horizontalSpacing', -1)
        vert = self.wprops.getProperty(elem, 'verticalSpacing', -1)

        if horiz >= 0 or vert >= 0:
            # We inject the new internal property.
            cme = SubElement(elem, 'property', name='pyuicSpacing')
            SubElement(cme, 'number').text = str(horiz)
            SubElement(cme, 'number').text = str(vert)

        classname = elem.attrib["class"]
        if self.stack.topIsLayout():
            parent = None
        else:
            parent = self.stack.topwidget
        if "name" not in elem.attrib:
            elem.attrib["name"] = classname[1:].lower()
        self.stack.push(self.setupObject(classname, parent, elem))
        self.traverseWidgetTree(elem)

        layout = self.stack.popLayout()
        if self.stack.topIsLayout():
            top_layout = self.stack.peek()
            gp = elem.attrib["grid-position"]

            if top_layout.inherits("QFormLayout"):
                if gp[1]:
                    role = QtGui.QFormLayout.FieldRole
                else:
                    role = QtGui.QFormLayout.LabelRole

                top_layout.setLayout(gp[0], role, layout)
            else:
                self.configureLayout(elem, layout)
                top_layout.addLayout(layout, *gp)
        else:
            self.configureLayout(elem, layout)

    def configureLayout(self, elem, layout):
        if layout.inherits("QGridLayout"):
            self.setArray(elem, 'columnminimumwidth',
                    layout.setColumnMinimumWidth)
            self.setArray(elem, 'rowminimumheight',
                    layout.setRowMinimumHeight)
            self.setArray(elem, 'columnstretch', layout.setColumnStretch)
            self.setArray(elem, 'rowstretch', layout.setRowStretch)

        elif layout.inherits("QBoxLayout"):
            self.setArray(elem, 'stretch', layout.setStretch)

    def setArray(self, elem, name, setter):
        array = elem.attrib.get(name)
        if array:
            for idx, value in enumerate(array.split(',')):
                value = int(value)
                if value > 0:
                    setter(idx, value)

    def handleItem(self, elem):
        if self.stack.topIsLayout():
            elem[0].attrib["grid-position"] = gridPosition(elem)
            self.traverseWidgetTree(elem)
        else:
          
            w = self.stack.topwidget

            if w.inherits("QComboBox"):
                text = self.wprops.getProperty(elem, "text")
                icon = self.wprops.getProperty(elem, "icon")

                if icon:
                    w.addItem(icon, '')
                else:
                    w.addItem('')

                w.setItemText(self.item_nr, text)

            elif w.inherits("QListWidget"):
                text = self.wprops.getProperty(elem, "text")
                icon = self.wprops.getProperty(elem, "icon")
                flags = self.wprops.getProperty(elem, "flags")
                check_state = self.wprops.getProperty(elem, "checkState")

                if icon or flags or check_state:
                    item_name = "item"
                else:
                    item_name = None

                item = self.factory.createQObject("QListWidgetItem", item_name,
                        (w, ), False)

                if self.item_nr == 0:
                    self.sorting_enabled = self.factory.invoke("__sortingEnabled", w.isSortingEnabled)
                    w.setSortingEnabled(False)

                if text:
                    w.item(self.item_nr).setText(text)

                if icon:
                    item.setIcon(icon)

                if flags:
                    item.setFlags(flags)

                if check_state:
                    item.setCheckState(check_state)

            elif w.inherits("QTreeWidget"):
                if self.itemstack:
                    parent, _ = self.itemstack[-1]
                    _, nr_in_root = self.itemstack[0]
                else:
                    parent = w
                    nr_in_root = self.item_nr

                item = self.factory.createQObject("QTreeWidgetItem",
                        "item_%d" % len(self.itemstack), (parent, ), False)

                if self.item_nr == 0 and not self.itemstack:
                    self.sorting_enabled = self.factory.invoke("__sortingEnabled", w.isSortingEnabled)
                    w.setSortingEnabled(False)

                self.itemstack.append((item, self.item_nr))
                self.item_nr = 0

                # We have to access the item via the tree when setting the
                # text.
                titm = w.topLevelItem(nr_in_root)
                for child, nr_in_parent in self.itemstack[1:]:
                    titm = titm.child(nr_in_parent)

                column = -1
                for prop in elem.findall("property"):
                    c_prop = self.wprops.convert(prop)
                    c_prop_name = prop.attrib["name"]

                    if c_prop_name == "text":
                        column += 1
                        if c_prop:
                            titm.setText(column, c_prop)
                    elif c_prop_name == "icon":
                        item.setIcon(column, c_prop)
                    elif c_prop_name == "flags":
                        item.setFlags(c_prop)
                    elif c_prop_name == "checkState":
                        item.setCheckState(column, c_prop)

                self.traverseWidgetTree(elem)
                _, self.item_nr = self.itemstack.pop()

            elif w.inherits("QTableWidget"):
                text = self.wprops.getProperty(elem, "text")
                icon = self.wprops.getProperty(elem, "icon")
                flags = self.wprops.getProperty(elem, "flags")
                check_state = self.wprops.getProperty(elem, "checkState")

                item = self.factory.createQObject("QTableWidgetItem", "item",
                        (), False)

                if self.item_nr == 0:
                    self.sorting_enabled = self.factory.invoke("__sortingEnabled", w.isSortingEnabled)
                    w.setSortingEnabled(False)

                row = int(elem.attrib["row"])
                col = int(elem.attrib["column"])

                if text:
                    w.item(row, col).setText(text)

                if icon:
                    item.setIcon(icon)

                if flags:
                    item.setFlags(flags)

                if check_state:
                    item.setCheckState(check_state)

                w.setItem(row, col, item)

            self.item_nr += 1

    def addAction(self, elem):
        self.actions.append((self.stack.topwidget, elem.attrib["name"]))

    def addHeader(self, elem):
        w = self.stack.topwidget

        if w.inherits("QTreeWidget"):
            text = self.wprops.getProperty(elem, "text")
            icon = self.wprops.getProperty(elem, "icon")

            if text:
                w.headerItem().setText(self.column_counter, text)

            if icon:
                w.headerItem().setIcon(self.column_counter, icon)

            self.column_counter += 1

        elif w.inherits("QTableWidget"):
            if len(elem) == 0:
                return

            text = self.wprops.getProperty(elem, "text")
            icon = self.wprops.getProperty(elem, "icon")

            item = self.factory.createQObject("QTableWidgetItem", "item",
                        (), False)

            if elem.tag == "column":
                w.setHorizontalHeaderItem(self.column_counter, item)

                if text:
                    w.horizontalHeaderItem(self.column_counter).setText(text)

                if icon:
                    item.setIcon(icon)

                self.column_counter += 1
            elif elem.tag == "row":
                w.setVerticalHeaderItem(self.row_counter, item)

                if text:
                    w.verticalHeaderItem(self.row_counter).setText(text)

                if icon:
                    item.setIcon(icon)

                self.row_counter += 1

    def createAction(self, elem):
        self.setupObject("QAction", self.currentActionGroup or self.toplevelWidget,
                         elem)

    def createActionGroup(self, elem):
        action_group = self.setupObject("QActionGroup", self.toplevelWidget, elem)
        self.currentActionGroup = action_group
        self.traverseWidgetTree(elem)
        self.currentActionGroup = None

    widgetTreeItemHandlers = {
        "widget"    : createWidget,
        "addaction" : addAction,
        "layout"    : createLayout,
        "spacer"    : createSpacer,
        "item"      : handleItem,
        "action"    : createAction,
        "actiongroup": createActionGroup,
        "column"    : addHeader,
        "row"       : addHeader,
        }

    def traverseWidgetTree(self, elem):
        for child in iter(elem):
            try:
                handler = self.widgetTreeItemHandlers[child.tag]
            except KeyError:
                continue

            handler(self, child)

    def createUserInterface(self, elem):
        # Get the names of the class and widget.
        cname = elem.attrib["class"]
        wname = elem.attrib["name"]

        # If there was no widget name then derive it from the class name.
        if not wname:
            wname = cname

            if wname.startswith("Q"):
                wname = wname[1:]

            wname = wname[0].lower() + wname[1:]

        self.toplevelWidget = self.createToplevelWidget(cname, wname)
        self.toplevelWidget.setObjectName(wname)
        DEBUG("toplevel widget is %s",
              self.toplevelWidget.className())
        self.wprops.setProperties(self.toplevelWidget, elem)
        self.stack.push(self.toplevelWidget)
        self.traverseWidgetTree(elem)
        self.stack.popWidget()
        self.addActions()
        self.setBuddies()
        self.setDelayedProps()
        
    def addActions(self):
        for widget, action_name in self.actions:
            if action_name == "separator":
                widget.addSeparator()
            else:
                DEBUG("add action %s to %s", action_name, widget.objectName)
                action_obj = getattr(self.toplevelWidget, action_name)
                if action_obj.inherits("QMenu"):
                    widget.addAction(action_obj.menuAction())
                elif not action_obj.inherits("QActionGroup"):
                    widget.addAction(action_obj)

    def setDelayedProps(self):
        for func, args in self.wprops.delayed_props:
            func(args)
            
    def setBuddies(self):
        for widget, buddy in self.wprops.buddies:
            DEBUG("%s is buddy of %s", buddy, widget.objectName)
            try:
                widget.setBuddy(getattr(self.toplevelWidget, buddy))
            except AttributeError:
                DEBUG("ERROR in ui spec: %s (buddy of %s) does not exist",
                      buddy, widget.objectName)

    def classname(self, elem):
        DEBUG("uiname is %s", elem.text)
        name = elem.text

        if name is None:
            name = ""

        self.uiname = name
        self.wprops.uiname = name
        self.setContext(name)

    def setContext(self, context):
        """
        Reimplemented by a sub-class if it needs to know the translation
        context.
        """
        pass

    def readDefaults(self, elem):
        self.defaults["margin"] = int(elem.attrib["margin"])
        self.defaults["spacing"] = int(elem.attrib["spacing"])

    def setTaborder(self, elem):
        lastwidget = None
        for widget_elem in elem:
            widget = getattr(self.toplevelWidget, widget_elem.text)

            if lastwidget is not None:
                self.toplevelWidget.setTabOrder(lastwidget, widget)

            lastwidget = widget

    def readResources(self, elem):
        """
        Read a "resources" tag and add the module to import to the parser's
        list of them.
        """
        for include in elem.getiterator("include"):
            loc = include.attrib.get("location")

            # Assume our convention for naming the Python files generated by
            # pyrcc4.
            if loc and loc.endswith('.qrc'):
                self.resources.append(os.path.basename(loc[:-4] + '_rc'))

    def createConnections(self, elem):
        def name2object(obj):
            if obj == self.uiname:
                return self.toplevelWidget
            else:
                return getattr(self.toplevelWidget, obj)
        for conn in iter(elem):
            name2object(conn.findtext("sender")).connect(
                conn.findtext("signal"),
                name2object(conn.findtext("receiver")),
                conn.findtext("slot"))

    def customWidgets(self, elem):
        def header2module(header):
            """header2module(header) -> string

            Convert paths to C++ header files to according Python modules
            >>> header2module("foo/bar/baz.h")
            'foo.bar.baz'
            """
            if header.endswith(".h"):
                header = header[:-2]

            mpath = []
            for part in header.split('/'):
                # Ignore any empty parts or those that refer to the current
                # directory.
                if part not in ('', '.'):
                    if part == '..':
                        # We should allow this for Python3.
                        raise SyntaxError("custom widget header file name may not contain '..'.")

                    mpath.append(part)

            return '.'.join(mpath)
    
        for custom_widget in iter(elem):
            classname = custom_widget.findtext("class")
            if classname.startswith("Q3"):
                raise NoSuchWidgetError(classname)
            self.factory.addCustomWidget(classname,
                                     custom_widget.findtext("extends") or "QWidget",
                                     header2module(custom_widget.findtext("header")))

    def createToplevelWidget(self, classname, widgetname):
        raise NotImplementedError
    
    # finalize will be called after the whole tree has been parsed and can be
    # overridden.
    def finalize(self):
        pass

    def parse(self, filename):
        # the order in which the different branches are handled is important
        # the widget tree handler relies on all custom widgets being known,
        # and in order to create the connections, all widgets have to be populated
        branchHandlers = (
            ("layoutdefault", self.readDefaults),
            ("class",         self.classname),
            ("customwidgets", self.customWidgets),
            ("widget",        self.createUserInterface),
            ("connections",   self.createConnections),
            ("tabstops",      self.setTaborder),
            ("resources",     self.readResources),
        )

        document = parse(filename)
        version = document.getroot().attrib["version"]
        DEBUG("UI version is %s" % (version,))
        # Right now, only version 4.0 is supported, which is used up to at
        # least Qt 4.4.
        assert version in ("4.0",)
        for tagname, actor in branchHandlers:
            elem = document.find(tagname)
            if elem is not None:
                actor(elem)
        self.finalize()
        w = self.toplevelWidget
        self.reset()
        return w
