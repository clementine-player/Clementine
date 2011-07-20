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
#include <frameobject.h>

#include <PythonQtConversion.h>
#include <com_trolltech_qt_core/com_trolltech_qt_core_init.h>
#include <com_trolltech_qt_gui/com_trolltech_qt_gui_init.h>
#include <com_trolltech_qt_network/com_trolltech_qt_network_init.h>

#include "objectdecorators.h"
#include "pythonengine.h"
#include "pythonscript.h"
#include "core/logging.h"
#include "core/song.h"
#include "core/player.h"
#include "core/taskmanager.h"
#include "covers/albumcoverfetcher.h"
#include "covers/coverproviders.h"
#include "internet/internetmodel.h"
#include "library/library.h"
#include "library/librarybackend.h"
#include "library/libraryview.h"
#include "playlist/playlistitem.h"
#include "playlist/playlistmanager.h"
#include "scripting/uiinterface.h"
#include "ui/settingsdialog.h"

#include <QFile>
#include <QModelIndex>
#include <QNetworkCookie>
#include <QtDebug>

Q_DECLARE_METATYPE(QModelIndex)
Q_DECLARE_METATYPE(QList<QModelIndex>)

const char* PythonEngine::kClementineModuleName = "clementine";
const char* PythonEngine::kScriptModulePrefix = "clementinescripts";
PythonEngine* PythonEngine::sInstance = NULL;


void PythonQt_init_Clementine(PyObject* module);


PythonEngine::PythonEngine(ScriptManager* manager)
  : LanguageEngine(manager),
    initialised_(false),
    modules_model_(new QStandardItemModel(this))
{
  Q_ASSERT(sInstance == NULL);
  sInstance = this;

  #ifdef Q_OS_DARWIN
    setenv("PYTHONPATH", (QCoreApplication::applicationDirPath() + "/../PlugIns").toLocal8Bit().constData(), 1);
  #endif
}

PythonEngine::~PythonEngine() {
  sInstance = NULL;

  modules_model_->clear();

  scripts_module_ = PythonQtObjectPtr();
  clementine_module_ = PythonQtObjectPtr();
  PythonQt::cleanup();
}

template <typename T>
static void RegisterListConverter(const char* other_class_name) {
  typedef QList<T> L;
  const int primary_id = qMetaTypeId<L>();
  const int secondary_id = QMetaType::type(other_class_name);

  PythonQtConvertMetaTypeToPythonCB* metatype_to_python =
      PythonQtConvertListOfValueTypeToPythonList<L, T>;
  PythonQtConvertPythonToMetaTypeCB* python_to_metatype =
      PythonQtConvertPythonListToListOfValueType<L, T>;

  qLog(Debug) << "Registering list converters for" <<
                 primary_id << QMetaType::typeName(primary_id) << "," <<
                 secondary_id << other_class_name;

  PythonQtConv::registerMetaTypeToPythonConverter(primary_id, metatype_to_python);
  PythonQtConv::registerMetaTypeToPythonConverter(secondary_id, metatype_to_python);

  PythonQtConv::registerPythonToMetaTypeConverter(primary_id, python_to_metatype);
  PythonQtConv::registerPythonToMetaTypeConverter(secondary_id, python_to_metatype);
}

bool PythonEngine::EnsureInitialised() {
  if (initialised_)
    return true;

  PythonQt::init(PythonQt::IgnoreSiteModule | PythonQt::RedirectStdOut);
  PythonQt* python_qt = PythonQt::self();

  // Add the Qt bindings
  PythonQt_init_QtCore(0);
  PythonQt_init_QtGui(0);
  PythonQt_init_QtNetwork(0);

  // Set the importer to allow imports from Qt resource paths
  python_qt->installDefaultImporter();
  python_qt->addSysPath(":/pythonlibs/");

  // Add some extra decorators on QObjects
  python_qt->addDecorators(new ObjectDecorators);

  // Register converters for list types
  RegisterListConverter<Song>("QList<Song>");
  RegisterListConverter<Directory>("QList<Directory>");
  RegisterListConverter<CoverSearchResult>("QList<CoverSearchResult>");
  RegisterListConverter<PlaylistItemPtr>("QList<PlaylistItemPtr>");
  RegisterListConverter<QModelIndex>("QList<QModelIndex>");
  RegisterListConverter<QNetworkCookie>("QList<QNetworkCookie>");

  // Connect stdout, stderr
  connect(python_qt, SIGNAL(pythonStdOut(QString)), SLOT(PythonStdOut(QString)));
  connect(python_qt, SIGNAL(pythonStdErr(QString)), SLOT(PythonStdErr(QString)));

  connect(python_qt, SIGNAL(signalConnectedToPython(PythonQtSignalReceiver*,int,PyObject*)),
          SLOT(SignalConnectedToPython(PythonQtSignalReceiver*,int,PyObject*)));

  // Create a clementine module
  clementine_module_ = python_qt->createModuleFromScript(kClementineModuleName);
  PythonQt_init_Clementine(clementine_module_);

  // Add classes
  python_qt->registerClass(&AutoExpandingTreeView::staticMetaObject, kClementineModuleName);

  const ScriptManager::GlobalData& data = manager()->data();
  if (data.valid_) {
    // Add objects
    clementine_module_.addObject("library",         data.library_->backend());
    clementine_module_.addObject("library_view",    data.library_view_);
    clementine_module_.addObject("player",          data.player_);
    clementine_module_.addObject("playlists",       data.playlists_);
    clementine_module_.addObject("internet_model",  data.internet_model_);
    clementine_module_.addObject("settings_dialog", data.settings_dialog_);
    clementine_module_.addObject("task_manager",    data.task_manager_);
    clementine_module_.addObject("cover_providers", &CoverProviders::instance());
  }

  clementine_module_.addObject("ui",           manager()->ui());
  clementine_module_.addObject("pythonengine", this);

  // Set up logging integration
  PythonQtObjectPtr logging_module = python_qt->importModule("clementinelogging");
  logging_module.call("setup_logging");

  // Create a module for scripts
  scripts_module_ = python_qt->createModuleFromScript(kScriptModulePrefix);

  // The modules model contains all the modules
  modules_model_->clear();
  AddModuleToModel("__main__", python_qt->getMainModule());

  initialised_ = true;
  return true;
}

Script* PythonEngine::CreateScript(const ScriptInfo& info) {
  // Initialise Python if it hasn't been done yet
  if (!EnsureInitialised()) {
    return NULL;
  }

  PythonScript* ret = new PythonScript(this, info);
  loaded_scripts_[ret->info().id()] = ret; // Used by RegisterNativeObject during startup
  if (ret->Init()) {
    AddModuleToModel(ret->module_name(), ret->module());
    return ret;
  }

  DestroyScript(ret);
  return NULL;
}

void PythonEngine::DestroyScript(Script* script) {
  PythonScript* python_script = static_cast<PythonScript*>(script);
  RemoveModuleFromModel(python_script->module_name());

  script->Unload();
  loaded_scripts_.remove(script->info().id());
  delete script;
}

void PythonEngine::AddStringToBuffer(const QString& str,
                                     const QString& buffer_name,
                                     QString* buffer, bool error) {
  buffer->append(str);

  int index = buffer->indexOf('\n');
  while (index != -1) {
    const QString message = buffer->left(index);
    buffer->remove(0, index + 1);
    index = buffer->indexOf('\n');

    logging::CreateLogger(logging::Level_Info, buffer_name, -1) <<
        message.toUtf8().constData();
    manager()->AddLogLine(buffer_name, message, error);
  }
}

void PythonEngine::PythonStdOut(const QString& str) {
  AddStringToBuffer(str, "sys.stdout", &stdout_buffer_, false);
}

void PythonEngine::PythonStdErr(const QString& str) {
  AddStringToBuffer(str, "sys.stderr", &stdout_buffer_, true);
}

void PythonEngine::HandleLogRecord(int level, const QString& logger_name,
                                   int lineno, const QString& message) {
  logging::Level        level_name = logging::Level_Debug;
  if      (level >= 40) level_name = logging::Level_Error;
  else if (level >= 30) level_name = logging::Level_Warning;
  else if (level >= 20) level_name = logging::Level_Info;

  logging::CreateLogger(level_name, logger_name, lineno) <<
      message.toUtf8().constData();
  manager()->AddLogLine(QString("%1:%2").arg(logger_name).arg(lineno),
                        message, level >= 30);
}

void PythonEngine::AddModuleToModel(const QString& name, PythonQtObjectPtr ptr) {
  QStandardItem* item = new QStandardItem(name);
  item->setData(QVariant::fromValue(ptr));
  modules_model_->appendRow(item);
}

void PythonEngine::RemoveModuleFromModel(const QString& name) {
  foreach (QStandardItem* item, modules_model_->findItems(name)) {
    modules_model_->removeRow(item->row());
  }
}

QString PythonEngine::CurrentScriptName() {
  // Walk up the Python stack and find the name of the script that's nearest
  // the top of the stack.
  const QString prefix = QString(kScriptModulePrefix) + ".";

  PyFrameObject* frame = PyEval_GetFrame();
  while (frame) {
    if (PyDict_Check(frame->f_globals)) {
      PyObject* __name__ = PyDict_GetItemString(frame->f_globals, "__name__");
      if (__name__ && PyString_Check(__name__)) {
        const QString name(PyString_AsString(__name__));
        if (name.startsWith(prefix)) {
          int n = name.indexOf('.', prefix.length());
          if (n != -1) {
            n -= prefix.length();
          }
          return name.mid(prefix.length(), n);
        }
      }
    }
    frame = frame->f_back;
  }
  return QString();
}

Script* PythonEngine::CurrentScript() const {
  const QString name(CurrentScriptName());
  if (loaded_scripts_.contains(name))
    return loaded_scripts_[name];
  return NULL;
}

void PythonEngine::SignalConnectedToPython(PythonQtSignalReceiver* receiver,
                                           int signal_id, PyObject* callable) {
  PythonScript* script = static_cast<PythonScript*>(CurrentScript());
  if (script) {
    script->RegisterSignalConnection(receiver, signal_id, callable);
  }
}
