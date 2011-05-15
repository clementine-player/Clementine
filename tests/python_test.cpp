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


class DISABLED_PythonTest : public ::testing::Test {
 protected:
  void SetUp() {
    manager_ = new ScriptManager;
    engine_ = qobject_cast<PythonEngine*>(
          manager_->EngineForLanguage(ScriptInfo::Language_Python));
  }

  void TearDown() {
    delete manager_;
  }

  ScriptManager* manager_;
  PythonEngine* engine_;
};


TEST_F(DISABLED_PythonTest, HasPythonEngine) {
  ASSERT_TRUE(engine_);
}

TEST_F(DISABLED_PythonTest, InitFromDirectory) {
  TemporaryScript script("pass");

  ScriptInfo info;
  info.InitFromDirectory(manager_, script.directory_);

  EXPECT_TRUE(info.is_valid());
  EXPECT_EQ(script.directory_, info.path());
  EXPECT_EQ(ScriptInfo::Language_Python, info.language());
  EXPECT_EQ(NULL, info.loaded());
}

TEST_F(DISABLED_PythonTest, StdioIsRedirected) {
  TemporaryScript script(
        "import sys\n"
        "print 'text on stdout'\n"
        "print >>sys.stderr, 'text on stderr'\n");
  ScriptInfo info;
  info.InitFromDirectory(manager_, script.directory_);

  engine_->CreateScript(info);

  QString log = manager_->log_lines_plain().join("\n");
  ASSERT_TRUE(log.contains("text on stdout"));
  ASSERT_TRUE(log.contains("text on stderr"));
}

TEST_F(DISABLED_PythonTest, CleanupModuleDict) {
  TemporaryScript script(
        "class Foo:\n"
        "  def __init__(self):\n"
        "    print 'constructor'\n"
        "  def __del__(self):\n"
        "    print 'destructor'\n"
        "f = Foo()\n");
  ScriptInfo info;
  info.InitFromDirectory(manager_, script.directory_);

  Script* s = engine_->CreateScript(info);
  ASSERT_TRUE(manager_->log_lines_plain().last().endsWith("constructor"));

  engine_->DestroyScript(s);
  ASSERT_TRUE(manager_->log_lines_plain().last().endsWith("destructor"));
}

TEST_F(DISABLED_PythonTest, ModuleConstants) {
  TemporaryScript script(
        "print __builtins__\n"
        "print __file__\n"
        "print __name__\n"
        "print __package__\n"
        "print __path__\n"
        "print script\n");
  ScriptInfo info;
  info.InitFromDirectory(manager_, script.directory_);

  engine_->CreateScript(info);

  const QStringList log = manager_->log_lines_plain();
  const int n = log.count();
  ASSERT_GE(n, 6);
  EXPECT_TRUE(log.at(n-6).endsWith("<module '__builtin__' (built-in)>"));     // __builtins__
  EXPECT_TRUE(log.at(n-5).endsWith(script.directory_ + "/script.py"));        // __file__
  EXPECT_TRUE(log.at(n-4).endsWith("clementinescripts." + info.id()));        // __name__
  EXPECT_TRUE(log.at(n-3).endsWith("None"));                                  // __package__
  EXPECT_TRUE(log.at(n-2).endsWith("['" + script.directory_ + "']"));         // __path__
  EXPECT_TRUE(log.at(n-1).contains("<clementine.ScriptInterface object at")); // script
}
