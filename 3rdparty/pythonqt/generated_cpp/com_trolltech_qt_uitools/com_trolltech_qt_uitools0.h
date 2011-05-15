#include <PythonQt.h>
#include <QObject>
#include <QVariant>
#include <qaction.h>
#include <qactiongroup.h>
#include <qbytearray.h>
#include <qcoreevent.h>
#include <qdir.h>
#include <qiodevice.h>
#include <qlayout.h>
#include <qlist.h>
#include <qobject.h>
#include <qstringlist.h>
#include <quiloader.h>
#include <qwidget.h>



class PythonQtShell_QUiLoader : public QUiLoader
{
public:
    PythonQtShell_QUiLoader(QObject*  parent = 0):QUiLoader(parent),_wrapper(NULL) {};

virtual void childEvent(QChildEvent*  arg__1);
virtual QAction*  createAction(QObject*  parent = 0, const QString&  name = QString());
virtual QActionGroup*  createActionGroup(QObject*  parent = 0, const QString&  name = QString());
virtual QLayout*  createLayout(const QString&  className, QObject*  parent = 0, const QString&  name = QString());
virtual QWidget*  createWidget(const QString&  className, QWidget*  parent = 0, const QString&  name = QString());
virtual void customEvent(QEvent*  arg__1);
virtual bool  event(QEvent*  arg__1);
virtual bool  eventFilter(QObject*  arg__1, QEvent*  arg__2);
virtual void timerEvent(QTimerEvent*  arg__1);

  PythonQtInstanceWrapper* _wrapper; 
};

class PythonQtPublicPromoter_QUiLoader : public QUiLoader
{ public:
inline QAction*  promoted_createAction(QObject*  parent = 0, const QString&  name = QString()) { return QUiLoader::createAction(parent, name); }
inline QActionGroup*  promoted_createActionGroup(QObject*  parent = 0, const QString&  name = QString()) { return QUiLoader::createActionGroup(parent, name); }
inline QLayout*  promoted_createLayout(const QString&  className, QObject*  parent = 0, const QString&  name = QString()) { return QUiLoader::createLayout(className, parent, name); }
inline QWidget*  promoted_createWidget(const QString&  className, QWidget*  parent = 0, const QString&  name = QString()) { return QUiLoader::createWidget(className, parent, name); }
};

class PythonQtWrapper_QUiLoader : public QObject
{ Q_OBJECT
public:
public slots:
QUiLoader* new_QUiLoader(QObject*  parent = 0);
void delete_QUiLoader(QUiLoader* obj) { delete obj; } 
   void addPluginPath(QUiLoader* theWrappedObject, const QString&  path);
   QStringList  availableLayouts(QUiLoader* theWrappedObject) const;
   QStringList  availableWidgets(QUiLoader* theWrappedObject) const;
   void clearPluginPaths(QUiLoader* theWrappedObject);
   QAction*  createAction(QUiLoader* theWrappedObject, QObject*  parent = 0, const QString&  name = QString());
   QActionGroup*  createActionGroup(QUiLoader* theWrappedObject, QObject*  parent = 0, const QString&  name = QString());
   QLayout*  createLayout(QUiLoader* theWrappedObject, const QString&  className, QObject*  parent = 0, const QString&  name = QString());
   QWidget*  createWidget(QUiLoader* theWrappedObject, const QString&  className, QWidget*  parent = 0, const QString&  name = QString());
   bool  isLanguageChangeEnabled(QUiLoader* theWrappedObject) const;
   bool  isScriptingEnabled(QUiLoader* theWrappedObject) const;
   bool  isTranslationEnabled(QUiLoader* theWrappedObject) const;
   QWidget*  load(QUiLoader* theWrappedObject, QIODevice*  device, QWidget*  parentWidget = 0);
   QStringList  pluginPaths(QUiLoader* theWrappedObject) const;
   void setLanguageChangeEnabled(QUiLoader* theWrappedObject, bool  enabled);
   void setScriptingEnabled(QUiLoader* theWrappedObject, bool  enabled);
   void setTranslationEnabled(QUiLoader* theWrappedObject, bool  enabled);
   void setWorkingDirectory(QUiLoader* theWrappedObject, const QDir&  dir);
   QDir  workingDirectory(QUiLoader* theWrappedObject) const;
};


