/* This file is part of Clementine.
   Copyright 2010, David Sansome <me@davidsansome.com>

   Clementine is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   Clementine is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with Clementine.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <Python.h>

#include "core/utilities.h"
#include "playlist/songplaylistitem.h"
#include "scripting/script.h"
#include "scripting/scriptmanager.h"
#include "scripting/python/pythonengine.h"

#include "gmock/gmock.h"
#include "gtest/gtest.h"

#include "testobjectdecorators.h"
#include "test_utils.h"

#include <QSettings>
#include <QtDebug>

#include <boost/noncopyable.hpp>


class TemporaryScript : boost::noncopyable {
public:
  TemporaryScript(const QString& code, const QString& directory_template = QString()) {
    directory_ = Utilities::MakeTempDir(directory_template);

    QSettings ini(directory_ + "/script.ini", QSettings::IniFormat);
    ini.beginGroup("Script");
    ini.setValue("language", "python");
    ini.setValue("script_file", "script.py");

    QFile script(directory_ + "/script.py");
    script.open(QIODevice::WriteOnly);
    script.write(code.toUtf8());
  }

  ~TemporaryScript() {
    if (!directory_.isEmpty()) {
      Utilities::RemoveRecursive(directory_);
    }
  }

  QString directory_;
};


class PythonTest : public ::testing::Test {
 protected:
  static void SetUpTestCase() {
    sManager = new ScriptManager;
    sEngine = qobject_cast<PythonEngine*>(
          sManager->EngineForLanguage(ScriptInfo::Language_Python));

    sEngine->EnsureInitialised();
    PythonQt::self()->addDecorators(new TestObjectDecorators());
  }

  static void TearDownTestCase() {
    delete sManager;
  }

  static ScriptManager* sManager;
  static PythonEngine* sEngine;
};

ScriptManager* PythonTest::sManager = NULL;
PythonEngine* PythonTest::sEngine = NULL;


TEST_F(PythonTest, HasPythonEngine) {
  ASSERT_TRUE(sEngine);
}

TEST_F(PythonTest, InitFromDirectory) {
  TemporaryScript script("pass");

  ScriptInfo info;
  info.InitFromDirectory(sManager, script.directory_);

  EXPECT_TRUE(info.is_valid());
  EXPECT_EQ(script.directory_, info.path());
  EXPECT_EQ(ScriptInfo::Language_Python, info.language());
  EXPECT_EQ(NULL, info.loaded());
}

TEST_F(PythonTest, StdioIsRedirected) {
  TemporaryScript script(
        "import sys\n"
        "print 'text on stdout'\n"
        "print >>sys.stderr, 'text on stderr'\n");
  ScriptInfo info;
  info.InitFromDirectory(sManager, script.directory_);

  sEngine->CreateScript(info);

  QString log = sManager->log_lines_plain().join("\n");
  ASSERT_TRUE(log.contains("text on stdout"));
  ASSERT_TRUE(log.contains("text on stderr"));
}

TEST_F(PythonTest, CleanupModuleDict) {
  TemporaryScript script(
        "class Foo:\n"
        "  def __init__(self):\n"
        "    print 'constructor'\n"
        "  def __del__(self):\n"
        "    print 'destructor'\n"
        "f = Foo()\n");
  ScriptInfo info;
  info.InitFromDirectory(sManager, script.directory_);

  Script* s = sEngine->CreateScript(info);
  ASSERT_TRUE(sManager->log_lines_plain().last().endsWith("constructor"));

  sEngine->DestroyScript(s);
  ASSERT_TRUE(sManager->log_lines_plain().last().endsWith("destructor"));
}

TEST_F(PythonTest, CleanupSignalConnections) {
  TemporaryScript script(
        "from PythonQt.QtCore import QCoreApplication\n"
        "class Foo:\n"
        "  def __init__(self):\n"
        "    QCoreApplication.instance().connect('aboutToQuit()', self.aslot)\n"
        "  def __del__(self):\n"
        "    print 'destructor'\n"
        "  def aslot(self):\n"
        "    pass\n"
        "f = Foo()\n");
  ScriptInfo info;
  info.InitFromDirectory(sManager, script.directory_);

  sEngine->DestroyScript(sEngine->CreateScript(info));

  ASSERT_TRUE(sManager->log_lines_plain().last().endsWith("destructor"));
}

TEST_F(PythonTest, ModuleConstants) {
  TemporaryScript script(
        "print type(__builtins__)\n"
        "print __file__\n"
        "print __name__\n"
        "print __package__\n"
        "print __path__\n"
        "print __script__\n");
  ScriptInfo info;
  info.InitFromDirectory(sManager, script.directory_);

  sEngine->CreateScript(info);

  const QStringList log = sManager->log_lines_plain();
  const int n = log.count();
  ASSERT_GE(n, 6);
  EXPECT_TRUE(log.at(n-6).endsWith("<type 'dict'>"));                  // __builtins__
  EXPECT_TRUE(log.at(n-5).endsWith(script.directory_ + "/script.py")); // __file__
  EXPECT_TRUE(log.at(n-4).endsWith("clementinescripts." + info.id())); // __name__
  EXPECT_TRUE(log.at(n-3).endsWith("None"));                           // __package__
  EXPECT_TRUE(log.at(n-2).endsWith("['" + script.directory_ + "']"));  // __path__
  EXPECT_TRUE(log.at(n-1).contains("ScriptInterface (QObject "));      // __script__
}

TEST_F(PythonTest, PythonQtAttrSetWrappedCPP) {
  // Tests 3rdparty/pythonqt/patches/call-slot-returnvalue.patch

  TemporaryScript script(
        "import PythonQt.QtGui\n"
        "PythonQt.QtGui.QStyleOption().version = 123\n"
        "PythonQt.QtGui.QStyleOption().version = 123\n"
        "PythonQt.QtGui.QStyleOption().version = 123\n");
  ScriptInfo info;
  info.InitFromDirectory(sManager, script.directory_);

  EXPECT_TRUE(sEngine->CreateScript(info));
}

TEST_F(PythonTest, PythonQtArgumentReferenceCount) {
  // Tests 3rdparty/pythonqt/patches/argument-reference-count.patch

  TemporaryScript script(
        "from PythonQt.QtCore import QFile, QObject\n"

        "class Foo(QFile):\n"
        "  def Init(self, parent):\n"
        "    QFile.__init__(self, parent)\n"

        "parent = QObject()\n"
        "Foo().Init(parent)\n"
        "assert parent\n");
  ScriptInfo info;
  info.InitFromDirectory(sManager, script.directory_);

  EXPECT_TRUE(sEngine->CreateScript(info));
}

TEST_F(PythonTest, PythonQtConversionStack) {
  // Tests 3rdparty/pythonqt/patches/conversion-stack.patch
  // This crash is triggered when a C++ thing calls a virtual method on a
  // python wrapper and that wrapper returns a QString, QByteArray or
  // QStringList.  In this instance, initStyleOption() calls text() in Foo.

  TemporaryScript script(
        "from PythonQt.QtGui import QProgressBar, QStyleOptionProgressBar\n"

        "class Foo(QProgressBar):\n"
        "  def text(self):\n"
        "    return 'something'\n"

        "for _ in xrange(1000):\n"
        "  Foo().initStyleOption(QStyleOptionProgressBar())\n");
  ScriptInfo info;
  info.InitFromDirectory(sManager, script.directory_);

  EXPECT_TRUE(sEngine->CreateScript(info));
}
