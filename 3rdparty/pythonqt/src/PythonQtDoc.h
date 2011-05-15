#ifndef _PYTHONQTDOC_H
#define _PYTHONQTDOC_H

/*
 *
 *  Copyright (C) 2010 MeVis Medical Solutions AG All Rights Reserved.
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2.1 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  Further, this software is distributed without any warranty that it is
 *  free of the rightful claim of any third person regarding infringement
 *  or the like.  Any license provided herein, whether implied or
 *  otherwise, applies only to this software file.  Patent licenses, if
 *  any, provided herein do not apply to combinations of this program with
 *  other software, or any other product whatsoever.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 *  Contact information: MeVis Medical Solutions AG, Universitaetsallee 29,
 *  28359 Bremen, Germany or:
 *
 *  http://www.mevis.de
 *
 */

//----------------------------------------------------------------------------------
/*!
// \file    PythonQtDoc.h
// \author  Florian Link
// \author  Last changed by $Author: florian $
// \date    2006-10
*/
//----------------------------------------------------------------------------------

/*!
  \if USE_GLOBAL_DOXYGEN_DOC
  \page PythonQtPage PythonQt Overview
  \else
  \mainpage notitle
  \endif

 \image html PythonQt.jpg

 \section Introduction

 \b PythonQt is a dynamic <a href="http://www.python.org" target="_blank"> 
Python</a> binding for the <a href="http://qt.nokia.com" target="_blank"> 
Qt framework</a>.
 It offers an easy way to embed the Python scripting language into
 your C++ Qt applications. It makes heavy use of the QMetaObject system and thus requires Qt 4.x.

 The focus of PythonQt is on embedding Python into an existing C++ application, not on writing the whole
 application completely in Python. If you want to write your whole application in Python,
 you should use <a href="http://www.riverbankcomputing.co.uk/pyqt/" target="_blank">PyQt</a> or <a href="http://www.pyside.org" target="_blank">PySide</a> instead.

 If you are looking for a simple way to embed Python objects into your C++/Qt Application
 and to script parts of your application via Python, PythonQt is the way to go!

 PythonQt is a stable library that was developed to make the
 Image Processing and Visualization platform <a href="http://www.mevislab.de" target="_blank">MeVisLab</a>
 scriptable from Python.

 - \ref Features
 - \ref Download
 - \ref License
 - \ref Developer
 - \ref Building
 - \ref Examples

 \page Features Features

 \section Builtin Built-in Features

 The following are the built-in features of the PythonQt library:

 - Access all \b slots, \b properties, children and registered enums of any QObject derived class from Python
 - Connecting Qt Signals to Python functions (both from within Python and from C++)
 - Easy wrapping of Python objects from C++ with smart, reference-counting PythonQtObjectPtr.
 - Convenient conversions to/from QVariant for PythonQtObjectPtr.
 - Wrapping of C++ objects (which are not derived from QObject) via PythonQtCppWrapperFactory
 - Extending C++ and QObject derived classes with additional slots, static methods and constructors (see Decorators)
 - StdOut/Err redirection to Qt signals instead of cout
 - Interface for creating your own \c import replacement, so that Python scripts can be e.g. signed/verified before they are executed (PythonQtImportFileInterface)
 - Mapping of plain-old-datatypes and ALL QVariant types to and from Python
 - Support for wrapping of user QVariant types which are registerd via QMetaType
 - Support for Qt namespace (with all enumerators)
 - All PythonQt wrapped objects support the dir() statement, so that you can see easily which attributes a QObject, CPP object or QVariant has
 - No preprocessing/wrapping tool needs to be started, PythonQt can script any QObject without prior knowledge about it (except for the MetaObject information from the \b moc)
 - Multiple inheritance for C++ objects (e.g. a QWidget is derived from QObject and QPaintDevice, PythonQt will automatically cast a QWidget to a QPaintDevice when needed)
 - Polymorphic downcasting (if e.g. PythonQt sees a QEvent, it can downcast it depending on the type(), so the Python e.g. sees a QPaintEvent instead of a plain QEvent)
 - Deriving C++ objects from Python and overwriting virtual method with a Python implementation (requires usage of wrapper generator or manual work!)
 - Extensible handler for Python/C++ conversion of complex types, e.g. mapping of QVector<SomeObject> to/from a Python array
 - Setting of dynamic QObject properties via setProperty(), dynamic properties can be accessed for reading and writing like normal Python attributes (but creating a new property needs to be done with setProperty(), to distinguish from normal Python attributes)

 \section FeaturesQtAll Features with wrapper generator

 Thanks to the new wrapper generator, PythonQt now offers the additional PythonQt_QtAll library which wraps the complete Qt API, including all C++ classes and all non-slots on QObject derived classes.
 This offers the following features:

 - Complete Qt API wrapped and accessible
 - The following modules are available as submodules of the PythonQt module:
  - QtCore
  - QtGui
  - QtNetwork
  - QtOpenGL
  - QtSql
  - QtSvg
  - QtUiTools
  - QtWebKit
  - QtXml
  - (QtXmlPatterns, QtScript, QtHelp, phonon, assistant, designer are currently not supported, this would require some additional effort on the code generator)
 - For convenience, all classes are also available in the PythonQt.Qt module, for people who do not care in which module a class is located
 - Any Qt class that has virtual methods can be easily derived from Python and the virtual methods can be reimplemented in Python (this feature is considered experimental!)
 - Polymorphic downcasting on QEvent, QGraphicsItem, QStyleOption, ...
 - Multiple inheritance support (e.g., QGraphicsTextItem is a QObject AND a QGraphicsItem, PythonQt will handle this well)

 \section Comparison Comparison with PyQt/PySide

 - PythonQt is not as pythonic as PyQt in many details (e.g. buffer protocol, pickling, translation support, ...) and it is mainly thought for embedding and intercommunication between Qt/Cpp and Python
 - PythonQt allows to communicate in both directions, e.g., calling a Python object from C++ AND calling a C++ method from Python, while PyQt only handles the Python->C++ direction
 - PythonQt offers properties as Python attributes, while PyQt offers them as setter/getter methods (e.g. QWidget.width is a property in PythonQt and a method in PyQt)
 - PythonQt currently does not support instanceof checks for Qt classes, except for the exact match and derived Python classes
 - QObject.emit to emit Qt signals from Python is not yet implemented but PythonQt allows to just emit a signal by calling it like a normal slot
 - PythonQt does not (yet) offer to add new signals to Python/C++ objects and it does not yet support the newstyle PyQt signals (so you need to connect via C++ string signatures)
 - Ownership of objects is a bit different in PythonQt, currently Python classes derived from a C++ class need to be manually referenced in Python to not get deleted too early (this will be fixed in a future version)
 - QStrings are always converted to unicode Python objects, QByteArray always stays a QByteArray and can be converted using str()
 - There are many details in the generated wrappers that could need some polishing, e.g., methods that use pointer arguments for additional return values could return a results tuple.
 - Not all types of QList/QVector/QHash templates are supported, some Qt methods use those as arguments/return values (but you can add your own handlers to handle them if you need them).
 - Probably there are lots of details that differ, I do not know PyQt that well to list them all.
 - In the long run, PythonQt will consider using/extending PySide with the features of PythonQt to get rid of its own generator and typesystem files, alternatively the KDE Smoke generator might be used in the future (this has not yet been decided, the current PythonQt generator works well and there is no hurry to switch). 

 \page Download Download

 PythonQt is hosted on <a href="http://sourceforge.net/projects/pythonqt/" target="_blank">SourceForge</a>.

 You can download the source code as a tarball at http://sourceforge.net/projects/pythonqt/files/.
 Alternatively you can get the latest version from the svn repository.
 
 You can also browse the source code online via ViewVC: http://pythonqt.svn.sourceforge.net/viewvc/pythonqt/trunk/

 \note We do not offer prebuilt binaries, since there are so many possible combinations of
 platforms (Windows/Linux/MacOs), architectures (32/64 bit) and Python versions.

 \page License License
  
 PythonQt is distributed under the LGPL license, so it pairs well with the LGPL of the Qt 4.5 release and allows
 to be used in commercial applications when following the LGPL 2.1 obligations.

 The build system of PythonQt makes use of a modified version of the LGPL'ed QtScript generator,
 located in the "generator" directory.

 See http://qt.gitorious.org/qt-labs/qtscriptgenerator for details on the original project.
 Thanks a lot to the QtJambi guys and the QtScript Generator project for the C++ parser and
 Qt typesystem files!

 The PythonQt wrappers generated by the generator located in the "generated_cpp" directory are free to be used without any licensing restrictions.

 The generated wrappers are pre-generated and checked-in for Qt 4.6.1, so you only need to build and run the
 generator when you want to build additional wrappers or you want to upgrade/downgrade to another Qt version.
 You may use the generator to generate C++ bindings for your own C++ classes (e.g., to make them inheritable in Python),
 but this is currently not documented and involves creating your own typesystem files (although the Qt Jambi examples might help you).


 \page Developer Developer

 \section Interface Interface

 The main interface to PythonQt is the PythonQt singleton.
 PythonQt needs to be initialized via PythonQt::init() once.
 Afterwards you communicate with the singleton via PythonQt::self().
 PythonQt offers a complete Qt binding, which
 needs to be enabled via PythonQt_QtAll::init().


 \section Datatype Datatype Mapping

  The following table shows the mapping between Python and Qt objects:
  <table>
  <tr><th>Qt/C++</th><th>Python</th></tr>
  <tr><td>bool</td><td>bool</td></tr>
  <tr><td>double</td><td>float</td></tr>
  <tr><td>float</td><td>float</td></tr>
  <tr><td>char/uchar,int/uint,short,ushort,QChar</td><td>integer</td></tr>
  <tr><td>long</td><td>integer</td></tr>
  <tr><td>ulong,longlong,ulonglong</td><td>long</td></tr>
  <tr><td>QString</td><td>unicode string</td></tr>
  <tr><td>QByteArray</td><td>QByteArray wrapper</td></tr>
  <tr><td>char*</td><td>str</td></tr>
  <tr><td>QStringList</td><td>tuple of unicode strings</td></tr>
  <tr><td>QVariantList</td><td>tuple of objects</td></tr>
  <tr><td>QVariantMap</td><td>dict of objects</td></tr>
  <tr><td>QVariant</td><td>depends on type, see below</td></tr>
  <tr><td>QSize, QRect and all other standard Qt QVariants</td><td>variant wrapper that supports complete API of the respective Qt classes</td></tr>
  <tr><td>OwnRegisteredMetaType</td><td>C++ wrapper, optionally with additional information/wrapping provided by registerCPPClass()</td></tr>
  <tr><td>QList<AnyObject*></td><td>converts to a list of CPP wrappers</td></tr>
  <tr><td>QVector<AnyObject*></td><td>converts to a list of CPP wrappers</td></tr>
  <tr><td>EnumType</td><td>Enum wrapper derived from python integer</td></tr>
  <tr><td>QObject (and derived classes)</td><td>QObject wrapper</td></tr>
  <tr><td>C++ object</td><td>CPP wrapper, either wrapped via PythonQtCppWrapperFactory or just decorated with decorators</td></tr>
  <tr><td>PyObject</td><td>PyObject</td></tr>
  </table>

  PyObject is passed as direct pointer, which allows to pass/return any Python object directly to/from
  a Qt slot that uses PyObject* as its argument/return value.
  QVariants are mapped recursively as given above, e.g. a dictionary can
  contain lists of dictionaries of doubles.
  All Qt QVariant types are implemented, PythonQt supports the complete Qt API for these object.

 \section QObject QObject Wrapping

 All classes derived from QObject are automatically wrapped with a python wrapper class
 when they become visible to the Python interpreter. This can happen via
 - the PythonQt::addObject() method
 - when a Qt \b slot returns a QObject derived object to python
 - when a Qt \b signal contains a QObject and is connected to a python function

 It is important that you call PythonQt::registerClass() for any QObject derived class
 that may become visible to Python, except when you add it via PythonQt::addObject().
 This will register the complete parent hierachy of the registered class, so that
 when you register e.g. a QPushButton, QWidget will be registered as well (and all intermediate
 parents).

 From Python, you can talk to the returned QObjects in a natural way by calling
 their slots and receiving the return values. You can also read/write all
 properties of the objects as if they where normal python properties.

 In addition to this, the wrapped objects support
 - className() - returns a string that reprents the classname of the QObject
 - help() - shows all properties, slots, enums, decorator slots and constructors of the object, in a printable form
 - delete() - deletes the object (use with care, especially if you passed the ownership to C++)
 - connect(signal, function) - connect the signal of the given object to a python function
 - connect(signal, qobject, slot) - connect the signal of the given object to a slot of another QObject
 - disconnect(signal, function) - disconnect the signal of the given object from a python function
 - disconnect(signal, qobject, slot) - disconnect the signal of the given object from a slot of another QObject
 - children() - returns the children of the object
 - setParent(QObject) - set the parent
 - QObject* parent() - get the parent

 The below example shows how to connect signals in Python:

 \code
 # define a signal handler function
 def someFunction(flag):
   print flag

 # button1 is a QPushButton that has been added to Python via addObject()
 # connect the clicked signal to a python function:
 button1.connect("clicked(bool)", someFunction)

 \endcode

\section CPP CPP Wrapping

You can create dedicated wrapper QObjects for any C++ class. This is done by deriving from PythonQtCppWrapperFactory
and adding your factory via addWrapperFactory().
Whenever PythonQt encounters a CPP pointer (e.g. on a slot or signal)
and it does not known it as a QObject derived class, it will create a generic CPP wrapper. So even unknown C++ objects
can be passed through Python. If the wrapper factory supports the CPP class, a QObject wrapper will be created for each
instance that enters Python. An alternative to a complete wrapper via the wrapper factory are decorators, see \ref Decorators

\section MetaObject Meta Object/Class access

For each known C++ class, PythonQt provides a Python class. These classes are visible
inside of the "PythonQt" python module or in subpackages if a package is given when the class is registered.

A Meta class supports:

- access to all declared enum values
- constructors
- static methods
- unbound non-static methods
- help() and className()

From within Python, you can import the module "PythonQt" to access these classes and the Qt namespace.

\code
from PythonQt import QtCore

# namespace access:
print QtCore.Qt.AlignLeft

# constructors
a = QtCore.QSize(12,13)
b = QtCore.QFont()

# static method
QtCore.QDate.currentDate()

# enum value
QtCore.QFont.UltraCondensed

\endcode

\section Decorators Decorator slots

PythonQt introduces a new generic approach to extend any wrapped QObject or CPP object with

- constructors
- destructors (for CPP objects)
- additional slots
- static slots (callable on both the Meta object and the instances)

The idea behind decorators is that we wanted to make it as easy as possible to extend
wrapped objects. Since we already have an implementation for invoking any Qt Slot from
Python, it looked promising to use this approach for the extension of wrapped objects as well.
This avoids that the PythonQt user needs to care about how Python arguments are mapped from/to
Qt when he wants to create static methods, constructors and additional member functions.

The basic idea about decorators is to create a QObject derived class that implements slots
which take one of the above roles (e.g. constructor, destructor etc.) via a naming convention.
These slots are then assigned to other classes via the naming convention.

- SomeClassName* new_SomeClassName(...) - defines a constructor for "SomeClassName" that returns a new object of type SomeClassName (where SomeClassName can be any CPP class, not just QObject classes)
- void delete_SomeClassName(SomeClassName* o) - defines a destructor, which should delete the passed in object o
- anything static_SomeClassName_someMethodName(...) - defines a static method that is callable on instances and the meta class
- anything someMethodName(SomeClassName* o, ...) - defines a slot that will be available on SomeClassName instances (and derived instances). When such a slot is called the first argument is the pointer to the instance and the rest of the arguments can be used to make a call on the instance.

The below example shows all kinds of decorators in action:

\code

// an example CPP object
class YourCPPObject {
public:
  YourCPPObject(int arg1, float arg2) { a = arg1; b = arg2; }

  float doSomething(int arg1) { return arg1*a*b; };

  private:

  int a;
  float b;
};

// an example decorator
class ExampleDecorator : public QObject
{
  Q_OBJECT

public slots:
  // add a constructor to QSize that takes a QPoint
  QSize* new_QSize(const QPoint& p) { return new QSize(p.x(), p.y()); }

  // add a constructor for QPushButton that takes a text and a parent widget
  QPushButton* new_QPushButton(const QString& text, QWidget* parent=NULL) { return new QPushButton(text, parent); }

  // add a constructor for a CPP object
  YourCPPObject* new_YourCPPObject(int arg1, float arg2) { return new YourCPPObject(arg1, arg2); }

  // add a destructor for a CPP object
  void delete_YourCPPObject(YourCPPObject* obj) { delete obj; }

  // add a static method to QWidget
  QWidget* static_QWidget_mouseGrabber() { return QWidget::mouseGrabber(); }

  // add an additional slot to QWidget (make move() callable, which is not declared as a slot in QWidget)
  void move(QWidget* w, const QPoint& p) { w->move(p); }

  // add an additional slot to QWidget, overloading the above move method
  void move(QWidget* w, int x, int y) { w->move(x,y); }

  // add a method to your own CPP object
  int doSomething(YourCPPObject* obj, int arg1) { return obj->doSomething(arg1); }
};

...

PythonQt::self()->addDecorators(new ExampleDecorator());
PythonQt::self()->registerCPPClass("YourCPPObject");

\endcode

After you have registered an instance of the above ExampleDecorator, you can do the following from Python
(all these calls are mapped to the above decorator slots):

\code
from PythonQt import QtCore, QtGui, YourCPPObject

# call our new constructor of QSize
size = QtCore.QSize(QPoint(1,2));

# call our new QPushButton constructor
button = QtGui.QPushButton("sometext");

# call the move slot (overload1)
button.move(QPoint(0,0))

# call the move slot (overload2)
button.move(0,0)

# call the static method
grabber = QtGui.QWidget.mouseWrapper();

# create a CPP object via constructor
yourCpp = YourCPPObject(1,11.5)

# call the wrapped method on CPP object
print yourCpp.doSomething(1);

# destructor will be called:
yourCpp = None

\endcode

 \page Building Building

 PythonQt requires at least Qt 4.6.1 (for earlier Qt versions, you will need to run the pythonqt_gerenator, Qt 4.3 is the  absolute minimum) and Python 2.5.x or 2.6.x on Windows, Linux and MacOS X. It has not yet been tested with Python 3.x, but it should only require minor changes.
 To compile PythonQt, you will need a python developer installation which includes Python's header files and
the python2x.[lib | dll | so | dynlib].
 The build scripts a currently set to use Python 2.6.
 You may need to tweak the \b build/python.prf file to set the correct Python includes and libs on your system.

 \subsection Windows

 On Windows, the (non-source) Python Windows installer can be used.
 Make sure that you use the same compiler, the current Python distribution is built
 with Visual Studio 2003. If you want to use another compiler, you will need to build
 Python yourself, using your compiler.

 To build PythonQt, you need to set the environment variable \b PYTHON_PATH to point to the root
 dir of the python installation and \b PYTHON_LIB to point to
 the directory where the python lib file is located.

 When using the prebuild Python installer, this will be:

 \code
 > set PYTHON_PATH = c:\Python26
 > set PYTHON_LIB  = c:\Python26\libs
 \endcode

 When using the python sources, this will be something like:

 \code
  > set PYTHON_PATH = c:\yourDir\Python-2.6.1\
  > set PYTHON_LIB  = c:\yourDir\Python-2.6.1\PCbuild8\Win32
 \endcode

 To build all, do the following (after setting the above variables):

 \code
 > cd PythonQtRoot
 > vcvars32
 > qmake
 > nmake
 \endcode

 This should build everything. If Python can not be linked or include files can not be found,
 you probably need to tweak \b build/python.prf

 The tests and examples are located in PythonQt/lib.

 \subsection Linux

 On Linux, you need to install a Python-dev package.
 If Python can not be linked or include files can not be found,
 you probably need to tweak \b build/python.prf

 To build PythonQt, just do a:

 \code
 > cd PythonQtRoot
 > qmake
 > make all
 \endcode

 The tests and examples are located in PythonQt/lib.
 You should add PythonQt/lib to your LD_LIBRARY_PATH so that the runtime
 linker can find the *.so files.

 \subsection MacOsX

 On Mac, Python is installed as a Framework, so you should not need to install it.
 To build PythonQt, just do a:

 \code
 > cd PythonQtRoot
 > qmake
 > make all
 \endcode

 \section Tests

 There is a unit test that tests most features of PythonQt, see the \b tests subdirectory for details.

 \page Examples Examples

 Examples are available in the \b examples directory. The PyScriptingConsole implements a simple
 interactive scripting console that shows how to script a simple application. The PyLauncher application can be used to run arbitrary PythonQt scripts given on the commandline.

 The following shows a simple example on how to integrate PythonQt into your Qt application:

 \code
 #include "PythonQt.h"
 #include <QApplication>
 ...

 int main( int argc, char **argv )
 {

  QApplication qapp(argc, argv);

  // init PythonQt and Python itself
  PythonQt::init();

  // get a smart pointer to the __main__ module of the Python interpreter
  PythonQtObjectPtr context = PythonQt::self()->getMainModule();

  // add a QObject as variable of name "example" to the namespace of the __main__ module
  PyExampleObject example;
  context.addObject("example", &example);

  // do something
  context.evalScript("print example");
  context.evalScript("def multiply(a,b):\n  return a*b;\n");
  QVariantList args;
  args << 42 << 47;
  QVariant result = context.call("multiply", args);
  ...
 \endcode


*/
