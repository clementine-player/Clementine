#include "clementine0.h"
#include <PythonQtConversion.h>
#include <PythonQtMethodInfo.h>
#include <PythonQtSignalReceiver.h>
#include <QVariant>
#include <albumcoverfetcher.h>
#include <albumcoverfetchersearch.h>
#include <coverprovider.h>
#include <coverproviderfactory.h>
#include <coverproviders.h>
#include <directory.h>
#include <librarybackend.h>
#include <libraryquery.h>
#include <qabstractitemmodel.h>
#include <qabstractnetworkcache.h>
#include <qbytearray.h>
#include <qcoreevent.h>
#include <qdatastream.h>
#include <qdir.h>
#include <qimage.h>
#include <qiodevice.h>
#include <qlist.h>
#include <qnetworkrequest.h>
#include <qobject.h>
#include <qpoint.h>
#include <qsize.h>
#include <qstandarditemmodel.h>
#include <qstringlist.h>
#include <qurl.h>
#include <qwidget.h>
#include <radiomodel.h>
#include <radioservice.h>
#include <song.h>
#include <taskmanager.h>
#include <urlhandler.h>

void PythonQtShell_AlbumCoverFetcherSearch::childEvent(QChildEvent*  arg__1)
{
if (_wrapper) {
  PyObject* obj = PyObject_GetAttrString((PyObject*)_wrapper, "childEvent");
  PyErr_Clear();
  if (obj && !PythonQtSlotFunction_Check(obj)) {
    static const char* argumentList[] ={"" , "QChildEvent*"};
    static const PythonQtMethodInfo* methodInfo = PythonQtMethodInfo::getCachedMethodInfoFromArgumentList(2, argumentList);
    void* args[2] = {NULL, (void*)&arg__1};
    PyObject* result = PythonQtSignalTarget::call(obj, methodInfo, args, true);
    if (result) { Py_DECREF(result); } 
    Py_DECREF(obj);
    return;
  }
}
  AlbumCoverFetcherSearch::childEvent(arg__1);
}
void PythonQtShell_AlbumCoverFetcherSearch::customEvent(QEvent*  arg__1)
{
if (_wrapper) {
  PyObject* obj = PyObject_GetAttrString((PyObject*)_wrapper, "customEvent");
  PyErr_Clear();
  if (obj && !PythonQtSlotFunction_Check(obj)) {
    static const char* argumentList[] ={"" , "QEvent*"};
    static const PythonQtMethodInfo* methodInfo = PythonQtMethodInfo::getCachedMethodInfoFromArgumentList(2, argumentList);
    void* args[2] = {NULL, (void*)&arg__1};
    PyObject* result = PythonQtSignalTarget::call(obj, methodInfo, args, true);
    if (result) { Py_DECREF(result); } 
    Py_DECREF(obj);
    return;
  }
}
  AlbumCoverFetcherSearch::customEvent(arg__1);
}
bool  PythonQtShell_AlbumCoverFetcherSearch::event(QEvent*  arg__1)
{
if (_wrapper) {
  PyObject* obj = PyObject_GetAttrString((PyObject*)_wrapper, "event");
  PyErr_Clear();
  if (obj && !PythonQtSlotFunction_Check(obj)) {
    static const char* argumentList[] ={"bool" , "QEvent*"};
    static const PythonQtMethodInfo* methodInfo = PythonQtMethodInfo::getCachedMethodInfoFromArgumentList(2, argumentList);
      bool returnValue;
    void* args[2] = {NULL, (void*)&arg__1};
    PyObject* result = PythonQtSignalTarget::call(obj, methodInfo, args, true);
    if (result) {
      args[0] = PythonQtConv::ConvertPythonToQt(methodInfo->parameters().at(0), result, false, NULL, &returnValue);
      if (args[0]!=&returnValue) {
        if (args[0]==NULL) {
          PythonQt::priv()->handleVirtualOverloadReturnError("event", methodInfo, result);
        } else {
          returnValue = *((bool*)args[0]);
        }
      }
    }
    if (result) { Py_DECREF(result); } 
    Py_DECREF(obj);
    return returnValue;
  }
}
  return AlbumCoverFetcherSearch::event(arg__1);
}
bool  PythonQtShell_AlbumCoverFetcherSearch::eventFilter(QObject*  arg__1, QEvent*  arg__2)
{
if (_wrapper) {
  PyObject* obj = PyObject_GetAttrString((PyObject*)_wrapper, "eventFilter");
  PyErr_Clear();
  if (obj && !PythonQtSlotFunction_Check(obj)) {
    static const char* argumentList[] ={"bool" , "QObject*" , "QEvent*"};
    static const PythonQtMethodInfo* methodInfo = PythonQtMethodInfo::getCachedMethodInfoFromArgumentList(3, argumentList);
      bool returnValue;
    void* args[3] = {NULL, (void*)&arg__1, (void*)&arg__2};
    PyObject* result = PythonQtSignalTarget::call(obj, methodInfo, args, true);
    if (result) {
      args[0] = PythonQtConv::ConvertPythonToQt(methodInfo->parameters().at(0), result, false, NULL, &returnValue);
      if (args[0]!=&returnValue) {
        if (args[0]==NULL) {
          PythonQt::priv()->handleVirtualOverloadReturnError("eventFilter", methodInfo, result);
        } else {
          returnValue = *((bool*)args[0]);
        }
      }
    }
    if (result) { Py_DECREF(result); } 
    Py_DECREF(obj);
    return returnValue;
  }
}
  return AlbumCoverFetcherSearch::eventFilter(arg__1, arg__2);
}
void PythonQtShell_AlbumCoverFetcherSearch::timerEvent(QTimerEvent*  arg__1)
{
if (_wrapper) {
  PyObject* obj = PyObject_GetAttrString((PyObject*)_wrapper, "timerEvent");
  PyErr_Clear();
  if (obj && !PythonQtSlotFunction_Check(obj)) {
    static const char* argumentList[] ={"" , "QTimerEvent*"};
    static const PythonQtMethodInfo* methodInfo = PythonQtMethodInfo::getCachedMethodInfoFromArgumentList(2, argumentList);
    void* args[2] = {NULL, (void*)&arg__1};
    PyObject* result = PythonQtSignalTarget::call(obj, methodInfo, args, true);
    if (result) { Py_DECREF(result); } 
    Py_DECREF(obj);
    return;
  }
}
  AlbumCoverFetcherSearch::timerEvent(arg__1);
}
void PythonQtWrapper_AlbumCoverFetcherSearch::Start(AlbumCoverFetcherSearch* theWrappedObject)
{
  ( theWrappedObject->Start());
}



QList<CoverSearchResult >  PythonQtShell_CoverProvider::ParseReply(QNetworkReply*  reply)
{
if (_wrapper) {
  PyObject* obj = PyObject_GetAttrString((PyObject*)_wrapper, "ParseReply");
  PyErr_Clear();
  if (obj && !PythonQtSlotFunction_Check(obj)) {
    static const char* argumentList[] ={"QList<CoverSearchResult >" , "QNetworkReply*"};
    static const PythonQtMethodInfo* methodInfo = PythonQtMethodInfo::getCachedMethodInfoFromArgumentList(2, argumentList);
      QList<CoverSearchResult > returnValue;
    void* args[2] = {NULL, (void*)&reply};
    PyObject* result = PythonQtSignalTarget::call(obj, methodInfo, args, true);
    if (result) {
      args[0] = PythonQtConv::ConvertPythonToQt(methodInfo->parameters().at(0), result, false, NULL, &returnValue);
      if (args[0]!=&returnValue) {
        if (args[0]==NULL) {
          PythonQt::priv()->handleVirtualOverloadReturnError("ParseReply", methodInfo, result);
        } else {
          returnValue = *((QList<CoverSearchResult >*)args[0]);
        }
      }
    }
    if (result) { Py_DECREF(result); } 
    Py_DECREF(obj);
    return returnValue;
  }
}
  return QList<CoverSearchResult >();
}
QNetworkReply*  PythonQtShell_CoverProvider::SendRequest(const QString&  query)
{
if (_wrapper) {
  PyObject* obj = PyObject_GetAttrString((PyObject*)_wrapper, "SendRequest");
  PyErr_Clear();
  if (obj && !PythonQtSlotFunction_Check(obj)) {
    static const char* argumentList[] ={"QNetworkReply*" , "const QString&"};
    static const PythonQtMethodInfo* methodInfo = PythonQtMethodInfo::getCachedMethodInfoFromArgumentList(2, argumentList);
      QNetworkReply* returnValue;
    void* args[2] = {NULL, (void*)&query};
    PyObject* result = PythonQtSignalTarget::call(obj, methodInfo, args, true);
    if (result) {
      args[0] = PythonQtConv::ConvertPythonToQt(methodInfo->parameters().at(0), result, false, NULL, &returnValue);
      if (args[0]!=&returnValue) {
        if (args[0]==NULL) {
          PythonQt::priv()->handleVirtualOverloadReturnError("SendRequest", methodInfo, result);
        } else {
          returnValue = *((QNetworkReply**)args[0]);
        }
      }
    }
    if (result) { Py_DECREF(result); } 
    Py_DECREF(obj);
    return returnValue;
  }
}
  return 0;
}
void PythonQtShell_CoverProvider::childEvent(QChildEvent*  arg__1)
{
if (_wrapper) {
  PyObject* obj = PyObject_GetAttrString((PyObject*)_wrapper, "childEvent");
  PyErr_Clear();
  if (obj && !PythonQtSlotFunction_Check(obj)) {
    static const char* argumentList[] ={"" , "QChildEvent*"};
    static const PythonQtMethodInfo* methodInfo = PythonQtMethodInfo::getCachedMethodInfoFromArgumentList(2, argumentList);
    void* args[2] = {NULL, (void*)&arg__1};
    PyObject* result = PythonQtSignalTarget::call(obj, methodInfo, args, true);
    if (result) { Py_DECREF(result); } 
    Py_DECREF(obj);
    return;
  }
}
  CoverProvider::childEvent(arg__1);
}
void PythonQtShell_CoverProvider::customEvent(QEvent*  arg__1)
{
if (_wrapper) {
  PyObject* obj = PyObject_GetAttrString((PyObject*)_wrapper, "customEvent");
  PyErr_Clear();
  if (obj && !PythonQtSlotFunction_Check(obj)) {
    static const char* argumentList[] ={"" , "QEvent*"};
    static const PythonQtMethodInfo* methodInfo = PythonQtMethodInfo::getCachedMethodInfoFromArgumentList(2, argumentList);
    void* args[2] = {NULL, (void*)&arg__1};
    PyObject* result = PythonQtSignalTarget::call(obj, methodInfo, args, true);
    if (result) { Py_DECREF(result); } 
    Py_DECREF(obj);
    return;
  }
}
  CoverProvider::customEvent(arg__1);
}
bool  PythonQtShell_CoverProvider::event(QEvent*  arg__1)
{
if (_wrapper) {
  PyObject* obj = PyObject_GetAttrString((PyObject*)_wrapper, "event");
  PyErr_Clear();
  if (obj && !PythonQtSlotFunction_Check(obj)) {
    static const char* argumentList[] ={"bool" , "QEvent*"};
    static const PythonQtMethodInfo* methodInfo = PythonQtMethodInfo::getCachedMethodInfoFromArgumentList(2, argumentList);
      bool returnValue;
    void* args[2] = {NULL, (void*)&arg__1};
    PyObject* result = PythonQtSignalTarget::call(obj, methodInfo, args, true);
    if (result) {
      args[0] = PythonQtConv::ConvertPythonToQt(methodInfo->parameters().at(0), result, false, NULL, &returnValue);
      if (args[0]!=&returnValue) {
        if (args[0]==NULL) {
          PythonQt::priv()->handleVirtualOverloadReturnError("event", methodInfo, result);
        } else {
          returnValue = *((bool*)args[0]);
        }
      }
    }
    if (result) { Py_DECREF(result); } 
    Py_DECREF(obj);
    return returnValue;
  }
}
  return CoverProvider::event(arg__1);
}
bool  PythonQtShell_CoverProvider::eventFilter(QObject*  arg__1, QEvent*  arg__2)
{
if (_wrapper) {
  PyObject* obj = PyObject_GetAttrString((PyObject*)_wrapper, "eventFilter");
  PyErr_Clear();
  if (obj && !PythonQtSlotFunction_Check(obj)) {
    static const char* argumentList[] ={"bool" , "QObject*" , "QEvent*"};
    static const PythonQtMethodInfo* methodInfo = PythonQtMethodInfo::getCachedMethodInfoFromArgumentList(3, argumentList);
      bool returnValue;
    void* args[3] = {NULL, (void*)&arg__1, (void*)&arg__2};
    PyObject* result = PythonQtSignalTarget::call(obj, methodInfo, args, true);
    if (result) {
      args[0] = PythonQtConv::ConvertPythonToQt(methodInfo->parameters().at(0), result, false, NULL, &returnValue);
      if (args[0]!=&returnValue) {
        if (args[0]==NULL) {
          PythonQt::priv()->handleVirtualOverloadReturnError("eventFilter", methodInfo, result);
        } else {
          returnValue = *((bool*)args[0]);
        }
      }
    }
    if (result) { Py_DECREF(result); } 
    Py_DECREF(obj);
    return returnValue;
  }
}
  return CoverProvider::eventFilter(arg__1, arg__2);
}
void PythonQtShell_CoverProvider::timerEvent(QTimerEvent*  arg__1)
{
if (_wrapper) {
  PyObject* obj = PyObject_GetAttrString((PyObject*)_wrapper, "timerEvent");
  PyErr_Clear();
  if (obj && !PythonQtSlotFunction_Check(obj)) {
    static const char* argumentList[] ={"" , "QTimerEvent*"};
    static const PythonQtMethodInfo* methodInfo = PythonQtMethodInfo::getCachedMethodInfoFromArgumentList(2, argumentList);
    void* args[2] = {NULL, (void*)&arg__1};
    PyObject* result = PythonQtSignalTarget::call(obj, methodInfo, args, true);
    if (result) { Py_DECREF(result); } 
    Py_DECREF(obj);
    return;
  }
}
  CoverProvider::timerEvent(arg__1);
}
CoverProvider* PythonQtWrapper_CoverProvider::new_CoverProvider(const QString&  name, QObject*  parent)
{ 
return new PythonQtShell_CoverProvider(name, parent); }

QString  PythonQtWrapper_CoverProvider::name(CoverProvider* theWrappedObject) const
{
  return ( theWrappedObject->name());
}



CoverProvider*  PythonQtShell_CoverProviderFactory::CreateCoverProvider(AlbumCoverFetcherSearch*  parent)
{
if (_wrapper) {
  PyObject* obj = PyObject_GetAttrString((PyObject*)_wrapper, "CreateCoverProvider");
  PyErr_Clear();
  if (obj && !PythonQtSlotFunction_Check(obj)) {
    static const char* argumentList[] ={"CoverProvider*" , "AlbumCoverFetcherSearch*"};
    static const PythonQtMethodInfo* methodInfo = PythonQtMethodInfo::getCachedMethodInfoFromArgumentList(2, argumentList);
      CoverProvider* returnValue;
    void* args[2] = {NULL, (void*)&parent};
    PyObject* result = PythonQtSignalTarget::call(obj, methodInfo, args, true);
    if (result) {
      args[0] = PythonQtConv::ConvertPythonToQt(methodInfo->parameters().at(0), result, false, NULL, &returnValue);
      if (args[0]!=&returnValue) {
        if (args[0]==NULL) {
          PythonQt::priv()->handleVirtualOverloadReturnError("CreateCoverProvider", methodInfo, result);
        } else {
          returnValue = *((CoverProvider**)args[0]);
        }
      }
    }
    if (result) { Py_DECREF(result); } 
    Py_DECREF(obj);
    return returnValue;
  }
}
  return 0;
}
void PythonQtShell_CoverProviderFactory::childEvent(QChildEvent*  arg__1)
{
if (_wrapper) {
  PyObject* obj = PyObject_GetAttrString((PyObject*)_wrapper, "childEvent");
  PyErr_Clear();
  if (obj && !PythonQtSlotFunction_Check(obj)) {
    static const char* argumentList[] ={"" , "QChildEvent*"};
    static const PythonQtMethodInfo* methodInfo = PythonQtMethodInfo::getCachedMethodInfoFromArgumentList(2, argumentList);
    void* args[2] = {NULL, (void*)&arg__1};
    PyObject* result = PythonQtSignalTarget::call(obj, methodInfo, args, true);
    if (result) { Py_DECREF(result); } 
    Py_DECREF(obj);
    return;
  }
}
  CoverProviderFactory::childEvent(arg__1);
}
void PythonQtShell_CoverProviderFactory::customEvent(QEvent*  arg__1)
{
if (_wrapper) {
  PyObject* obj = PyObject_GetAttrString((PyObject*)_wrapper, "customEvent");
  PyErr_Clear();
  if (obj && !PythonQtSlotFunction_Check(obj)) {
    static const char* argumentList[] ={"" , "QEvent*"};
    static const PythonQtMethodInfo* methodInfo = PythonQtMethodInfo::getCachedMethodInfoFromArgumentList(2, argumentList);
    void* args[2] = {NULL, (void*)&arg__1};
    PyObject* result = PythonQtSignalTarget::call(obj, methodInfo, args, true);
    if (result) { Py_DECREF(result); } 
    Py_DECREF(obj);
    return;
  }
}
  CoverProviderFactory::customEvent(arg__1);
}
bool  PythonQtShell_CoverProviderFactory::event(QEvent*  arg__1)
{
if (_wrapper) {
  PyObject* obj = PyObject_GetAttrString((PyObject*)_wrapper, "event");
  PyErr_Clear();
  if (obj && !PythonQtSlotFunction_Check(obj)) {
    static const char* argumentList[] ={"bool" , "QEvent*"};
    static const PythonQtMethodInfo* methodInfo = PythonQtMethodInfo::getCachedMethodInfoFromArgumentList(2, argumentList);
      bool returnValue;
    void* args[2] = {NULL, (void*)&arg__1};
    PyObject* result = PythonQtSignalTarget::call(obj, methodInfo, args, true);
    if (result) {
      args[0] = PythonQtConv::ConvertPythonToQt(methodInfo->parameters().at(0), result, false, NULL, &returnValue);
      if (args[0]!=&returnValue) {
        if (args[0]==NULL) {
          PythonQt::priv()->handleVirtualOverloadReturnError("event", methodInfo, result);
        } else {
          returnValue = *((bool*)args[0]);
        }
      }
    }
    if (result) { Py_DECREF(result); } 
    Py_DECREF(obj);
    return returnValue;
  }
}
  return CoverProviderFactory::event(arg__1);
}
bool  PythonQtShell_CoverProviderFactory::eventFilter(QObject*  arg__1, QEvent*  arg__2)
{
if (_wrapper) {
  PyObject* obj = PyObject_GetAttrString((PyObject*)_wrapper, "eventFilter");
  PyErr_Clear();
  if (obj && !PythonQtSlotFunction_Check(obj)) {
    static const char* argumentList[] ={"bool" , "QObject*" , "QEvent*"};
    static const PythonQtMethodInfo* methodInfo = PythonQtMethodInfo::getCachedMethodInfoFromArgumentList(3, argumentList);
      bool returnValue;
    void* args[3] = {NULL, (void*)&arg__1, (void*)&arg__2};
    PyObject* result = PythonQtSignalTarget::call(obj, methodInfo, args, true);
    if (result) {
      args[0] = PythonQtConv::ConvertPythonToQt(methodInfo->parameters().at(0), result, false, NULL, &returnValue);
      if (args[0]!=&returnValue) {
        if (args[0]==NULL) {
          PythonQt::priv()->handleVirtualOverloadReturnError("eventFilter", methodInfo, result);
        } else {
          returnValue = *((bool*)args[0]);
        }
      }
    }
    if (result) { Py_DECREF(result); } 
    Py_DECREF(obj);
    return returnValue;
  }
}
  return CoverProviderFactory::eventFilter(arg__1, arg__2);
}
void PythonQtShell_CoverProviderFactory::timerEvent(QTimerEvent*  arg__1)
{
if (_wrapper) {
  PyObject* obj = PyObject_GetAttrString((PyObject*)_wrapper, "timerEvent");
  PyErr_Clear();
  if (obj && !PythonQtSlotFunction_Check(obj)) {
    static const char* argumentList[] ={"" , "QTimerEvent*"};
    static const PythonQtMethodInfo* methodInfo = PythonQtMethodInfo::getCachedMethodInfoFromArgumentList(2, argumentList);
    void* args[2] = {NULL, (void*)&arg__1};
    PyObject* result = PythonQtSignalTarget::call(obj, methodInfo, args, true);
    if (result) { Py_DECREF(result); } 
    Py_DECREF(obj);
    return;
  }
}
  CoverProviderFactory::timerEvent(arg__1);
}
CoverProviderFactory* PythonQtWrapper_CoverProviderFactory::new_CoverProviderFactory(QObject*  parent)
{ 
return new PythonQtShell_CoverProviderFactory(parent); }



void PythonQtWrapper_CoverProviders::AddProviderFactory(CoverProviders* theWrappedObject, CoverProviderFactory*  factory)
{
  ( theWrappedObject->AddProviderFactory(factory));
}

bool  PythonQtWrapper_CoverProviders::HasAnyProviderFactories(CoverProviders* theWrappedObject)
{
  return ( theWrappedObject->HasAnyProviderFactories());
}

QList<CoverProvider* >  PythonQtWrapper_CoverProviders::List(CoverProviders* theWrappedObject, AlbumCoverFetcherSearch*  parent)
{
  return ( theWrappedObject->List(parent));
}

CoverProviders*  PythonQtWrapper_CoverProviders::static_CoverProviders_instance()
{
  return &(CoverProviders::instance());
}



CoverSearchResult* PythonQtWrapper_CoverSearchResult::new_CoverSearchResult()
{ 
return new PythonQtShell_CoverSearchResult(); }



Directory* PythonQtWrapper_Directory::new_Directory()
{ 
return new PythonQtShell_Directory(); }



void PythonQtShell_LibraryBackend::AddDirectory(const QString&  path)
{
if (_wrapper) {
  PyObject* obj = PyObject_GetAttrString((PyObject*)_wrapper, "AddDirectory");
  PyErr_Clear();
  if (obj && !PythonQtSlotFunction_Check(obj)) {
    static const char* argumentList[] ={"" , "const QString&"};
    static const PythonQtMethodInfo* methodInfo = PythonQtMethodInfo::getCachedMethodInfoFromArgumentList(2, argumentList);
    void* args[2] = {NULL, (void*)&path};
    PyObject* result = PythonQtSignalTarget::call(obj, methodInfo, args, true);
    if (result) { Py_DECREF(result); } 
    Py_DECREF(obj);
    return;
  }
}
  LibraryBackend::AddDirectory(path);
}
void PythonQtShell_LibraryBackend::ChangeDirPath(int  id, const QString&  old_path, const QString&  new_path)
{
if (_wrapper) {
  PyObject* obj = PyObject_GetAttrString((PyObject*)_wrapper, "ChangeDirPath");
  PyErr_Clear();
  if (obj && !PythonQtSlotFunction_Check(obj)) {
    static const char* argumentList[] ={"" , "int" , "const QString&" , "const QString&"};
    static const PythonQtMethodInfo* methodInfo = PythonQtMethodInfo::getCachedMethodInfoFromArgumentList(4, argumentList);
    void* args[4] = {NULL, (void*)&id, (void*)&old_path, (void*)&new_path};
    PyObject* result = PythonQtSignalTarget::call(obj, methodInfo, args, true);
    if (result) { Py_DECREF(result); } 
    Py_DECREF(obj);
    return;
  }
}
  LibraryBackend::ChangeDirPath(id, old_path, new_path);
}
bool  PythonQtShell_LibraryBackend::ExecQuery(LibraryQuery*  q)
{
if (_wrapper) {
  PyObject* obj = PyObject_GetAttrString((PyObject*)_wrapper, "ExecQuery");
  PyErr_Clear();
  if (obj && !PythonQtSlotFunction_Check(obj)) {
    static const char* argumentList[] ={"bool" , "LibraryQuery*"};
    static const PythonQtMethodInfo* methodInfo = PythonQtMethodInfo::getCachedMethodInfoFromArgumentList(2, argumentList);
      bool returnValue;
    void* args[2] = {NULL, (void*)&q};
    PyObject* result = PythonQtSignalTarget::call(obj, methodInfo, args, true);
    if (result) {
      args[0] = PythonQtConv::ConvertPythonToQt(methodInfo->parameters().at(0), result, false, NULL, &returnValue);
      if (args[0]!=&returnValue) {
        if (args[0]==NULL) {
          PythonQt::priv()->handleVirtualOverloadReturnError("ExecQuery", methodInfo, result);
        } else {
          returnValue = *((bool*)args[0]);
        }
      }
    }
    if (result) { Py_DECREF(result); } 
    Py_DECREF(obj);
    return returnValue;
  }
}
  return LibraryBackend::ExecQuery(q);
}
QList<Song >  PythonQtShell_LibraryBackend::FindSongsInDirectory(int  id)
{
if (_wrapper) {
  PyObject* obj = PyObject_GetAttrString((PyObject*)_wrapper, "FindSongsInDirectory");
  PyErr_Clear();
  if (obj && !PythonQtSlotFunction_Check(obj)) {
    static const char* argumentList[] ={"QList<Song >" , "int"};
    static const PythonQtMethodInfo* methodInfo = PythonQtMethodInfo::getCachedMethodInfoFromArgumentList(2, argumentList);
      QList<Song > returnValue;
    void* args[2] = {NULL, (void*)&id};
    PyObject* result = PythonQtSignalTarget::call(obj, methodInfo, args, true);
    if (result) {
      args[0] = PythonQtConv::ConvertPythonToQt(methodInfo->parameters().at(0), result, false, NULL, &returnValue);
      if (args[0]!=&returnValue) {
        if (args[0]==NULL) {
          PythonQt::priv()->handleVirtualOverloadReturnError("FindSongsInDirectory", methodInfo, result);
        } else {
          returnValue = *((QList<Song >*)args[0]);
        }
      }
    }
    if (result) { Py_DECREF(result); } 
    Py_DECREF(obj);
    return returnValue;
  }
}
  return LibraryBackend::FindSongsInDirectory(id);
}
LibraryBackendInterface::Album  PythonQtShell_LibraryBackend::GetAlbumArt(const QString&  artist, const QString&  album)
{
if (_wrapper) {
  PyObject* obj = PyObject_GetAttrString((PyObject*)_wrapper, "GetAlbumArt");
  PyErr_Clear();
  if (obj && !PythonQtSlotFunction_Check(obj)) {
    static const char* argumentList[] ={"LibraryBackendInterface::Album" , "const QString&" , "const QString&"};
    static const PythonQtMethodInfo* methodInfo = PythonQtMethodInfo::getCachedMethodInfoFromArgumentList(3, argumentList);
      LibraryBackendInterface::Album returnValue;
    void* args[3] = {NULL, (void*)&artist, (void*)&album};
    PyObject* result = PythonQtSignalTarget::call(obj, methodInfo, args, true);
    if (result) {
      args[0] = PythonQtConv::ConvertPythonToQt(methodInfo->parameters().at(0), result, false, NULL, &returnValue);
      if (args[0]!=&returnValue) {
        if (args[0]==NULL) {
          PythonQt::priv()->handleVirtualOverloadReturnError("GetAlbumArt", methodInfo, result);
        } else {
          returnValue = *((LibraryBackendInterface::Album*)args[0]);
        }
      }
    }
    if (result) { Py_DECREF(result); } 
    Py_DECREF(obj);
    return returnValue;
  }
}
  return LibraryBackend::GetAlbumArt(artist, album);
}
QList<LibraryBackendInterface::Album >  PythonQtShell_LibraryBackend::GetAlbumsByArtist(const QString&  artist, const QueryOptions&  opt)
{
if (_wrapper) {
  PyObject* obj = PyObject_GetAttrString((PyObject*)_wrapper, "GetAlbumsByArtist");
  PyErr_Clear();
  if (obj && !PythonQtSlotFunction_Check(obj)) {
    static const char* argumentList[] ={"QList<LibraryBackendInterface::Album >" , "const QString&" , "const QueryOptions&"};
    static const PythonQtMethodInfo* methodInfo = PythonQtMethodInfo::getCachedMethodInfoFromArgumentList(3, argumentList);
      QList<LibraryBackendInterface::Album > returnValue;
    void* args[3] = {NULL, (void*)&artist, (void*)&opt};
    PyObject* result = PythonQtSignalTarget::call(obj, methodInfo, args, true);
    if (result) {
      args[0] = PythonQtConv::ConvertPythonToQt(methodInfo->parameters().at(0), result, false, NULL, &returnValue);
      if (args[0]!=&returnValue) {
        if (args[0]==NULL) {
          PythonQt::priv()->handleVirtualOverloadReturnError("GetAlbumsByArtist", methodInfo, result);
        } else {
          returnValue = *((QList<LibraryBackendInterface::Album >*)args[0]);
        }
      }
    }
    if (result) { Py_DECREF(result); } 
    Py_DECREF(obj);
    return returnValue;
  }
}
  return LibraryBackend::GetAlbumsByArtist(artist, opt);
}
QList<LibraryBackendInterface::Album >  PythonQtShell_LibraryBackend::GetAllAlbums(const QueryOptions&  opt)
{
if (_wrapper) {
  PyObject* obj = PyObject_GetAttrString((PyObject*)_wrapper, "GetAllAlbums");
  PyErr_Clear();
  if (obj && !PythonQtSlotFunction_Check(obj)) {
    static const char* argumentList[] ={"QList<LibraryBackendInterface::Album >" , "const QueryOptions&"};
    static const PythonQtMethodInfo* methodInfo = PythonQtMethodInfo::getCachedMethodInfoFromArgumentList(2, argumentList);
      QList<LibraryBackendInterface::Album > returnValue;
    void* args[2] = {NULL, (void*)&opt};
    PyObject* result = PythonQtSignalTarget::call(obj, methodInfo, args, true);
    if (result) {
      args[0] = PythonQtConv::ConvertPythonToQt(methodInfo->parameters().at(0), result, false, NULL, &returnValue);
      if (args[0]!=&returnValue) {
        if (args[0]==NULL) {
          PythonQt::priv()->handleVirtualOverloadReturnError("GetAllAlbums", methodInfo, result);
        } else {
          returnValue = *((QList<LibraryBackendInterface::Album >*)args[0]);
        }
      }
    }
    if (result) { Py_DECREF(result); } 
    Py_DECREF(obj);
    return returnValue;
  }
}
  return LibraryBackend::GetAllAlbums(opt);
}
QStringList  PythonQtShell_LibraryBackend::GetAllArtists(const QueryOptions&  opt)
{
if (_wrapper) {
  PyObject* obj = PyObject_GetAttrString((PyObject*)_wrapper, "GetAllArtists");
  PyErr_Clear();
  if (obj && !PythonQtSlotFunction_Check(obj)) {
    static const char* argumentList[] ={"QStringList" , "const QueryOptions&"};
    static const PythonQtMethodInfo* methodInfo = PythonQtMethodInfo::getCachedMethodInfoFromArgumentList(2, argumentList);
      QStringList returnValue;
    void* args[2] = {NULL, (void*)&opt};
    PyObject* result = PythonQtSignalTarget::call(obj, methodInfo, args, true);
    if (result) {
      args[0] = PythonQtConv::ConvertPythonToQt(methodInfo->parameters().at(0), result, false, NULL, &returnValue);
      if (args[0]!=&returnValue) {
        if (args[0]==NULL) {
          PythonQt::priv()->handleVirtualOverloadReturnError("GetAllArtists", methodInfo, result);
        } else {
          returnValue = *((QStringList*)args[0]);
        }
      }
    }
    if (result) { Py_DECREF(result); } 
    Py_DECREF(obj);
    return returnValue;
  }
}
  return LibraryBackend::GetAllArtists(opt);
}
QStringList  PythonQtShell_LibraryBackend::GetAllArtistsWithAlbums(const QueryOptions&  opt)
{
if (_wrapper) {
  PyObject* obj = PyObject_GetAttrString((PyObject*)_wrapper, "GetAllArtistsWithAlbums");
  PyErr_Clear();
  if (obj && !PythonQtSlotFunction_Check(obj)) {
    static const char* argumentList[] ={"QStringList" , "const QueryOptions&"};
    static const PythonQtMethodInfo* methodInfo = PythonQtMethodInfo::getCachedMethodInfoFromArgumentList(2, argumentList);
      QStringList returnValue;
    void* args[2] = {NULL, (void*)&opt};
    PyObject* result = PythonQtSignalTarget::call(obj, methodInfo, args, true);
    if (result) {
      args[0] = PythonQtConv::ConvertPythonToQt(methodInfo->parameters().at(0), result, false, NULL, &returnValue);
      if (args[0]!=&returnValue) {
        if (args[0]==NULL) {
          PythonQt::priv()->handleVirtualOverloadReturnError("GetAllArtistsWithAlbums", methodInfo, result);
        } else {
          returnValue = *((QStringList*)args[0]);
        }
      }
    }
    if (result) { Py_DECREF(result); } 
    Py_DECREF(obj);
    return returnValue;
  }
}
  return LibraryBackend::GetAllArtistsWithAlbums(opt);
}
QList<Directory >  PythonQtShell_LibraryBackend::GetAllDirectories()
{
if (_wrapper) {
  PyObject* obj = PyObject_GetAttrString((PyObject*)_wrapper, "GetAllDirectories");
  PyErr_Clear();
  if (obj && !PythonQtSlotFunction_Check(obj)) {
    static const char* argumentList[] ={"QList<Directory >"};
    static const PythonQtMethodInfo* methodInfo = PythonQtMethodInfo::getCachedMethodInfoFromArgumentList(1, argumentList);
      QList<Directory > returnValue;
    void* args[1] = {NULL};
    PyObject* result = PythonQtSignalTarget::call(obj, methodInfo, args, true);
    if (result) {
      args[0] = PythonQtConv::ConvertPythonToQt(methodInfo->parameters().at(0), result, false, NULL, &returnValue);
      if (args[0]!=&returnValue) {
        if (args[0]==NULL) {
          PythonQt::priv()->handleVirtualOverloadReturnError("GetAllDirectories", methodInfo, result);
        } else {
          returnValue = *((QList<Directory >*)args[0]);
        }
      }
    }
    if (result) { Py_DECREF(result); } 
    Py_DECREF(obj);
    return returnValue;
  }
}
  return LibraryBackend::GetAllDirectories();
}
QList<LibraryBackendInterface::Album >  PythonQtShell_LibraryBackend::GetCompilationAlbums(const QueryOptions&  opt)
{
if (_wrapper) {
  PyObject* obj = PyObject_GetAttrString((PyObject*)_wrapper, "GetCompilationAlbums");
  PyErr_Clear();
  if (obj && !PythonQtSlotFunction_Check(obj)) {
    static const char* argumentList[] ={"QList<LibraryBackendInterface::Album >" , "const QueryOptions&"};
    static const PythonQtMethodInfo* methodInfo = PythonQtMethodInfo::getCachedMethodInfoFromArgumentList(2, argumentList);
      QList<LibraryBackendInterface::Album > returnValue;
    void* args[2] = {NULL, (void*)&opt};
    PyObject* result = PythonQtSignalTarget::call(obj, methodInfo, args, true);
    if (result) {
      args[0] = PythonQtConv::ConvertPythonToQt(methodInfo->parameters().at(0), result, false, NULL, &returnValue);
      if (args[0]!=&returnValue) {
        if (args[0]==NULL) {
          PythonQt::priv()->handleVirtualOverloadReturnError("GetCompilationAlbums", methodInfo, result);
        } else {
          returnValue = *((QList<LibraryBackendInterface::Album >*)args[0]);
        }
      }
    }
    if (result) { Py_DECREF(result); } 
    Py_DECREF(obj);
    return returnValue;
  }
}
  return LibraryBackend::GetCompilationAlbums(opt);
}
QList<Song >  PythonQtShell_LibraryBackend::GetCompilationSongs(const QString&  album, const QueryOptions&  opt)
{
if (_wrapper) {
  PyObject* obj = PyObject_GetAttrString((PyObject*)_wrapper, "GetCompilationSongs");
  PyErr_Clear();
  if (obj && !PythonQtSlotFunction_Check(obj)) {
    static const char* argumentList[] ={"QList<Song >" , "const QString&" , "const QueryOptions&"};
    static const PythonQtMethodInfo* methodInfo = PythonQtMethodInfo::getCachedMethodInfoFromArgumentList(3, argumentList);
      QList<Song > returnValue;
    void* args[3] = {NULL, (void*)&album, (void*)&opt};
    PyObject* result = PythonQtSignalTarget::call(obj, methodInfo, args, true);
    if (result) {
      args[0] = PythonQtConv::ConvertPythonToQt(methodInfo->parameters().at(0), result, false, NULL, &returnValue);
      if (args[0]!=&returnValue) {
        if (args[0]==NULL) {
          PythonQt::priv()->handleVirtualOverloadReturnError("GetCompilationSongs", methodInfo, result);
        } else {
          returnValue = *((QList<Song >*)args[0]);
        }
      }
    }
    if (result) { Py_DECREF(result); } 
    Py_DECREF(obj);
    return returnValue;
  }
}
  return LibraryBackend::GetCompilationSongs(album, opt);
}
Song  PythonQtShell_LibraryBackend::GetSongById(int  id)
{
if (_wrapper) {
  PyObject* obj = PyObject_GetAttrString((PyObject*)_wrapper, "GetSongById");
  PyErr_Clear();
  if (obj && !PythonQtSlotFunction_Check(obj)) {
    static const char* argumentList[] ={"Song" , "int"};
    static const PythonQtMethodInfo* methodInfo = PythonQtMethodInfo::getCachedMethodInfoFromArgumentList(2, argumentList);
      Song returnValue;
    void* args[2] = {NULL, (void*)&id};
    PyObject* result = PythonQtSignalTarget::call(obj, methodInfo, args, true);
    if (result) {
      args[0] = PythonQtConv::ConvertPythonToQt(methodInfo->parameters().at(0), result, false, NULL, &returnValue);
      if (args[0]!=&returnValue) {
        if (args[0]==NULL) {
          PythonQt::priv()->handleVirtualOverloadReturnError("GetSongById", methodInfo, result);
        } else {
          returnValue = *((Song*)args[0]);
        }
      }
    }
    if (result) { Py_DECREF(result); } 
    Py_DECREF(obj);
    return returnValue;
  }
}
  return LibraryBackend::GetSongById(id);
}
Song  PythonQtShell_LibraryBackend::GetSongByUrl(const QUrl&  url, qint64  beginning)
{
if (_wrapper) {
  PyObject* obj = PyObject_GetAttrString((PyObject*)_wrapper, "GetSongByUrl");
  PyErr_Clear();
  if (obj && !PythonQtSlotFunction_Check(obj)) {
    static const char* argumentList[] ={"Song" , "const QUrl&" , "qint64"};
    static const PythonQtMethodInfo* methodInfo = PythonQtMethodInfo::getCachedMethodInfoFromArgumentList(3, argumentList);
      Song returnValue;
    void* args[3] = {NULL, (void*)&url, (void*)&beginning};
    PyObject* result = PythonQtSignalTarget::call(obj, methodInfo, args, true);
    if (result) {
      args[0] = PythonQtConv::ConvertPythonToQt(methodInfo->parameters().at(0), result, false, NULL, &returnValue);
      if (args[0]!=&returnValue) {
        if (args[0]==NULL) {
          PythonQt::priv()->handleVirtualOverloadReturnError("GetSongByUrl", methodInfo, result);
        } else {
          returnValue = *((Song*)args[0]);
        }
      }
    }
    if (result) { Py_DECREF(result); } 
    Py_DECREF(obj);
    return returnValue;
  }
}
  return LibraryBackend::GetSongByUrl(url, beginning);
}
QList<Song >  PythonQtShell_LibraryBackend::GetSongs(const QString&  artist, const QString&  album, const QueryOptions&  opt)
{
if (_wrapper) {
  PyObject* obj = PyObject_GetAttrString((PyObject*)_wrapper, "GetSongs");
  PyErr_Clear();
  if (obj && !PythonQtSlotFunction_Check(obj)) {
    static const char* argumentList[] ={"QList<Song >" , "const QString&" , "const QString&" , "const QueryOptions&"};
    static const PythonQtMethodInfo* methodInfo = PythonQtMethodInfo::getCachedMethodInfoFromArgumentList(4, argumentList);
      QList<Song > returnValue;
    void* args[4] = {NULL, (void*)&artist, (void*)&album, (void*)&opt};
    PyObject* result = PythonQtSignalTarget::call(obj, methodInfo, args, true);
    if (result) {
      args[0] = PythonQtConv::ConvertPythonToQt(methodInfo->parameters().at(0), result, false, NULL, &returnValue);
      if (args[0]!=&returnValue) {
        if (args[0]==NULL) {
          PythonQt::priv()->handleVirtualOverloadReturnError("GetSongs", methodInfo, result);
        } else {
          returnValue = *((QList<Song >*)args[0]);
        }
      }
    }
    if (result) { Py_DECREF(result); } 
    Py_DECREF(obj);
    return returnValue;
  }
}
  return LibraryBackend::GetSongs(artist, album, opt);
}
QList<Song >  PythonQtShell_LibraryBackend::GetSongsByUrl(const QUrl&  url)
{
if (_wrapper) {
  PyObject* obj = PyObject_GetAttrString((PyObject*)_wrapper, "GetSongsByUrl");
  PyErr_Clear();
  if (obj && !PythonQtSlotFunction_Check(obj)) {
    static const char* argumentList[] ={"QList<Song >" , "const QUrl&"};
    static const PythonQtMethodInfo* methodInfo = PythonQtMethodInfo::getCachedMethodInfoFromArgumentList(2, argumentList);
      QList<Song > returnValue;
    void* args[2] = {NULL, (void*)&url};
    PyObject* result = PythonQtSignalTarget::call(obj, methodInfo, args, true);
    if (result) {
      args[0] = PythonQtConv::ConvertPythonToQt(methodInfo->parameters().at(0), result, false, NULL, &returnValue);
      if (args[0]!=&returnValue) {
        if (args[0]==NULL) {
          PythonQt::priv()->handleVirtualOverloadReturnError("GetSongsByUrl", methodInfo, result);
        } else {
          returnValue = *((QList<Song >*)args[0]);
        }
      }
    }
    if (result) { Py_DECREF(result); } 
    Py_DECREF(obj);
    return returnValue;
  }
}
  return LibraryBackend::GetSongsByUrl(url);
}
bool  PythonQtShell_LibraryBackend::HasCompilations(const QueryOptions&  opt)
{
if (_wrapper) {
  PyObject* obj = PyObject_GetAttrString((PyObject*)_wrapper, "HasCompilations");
  PyErr_Clear();
  if (obj && !PythonQtSlotFunction_Check(obj)) {
    static const char* argumentList[] ={"bool" , "const QueryOptions&"};
    static const PythonQtMethodInfo* methodInfo = PythonQtMethodInfo::getCachedMethodInfoFromArgumentList(2, argumentList);
      bool returnValue;
    void* args[2] = {NULL, (void*)&opt};
    PyObject* result = PythonQtSignalTarget::call(obj, methodInfo, args, true);
    if (result) {
      args[0] = PythonQtConv::ConvertPythonToQt(methodInfo->parameters().at(0), result, false, NULL, &returnValue);
      if (args[0]!=&returnValue) {
        if (args[0]==NULL) {
          PythonQt::priv()->handleVirtualOverloadReturnError("HasCompilations", methodInfo, result);
        } else {
          returnValue = *((bool*)args[0]);
        }
      }
    }
    if (result) { Py_DECREF(result); } 
    Py_DECREF(obj);
    return returnValue;
  }
}
  return LibraryBackend::HasCompilations(opt);
}
void PythonQtShell_LibraryBackend::LoadDirectoriesAsync()
{
if (_wrapper) {
  PyObject* obj = PyObject_GetAttrString((PyObject*)_wrapper, "LoadDirectoriesAsync");
  PyErr_Clear();
  if (obj && !PythonQtSlotFunction_Check(obj)) {
    static const char* argumentList[] ={""};
    static const PythonQtMethodInfo* methodInfo = PythonQtMethodInfo::getCachedMethodInfoFromArgumentList(1, argumentList);
    void* args[1] = {NULL};
    PyObject* result = PythonQtSignalTarget::call(obj, methodInfo, args, true);
    if (result) { Py_DECREF(result); } 
    Py_DECREF(obj);
    return;
  }
}
  LibraryBackend::LoadDirectoriesAsync();
}
void PythonQtShell_LibraryBackend::RemoveDirectory(const Directory&  dir)
{
if (_wrapper) {
  PyObject* obj = PyObject_GetAttrString((PyObject*)_wrapper, "RemoveDirectory");
  PyErr_Clear();
  if (obj && !PythonQtSlotFunction_Check(obj)) {
    static const char* argumentList[] ={"" , "const Directory&"};
    static const PythonQtMethodInfo* methodInfo = PythonQtMethodInfo::getCachedMethodInfoFromArgumentList(2, argumentList);
    void* args[2] = {NULL, (void*)&dir};
    PyObject* result = PythonQtSignalTarget::call(obj, methodInfo, args, true);
    if (result) { Py_DECREF(result); } 
    Py_DECREF(obj);
    return;
  }
}
  LibraryBackend::RemoveDirectory(dir);
}
QList<Subdirectory >  PythonQtShell_LibraryBackend::SubdirsInDirectory(int  id)
{
if (_wrapper) {
  PyObject* obj = PyObject_GetAttrString((PyObject*)_wrapper, "SubdirsInDirectory");
  PyErr_Clear();
  if (obj && !PythonQtSlotFunction_Check(obj)) {
    static const char* argumentList[] ={"QList<Subdirectory >" , "int"};
    static const PythonQtMethodInfo* methodInfo = PythonQtMethodInfo::getCachedMethodInfoFromArgumentList(2, argumentList);
      QList<Subdirectory > returnValue;
    void* args[2] = {NULL, (void*)&id};
    PyObject* result = PythonQtSignalTarget::call(obj, methodInfo, args, true);
    if (result) {
      args[0] = PythonQtConv::ConvertPythonToQt(methodInfo->parameters().at(0), result, false, NULL, &returnValue);
      if (args[0]!=&returnValue) {
        if (args[0]==NULL) {
          PythonQt::priv()->handleVirtualOverloadReturnError("SubdirsInDirectory", methodInfo, result);
        } else {
          returnValue = *((QList<Subdirectory >*)args[0]);
        }
      }
    }
    if (result) { Py_DECREF(result); } 
    Py_DECREF(obj);
    return returnValue;
  }
}
  return LibraryBackend::SubdirsInDirectory(id);
}
void PythonQtShell_LibraryBackend::UpdateManualAlbumArtAsync(const QString&  artist, const QString&  album, const QString&  art)
{
if (_wrapper) {
  PyObject* obj = PyObject_GetAttrString((PyObject*)_wrapper, "UpdateManualAlbumArtAsync");
  PyErr_Clear();
  if (obj && !PythonQtSlotFunction_Check(obj)) {
    static const char* argumentList[] ={"" , "const QString&" , "const QString&" , "const QString&"};
    static const PythonQtMethodInfo* methodInfo = PythonQtMethodInfo::getCachedMethodInfoFromArgumentList(4, argumentList);
    void* args[4] = {NULL, (void*)&artist, (void*)&album, (void*)&art};
    PyObject* result = PythonQtSignalTarget::call(obj, methodInfo, args, true);
    if (result) { Py_DECREF(result); } 
    Py_DECREF(obj);
    return;
  }
}
  LibraryBackend::UpdateManualAlbumArtAsync(artist, album, art);
}
void PythonQtShell_LibraryBackend::UpdateTotalSongCountAsync()
{
if (_wrapper) {
  PyObject* obj = PyObject_GetAttrString((PyObject*)_wrapper, "UpdateTotalSongCountAsync");
  PyErr_Clear();
  if (obj && !PythonQtSlotFunction_Check(obj)) {
    static const char* argumentList[] ={""};
    static const PythonQtMethodInfo* methodInfo = PythonQtMethodInfo::getCachedMethodInfoFromArgumentList(1, argumentList);
    void* args[1] = {NULL};
    PyObject* result = PythonQtSignalTarget::call(obj, methodInfo, args, true);
    if (result) { Py_DECREF(result); } 
    Py_DECREF(obj);
    return;
  }
}
  LibraryBackend::UpdateTotalSongCountAsync();
}
void PythonQtShell_LibraryBackend::childEvent(QChildEvent*  arg__1)
{
if (_wrapper) {
  PyObject* obj = PyObject_GetAttrString((PyObject*)_wrapper, "childEvent");
  PyErr_Clear();
  if (obj && !PythonQtSlotFunction_Check(obj)) {
    static const char* argumentList[] ={"" , "QChildEvent*"};
    static const PythonQtMethodInfo* methodInfo = PythonQtMethodInfo::getCachedMethodInfoFromArgumentList(2, argumentList);
    void* args[2] = {NULL, (void*)&arg__1};
    PyObject* result = PythonQtSignalTarget::call(obj, methodInfo, args, true);
    if (result) { Py_DECREF(result); } 
    Py_DECREF(obj);
    return;
  }
}
  LibraryBackend::childEvent(arg__1);
}
void PythonQtShell_LibraryBackend::customEvent(QEvent*  arg__1)
{
if (_wrapper) {
  PyObject* obj = PyObject_GetAttrString((PyObject*)_wrapper, "customEvent");
  PyErr_Clear();
  if (obj && !PythonQtSlotFunction_Check(obj)) {
    static const char* argumentList[] ={"" , "QEvent*"};
    static const PythonQtMethodInfo* methodInfo = PythonQtMethodInfo::getCachedMethodInfoFromArgumentList(2, argumentList);
    void* args[2] = {NULL, (void*)&arg__1};
    PyObject* result = PythonQtSignalTarget::call(obj, methodInfo, args, true);
    if (result) { Py_DECREF(result); } 
    Py_DECREF(obj);
    return;
  }
}
  LibraryBackend::customEvent(arg__1);
}
bool  PythonQtShell_LibraryBackend::event(QEvent*  arg__1)
{
if (_wrapper) {
  PyObject* obj = PyObject_GetAttrString((PyObject*)_wrapper, "event");
  PyErr_Clear();
  if (obj && !PythonQtSlotFunction_Check(obj)) {
    static const char* argumentList[] ={"bool" , "QEvent*"};
    static const PythonQtMethodInfo* methodInfo = PythonQtMethodInfo::getCachedMethodInfoFromArgumentList(2, argumentList);
      bool returnValue;
    void* args[2] = {NULL, (void*)&arg__1};
    PyObject* result = PythonQtSignalTarget::call(obj, methodInfo, args, true);
    if (result) {
      args[0] = PythonQtConv::ConvertPythonToQt(methodInfo->parameters().at(0), result, false, NULL, &returnValue);
      if (args[0]!=&returnValue) {
        if (args[0]==NULL) {
          PythonQt::priv()->handleVirtualOverloadReturnError("event", methodInfo, result);
        } else {
          returnValue = *((bool*)args[0]);
        }
      }
    }
    if (result) { Py_DECREF(result); } 
    Py_DECREF(obj);
    return returnValue;
  }
}
  return LibraryBackend::event(arg__1);
}
bool  PythonQtShell_LibraryBackend::eventFilter(QObject*  arg__1, QEvent*  arg__2)
{
if (_wrapper) {
  PyObject* obj = PyObject_GetAttrString((PyObject*)_wrapper, "eventFilter");
  PyErr_Clear();
  if (obj && !PythonQtSlotFunction_Check(obj)) {
    static const char* argumentList[] ={"bool" , "QObject*" , "QEvent*"};
    static const PythonQtMethodInfo* methodInfo = PythonQtMethodInfo::getCachedMethodInfoFromArgumentList(3, argumentList);
      bool returnValue;
    void* args[3] = {NULL, (void*)&arg__1, (void*)&arg__2};
    PyObject* result = PythonQtSignalTarget::call(obj, methodInfo, args, true);
    if (result) {
      args[0] = PythonQtConv::ConvertPythonToQt(methodInfo->parameters().at(0), result, false, NULL, &returnValue);
      if (args[0]!=&returnValue) {
        if (args[0]==NULL) {
          PythonQt::priv()->handleVirtualOverloadReturnError("eventFilter", methodInfo, result);
        } else {
          returnValue = *((bool*)args[0]);
        }
      }
    }
    if (result) { Py_DECREF(result); } 
    Py_DECREF(obj);
    return returnValue;
  }
}
  return LibraryBackend::eventFilter(arg__1, arg__2);
}
void PythonQtShell_LibraryBackend::timerEvent(QTimerEvent*  arg__1)
{
if (_wrapper) {
  PyObject* obj = PyObject_GetAttrString((PyObject*)_wrapper, "timerEvent");
  PyErr_Clear();
  if (obj && !PythonQtSlotFunction_Check(obj)) {
    static const char* argumentList[] ={"" , "QTimerEvent*"};
    static const PythonQtMethodInfo* methodInfo = PythonQtMethodInfo::getCachedMethodInfoFromArgumentList(2, argumentList);
    void* args[2] = {NULL, (void*)&arg__1};
    PyObject* result = PythonQtSignalTarget::call(obj, methodInfo, args, true);
    if (result) { Py_DECREF(result); } 
    Py_DECREF(obj);
    return;
  }
}
  LibraryBackend::timerEvent(arg__1);
}
LibraryBackend* PythonQtWrapper_LibraryBackend::new_LibraryBackend(QObject*  parent)
{ 
return new PythonQtShell_LibraryBackend(parent); }

void PythonQtWrapper_LibraryBackend::AddDirectory(LibraryBackend* theWrappedObject, const QString&  path)
{
  ( ((PythonQtPublicPromoter_LibraryBackend*)theWrappedObject)->promoted_AddDirectory(path));
}

void PythonQtWrapper_LibraryBackend::ChangeDirPath(LibraryBackend* theWrappedObject, int  id, const QString&  old_path, const QString&  new_path)
{
  ( ((PythonQtPublicPromoter_LibraryBackend*)theWrappedObject)->promoted_ChangeDirPath(id, old_path, new_path));
}

void PythonQtWrapper_LibraryBackend::DeleteAll(LibraryBackend* theWrappedObject)
{
  ( theWrappedObject->DeleteAll());
}

bool  PythonQtWrapper_LibraryBackend::ExecQuery(LibraryBackend* theWrappedObject, LibraryQuery*  q)
{
  return ( ((PythonQtPublicPromoter_LibraryBackend*)theWrappedObject)->promoted_ExecQuery(q));
}

QList<Song >  PythonQtWrapper_LibraryBackend::FindSongsInDirectory(LibraryBackend* theWrappedObject, int  id)
{
  return ( ((PythonQtPublicPromoter_LibraryBackend*)theWrappedObject)->promoted_FindSongsInDirectory(id));
}

LibraryBackendInterface::Album  PythonQtWrapper_LibraryBackend::GetAlbumArt(LibraryBackend* theWrappedObject, const QString&  artist, const QString&  album)
{
  return ( ((PythonQtPublicPromoter_LibraryBackend*)theWrappedObject)->promoted_GetAlbumArt(artist, album));
}

QList<LibraryBackendInterface::Album >  PythonQtWrapper_LibraryBackend::GetAlbumsByArtist(LibraryBackend* theWrappedObject, const QString&  artist, const QueryOptions&  opt)
{
  return ( ((PythonQtPublicPromoter_LibraryBackend*)theWrappedObject)->promoted_GetAlbumsByArtist(artist, opt));
}

QStringList  PythonQtWrapper_LibraryBackend::GetAll(LibraryBackend* theWrappedObject, const QString&  column, const QueryOptions&  opt)
{
  return ( theWrappedObject->GetAll(column, opt));
}

QList<LibraryBackendInterface::Album >  PythonQtWrapper_LibraryBackend::GetAllAlbums(LibraryBackend* theWrappedObject, const QueryOptions&  opt)
{
  return ( ((PythonQtPublicPromoter_LibraryBackend*)theWrappedObject)->promoted_GetAllAlbums(opt));
}

QStringList  PythonQtWrapper_LibraryBackend::GetAllArtists(LibraryBackend* theWrappedObject, const QueryOptions&  opt)
{
  return ( ((PythonQtPublicPromoter_LibraryBackend*)theWrappedObject)->promoted_GetAllArtists(opt));
}

QStringList  PythonQtWrapper_LibraryBackend::GetAllArtistsWithAlbums(LibraryBackend* theWrappedObject, const QueryOptions&  opt)
{
  return ( ((PythonQtPublicPromoter_LibraryBackend*)theWrappedObject)->promoted_GetAllArtistsWithAlbums(opt));
}

QList<Directory >  PythonQtWrapper_LibraryBackend::GetAllDirectories(LibraryBackend* theWrappedObject)
{
  return ( ((PythonQtPublicPromoter_LibraryBackend*)theWrappedObject)->promoted_GetAllDirectories());
}

QList<LibraryBackendInterface::Album >  PythonQtWrapper_LibraryBackend::GetCompilationAlbums(LibraryBackend* theWrappedObject, const QueryOptions&  opt)
{
  return ( ((PythonQtPublicPromoter_LibraryBackend*)theWrappedObject)->promoted_GetCompilationAlbums(opt));
}

QList<Song >  PythonQtWrapper_LibraryBackend::GetCompilationSongs(LibraryBackend* theWrappedObject, const QString&  album, const QueryOptions&  opt)
{
  return ( ((PythonQtPublicPromoter_LibraryBackend*)theWrappedObject)->promoted_GetCompilationSongs(album, opt));
}

Song  PythonQtWrapper_LibraryBackend::GetSongById(LibraryBackend* theWrappedObject, int  id)
{
  return ( ((PythonQtPublicPromoter_LibraryBackend*)theWrappedObject)->promoted_GetSongById(id));
}

Song  PythonQtWrapper_LibraryBackend::GetSongByUrl(LibraryBackend* theWrappedObject, const QUrl&  url, qint64  beginning)
{
  return ( ((PythonQtPublicPromoter_LibraryBackend*)theWrappedObject)->promoted_GetSongByUrl(url, beginning));
}

QList<Song >  PythonQtWrapper_LibraryBackend::GetSongs(LibraryBackend* theWrappedObject, const QString&  artist, const QString&  album, const QueryOptions&  opt)
{
  return ( ((PythonQtPublicPromoter_LibraryBackend*)theWrappedObject)->promoted_GetSongs(artist, album, opt));
}

QList<Song >  PythonQtWrapper_LibraryBackend::GetSongsByForeignId(LibraryBackend* theWrappedObject, const QStringList&  ids, const QString&  table, const QString&  column)
{
  return ( theWrappedObject->GetSongsByForeignId(ids, table, column));
}

QList<Song >  PythonQtWrapper_LibraryBackend::GetSongsById(LibraryBackend* theWrappedObject, const QList<int >&  ids)
{
  return ( theWrappedObject->GetSongsById(ids));
}

QList<Song >  PythonQtWrapper_LibraryBackend::GetSongsById(LibraryBackend* theWrappedObject, const QStringList&  ids)
{
  return ( theWrappedObject->GetSongsById(ids));
}

QList<Song >  PythonQtWrapper_LibraryBackend::GetSongsByUrl(LibraryBackend* theWrappedObject, const QUrl&  url)
{
  return ( ((PythonQtPublicPromoter_LibraryBackend*)theWrappedObject)->promoted_GetSongsByUrl(url));
}

bool  PythonQtWrapper_LibraryBackend::HasCompilations(LibraryBackend* theWrappedObject, const QueryOptions&  opt)
{
  return ( ((PythonQtPublicPromoter_LibraryBackend*)theWrappedObject)->promoted_HasCompilations(opt));
}

void PythonQtWrapper_LibraryBackend::IncrementPlayCountAsync(LibraryBackend* theWrappedObject, int  id)
{
  ( theWrappedObject->IncrementPlayCountAsync(id));
}

void PythonQtWrapper_LibraryBackend::IncrementSkipCountAsync(LibraryBackend* theWrappedObject, int  id, float  progress)
{
  ( theWrappedObject->IncrementSkipCountAsync(id, progress));
}

void PythonQtWrapper_LibraryBackend::LoadDirectoriesAsync(LibraryBackend* theWrappedObject)
{
  ( ((PythonQtPublicPromoter_LibraryBackend*)theWrappedObject)->promoted_LoadDirectoriesAsync());
}

void PythonQtWrapper_LibraryBackend::RemoveDirectory(LibraryBackend* theWrappedObject, const Directory&  dir)
{
  ( ((PythonQtPublicPromoter_LibraryBackend*)theWrappedObject)->promoted_RemoveDirectory(dir));
}

void PythonQtWrapper_LibraryBackend::ResetStatisticsAsync(LibraryBackend* theWrappedObject, int  id)
{
  ( theWrappedObject->ResetStatisticsAsync(id));
}

QList<Subdirectory >  PythonQtWrapper_LibraryBackend::SubdirsInDirectory(LibraryBackend* theWrappedObject, int  id)
{
  return ( ((PythonQtPublicPromoter_LibraryBackend*)theWrappedObject)->promoted_SubdirsInDirectory(id));
}

void PythonQtWrapper_LibraryBackend::UpdateManualAlbumArtAsync(LibraryBackend* theWrappedObject, const QString&  artist, const QString&  album, const QString&  art)
{
  ( ((PythonQtPublicPromoter_LibraryBackend*)theWrappedObject)->promoted_UpdateManualAlbumArtAsync(artist, album, art));
}

void PythonQtWrapper_LibraryBackend::UpdateSongRatingAsync(LibraryBackend* theWrappedObject, int  id, float  rating)
{
  ( theWrappedObject->UpdateSongRatingAsync(id, rating));
}

void PythonQtWrapper_LibraryBackend::UpdateTotalSongCountAsync(LibraryBackend* theWrappedObject)
{
  ( ((PythonQtPublicPromoter_LibraryBackend*)theWrappedObject)->promoted_UpdateTotalSongCountAsync());
}

QString  PythonQtWrapper_LibraryBackend::dirs_table(LibraryBackend* theWrappedObject) const
{
  return ( theWrappedObject->dirs_table());
}

QString  PythonQtWrapper_LibraryBackend::songs_table(LibraryBackend* theWrappedObject) const
{
  return ( theWrappedObject->songs_table());
}

QString  PythonQtWrapper_LibraryBackend::subdirs_table(LibraryBackend* theWrappedObject) const
{
  return ( theWrappedObject->subdirs_table());
}



void PythonQtShell_LibraryBackendInterface::AddDirectory(const QString&  path)
{
if (_wrapper) {
  PyObject* obj = PyObject_GetAttrString((PyObject*)_wrapper, "AddDirectory");
  PyErr_Clear();
  if (obj && !PythonQtSlotFunction_Check(obj)) {
    static const char* argumentList[] ={"" , "const QString&"};
    static const PythonQtMethodInfo* methodInfo = PythonQtMethodInfo::getCachedMethodInfoFromArgumentList(2, argumentList);
    void* args[2] = {NULL, (void*)&path};
    PyObject* result = PythonQtSignalTarget::call(obj, methodInfo, args, true);
    if (result) { Py_DECREF(result); } 
    Py_DECREF(obj);
    return;
  }
}
  
}
void PythonQtShell_LibraryBackendInterface::ChangeDirPath(int  id, const QString&  old_path, const QString&  new_path)
{
if (_wrapper) {
  PyObject* obj = PyObject_GetAttrString((PyObject*)_wrapper, "ChangeDirPath");
  PyErr_Clear();
  if (obj && !PythonQtSlotFunction_Check(obj)) {
    static const char* argumentList[] ={"" , "int" , "const QString&" , "const QString&"};
    static const PythonQtMethodInfo* methodInfo = PythonQtMethodInfo::getCachedMethodInfoFromArgumentList(4, argumentList);
    void* args[4] = {NULL, (void*)&id, (void*)&old_path, (void*)&new_path};
    PyObject* result = PythonQtSignalTarget::call(obj, methodInfo, args, true);
    if (result) { Py_DECREF(result); } 
    Py_DECREF(obj);
    return;
  }
}
  
}
bool  PythonQtShell_LibraryBackendInterface::ExecQuery(LibraryQuery*  q)
{
if (_wrapper) {
  PyObject* obj = PyObject_GetAttrString((PyObject*)_wrapper, "ExecQuery");
  PyErr_Clear();
  if (obj && !PythonQtSlotFunction_Check(obj)) {
    static const char* argumentList[] ={"bool" , "LibraryQuery*"};
    static const PythonQtMethodInfo* methodInfo = PythonQtMethodInfo::getCachedMethodInfoFromArgumentList(2, argumentList);
      bool returnValue;
    void* args[2] = {NULL, (void*)&q};
    PyObject* result = PythonQtSignalTarget::call(obj, methodInfo, args, true);
    if (result) {
      args[0] = PythonQtConv::ConvertPythonToQt(methodInfo->parameters().at(0), result, false, NULL, &returnValue);
      if (args[0]!=&returnValue) {
        if (args[0]==NULL) {
          PythonQt::priv()->handleVirtualOverloadReturnError("ExecQuery", methodInfo, result);
        } else {
          returnValue = *((bool*)args[0]);
        }
      }
    }
    if (result) { Py_DECREF(result); } 
    Py_DECREF(obj);
    return returnValue;
  }
}
  return bool();
}
QList<Song >  PythonQtShell_LibraryBackendInterface::FindSongsInDirectory(int  id)
{
if (_wrapper) {
  PyObject* obj = PyObject_GetAttrString((PyObject*)_wrapper, "FindSongsInDirectory");
  PyErr_Clear();
  if (obj && !PythonQtSlotFunction_Check(obj)) {
    static const char* argumentList[] ={"QList<Song >" , "int"};
    static const PythonQtMethodInfo* methodInfo = PythonQtMethodInfo::getCachedMethodInfoFromArgumentList(2, argumentList);
      QList<Song > returnValue;
    void* args[2] = {NULL, (void*)&id};
    PyObject* result = PythonQtSignalTarget::call(obj, methodInfo, args, true);
    if (result) {
      args[0] = PythonQtConv::ConvertPythonToQt(methodInfo->parameters().at(0), result, false, NULL, &returnValue);
      if (args[0]!=&returnValue) {
        if (args[0]==NULL) {
          PythonQt::priv()->handleVirtualOverloadReturnError("FindSongsInDirectory", methodInfo, result);
        } else {
          returnValue = *((QList<Song >*)args[0]);
        }
      }
    }
    if (result) { Py_DECREF(result); } 
    Py_DECREF(obj);
    return returnValue;
  }
}
  return QList<Song >();
}
LibraryBackendInterface::Album  PythonQtShell_LibraryBackendInterface::GetAlbumArt(const QString&  artist, const QString&  album)
{
if (_wrapper) {
  PyObject* obj = PyObject_GetAttrString((PyObject*)_wrapper, "GetAlbumArt");
  PyErr_Clear();
  if (obj && !PythonQtSlotFunction_Check(obj)) {
    static const char* argumentList[] ={"LibraryBackendInterface::Album" , "const QString&" , "const QString&"};
    static const PythonQtMethodInfo* methodInfo = PythonQtMethodInfo::getCachedMethodInfoFromArgumentList(3, argumentList);
      LibraryBackendInterface::Album returnValue;
    void* args[3] = {NULL, (void*)&artist, (void*)&album};
    PyObject* result = PythonQtSignalTarget::call(obj, methodInfo, args, true);
    if (result) {
      args[0] = PythonQtConv::ConvertPythonToQt(methodInfo->parameters().at(0), result, false, NULL, &returnValue);
      if (args[0]!=&returnValue) {
        if (args[0]==NULL) {
          PythonQt::priv()->handleVirtualOverloadReturnError("GetAlbumArt", methodInfo, result);
        } else {
          returnValue = *((LibraryBackendInterface::Album*)args[0]);
        }
      }
    }
    if (result) { Py_DECREF(result); } 
    Py_DECREF(obj);
    return returnValue;
  }
}
  return LibraryBackendInterface::Album();
}
QList<LibraryBackendInterface::Album >  PythonQtShell_LibraryBackendInterface::GetAlbumsByArtist(const QString&  artist, const QueryOptions&  opt)
{
if (_wrapper) {
  PyObject* obj = PyObject_GetAttrString((PyObject*)_wrapper, "GetAlbumsByArtist");
  PyErr_Clear();
  if (obj && !PythonQtSlotFunction_Check(obj)) {
    static const char* argumentList[] ={"QList<LibraryBackendInterface::Album >" , "const QString&" , "const QueryOptions&"};
    static const PythonQtMethodInfo* methodInfo = PythonQtMethodInfo::getCachedMethodInfoFromArgumentList(3, argumentList);
      QList<LibraryBackendInterface::Album > returnValue;
    void* args[3] = {NULL, (void*)&artist, (void*)&opt};
    PyObject* result = PythonQtSignalTarget::call(obj, methodInfo, args, true);
    if (result) {
      args[0] = PythonQtConv::ConvertPythonToQt(methodInfo->parameters().at(0), result, false, NULL, &returnValue);
      if (args[0]!=&returnValue) {
        if (args[0]==NULL) {
          PythonQt::priv()->handleVirtualOverloadReturnError("GetAlbumsByArtist", methodInfo, result);
        } else {
          returnValue = *((QList<LibraryBackendInterface::Album >*)args[0]);
        }
      }
    }
    if (result) { Py_DECREF(result); } 
    Py_DECREF(obj);
    return returnValue;
  }
}
  return QList<LibraryBackendInterface::Album >();
}
QList<LibraryBackendInterface::Album >  PythonQtShell_LibraryBackendInterface::GetAllAlbums(const QueryOptions&  opt)
{
if (_wrapper) {
  PyObject* obj = PyObject_GetAttrString((PyObject*)_wrapper, "GetAllAlbums");
  PyErr_Clear();
  if (obj && !PythonQtSlotFunction_Check(obj)) {
    static const char* argumentList[] ={"QList<LibraryBackendInterface::Album >" , "const QueryOptions&"};
    static const PythonQtMethodInfo* methodInfo = PythonQtMethodInfo::getCachedMethodInfoFromArgumentList(2, argumentList);
      QList<LibraryBackendInterface::Album > returnValue;
    void* args[2] = {NULL, (void*)&opt};
    PyObject* result = PythonQtSignalTarget::call(obj, methodInfo, args, true);
    if (result) {
      args[0] = PythonQtConv::ConvertPythonToQt(methodInfo->parameters().at(0), result, false, NULL, &returnValue);
      if (args[0]!=&returnValue) {
        if (args[0]==NULL) {
          PythonQt::priv()->handleVirtualOverloadReturnError("GetAllAlbums", methodInfo, result);
        } else {
          returnValue = *((QList<LibraryBackendInterface::Album >*)args[0]);
        }
      }
    }
    if (result) { Py_DECREF(result); } 
    Py_DECREF(obj);
    return returnValue;
  }
}
  return QList<LibraryBackendInterface::Album >();
}
QStringList  PythonQtShell_LibraryBackendInterface::GetAllArtists(const QueryOptions&  opt)
{
if (_wrapper) {
  PyObject* obj = PyObject_GetAttrString((PyObject*)_wrapper, "GetAllArtists");
  PyErr_Clear();
  if (obj && !PythonQtSlotFunction_Check(obj)) {
    static const char* argumentList[] ={"QStringList" , "const QueryOptions&"};
    static const PythonQtMethodInfo* methodInfo = PythonQtMethodInfo::getCachedMethodInfoFromArgumentList(2, argumentList);
      QStringList returnValue;
    void* args[2] = {NULL, (void*)&opt};
    PyObject* result = PythonQtSignalTarget::call(obj, methodInfo, args, true);
    if (result) {
      args[0] = PythonQtConv::ConvertPythonToQt(methodInfo->parameters().at(0), result, false, NULL, &returnValue);
      if (args[0]!=&returnValue) {
        if (args[0]==NULL) {
          PythonQt::priv()->handleVirtualOverloadReturnError("GetAllArtists", methodInfo, result);
        } else {
          returnValue = *((QStringList*)args[0]);
        }
      }
    }
    if (result) { Py_DECREF(result); } 
    Py_DECREF(obj);
    return returnValue;
  }
}
  return QStringList();
}
QStringList  PythonQtShell_LibraryBackendInterface::GetAllArtistsWithAlbums(const QueryOptions&  opt)
{
if (_wrapper) {
  PyObject* obj = PyObject_GetAttrString((PyObject*)_wrapper, "GetAllArtistsWithAlbums");
  PyErr_Clear();
  if (obj && !PythonQtSlotFunction_Check(obj)) {
    static const char* argumentList[] ={"QStringList" , "const QueryOptions&"};
    static const PythonQtMethodInfo* methodInfo = PythonQtMethodInfo::getCachedMethodInfoFromArgumentList(2, argumentList);
      QStringList returnValue;
    void* args[2] = {NULL, (void*)&opt};
    PyObject* result = PythonQtSignalTarget::call(obj, methodInfo, args, true);
    if (result) {
      args[0] = PythonQtConv::ConvertPythonToQt(methodInfo->parameters().at(0), result, false, NULL, &returnValue);
      if (args[0]!=&returnValue) {
        if (args[0]==NULL) {
          PythonQt::priv()->handleVirtualOverloadReturnError("GetAllArtistsWithAlbums", methodInfo, result);
        } else {
          returnValue = *((QStringList*)args[0]);
        }
      }
    }
    if (result) { Py_DECREF(result); } 
    Py_DECREF(obj);
    return returnValue;
  }
}
  return QStringList();
}
QList<Directory >  PythonQtShell_LibraryBackendInterface::GetAllDirectories()
{
if (_wrapper) {
  PyObject* obj = PyObject_GetAttrString((PyObject*)_wrapper, "GetAllDirectories");
  PyErr_Clear();
  if (obj && !PythonQtSlotFunction_Check(obj)) {
    static const char* argumentList[] ={"QList<Directory >"};
    static const PythonQtMethodInfo* methodInfo = PythonQtMethodInfo::getCachedMethodInfoFromArgumentList(1, argumentList);
      QList<Directory > returnValue;
    void* args[1] = {NULL};
    PyObject* result = PythonQtSignalTarget::call(obj, methodInfo, args, true);
    if (result) {
      args[0] = PythonQtConv::ConvertPythonToQt(methodInfo->parameters().at(0), result, false, NULL, &returnValue);
      if (args[0]!=&returnValue) {
        if (args[0]==NULL) {
          PythonQt::priv()->handleVirtualOverloadReturnError("GetAllDirectories", methodInfo, result);
        } else {
          returnValue = *((QList<Directory >*)args[0]);
        }
      }
    }
    if (result) { Py_DECREF(result); } 
    Py_DECREF(obj);
    return returnValue;
  }
}
  return QList<Directory >();
}
QList<LibraryBackendInterface::Album >  PythonQtShell_LibraryBackendInterface::GetCompilationAlbums(const QueryOptions&  opt)
{
if (_wrapper) {
  PyObject* obj = PyObject_GetAttrString((PyObject*)_wrapper, "GetCompilationAlbums");
  PyErr_Clear();
  if (obj && !PythonQtSlotFunction_Check(obj)) {
    static const char* argumentList[] ={"QList<LibraryBackendInterface::Album >" , "const QueryOptions&"};
    static const PythonQtMethodInfo* methodInfo = PythonQtMethodInfo::getCachedMethodInfoFromArgumentList(2, argumentList);
      QList<LibraryBackendInterface::Album > returnValue;
    void* args[2] = {NULL, (void*)&opt};
    PyObject* result = PythonQtSignalTarget::call(obj, methodInfo, args, true);
    if (result) {
      args[0] = PythonQtConv::ConvertPythonToQt(methodInfo->parameters().at(0), result, false, NULL, &returnValue);
      if (args[0]!=&returnValue) {
        if (args[0]==NULL) {
          PythonQt::priv()->handleVirtualOverloadReturnError("GetCompilationAlbums", methodInfo, result);
        } else {
          returnValue = *((QList<LibraryBackendInterface::Album >*)args[0]);
        }
      }
    }
    if (result) { Py_DECREF(result); } 
    Py_DECREF(obj);
    return returnValue;
  }
}
  return QList<LibraryBackendInterface::Album >();
}
QList<Song >  PythonQtShell_LibraryBackendInterface::GetCompilationSongs(const QString&  album, const QueryOptions&  opt)
{
if (_wrapper) {
  PyObject* obj = PyObject_GetAttrString((PyObject*)_wrapper, "GetCompilationSongs");
  PyErr_Clear();
  if (obj && !PythonQtSlotFunction_Check(obj)) {
    static const char* argumentList[] ={"QList<Song >" , "const QString&" , "const QueryOptions&"};
    static const PythonQtMethodInfo* methodInfo = PythonQtMethodInfo::getCachedMethodInfoFromArgumentList(3, argumentList);
      QList<Song > returnValue;
    void* args[3] = {NULL, (void*)&album, (void*)&opt};
    PyObject* result = PythonQtSignalTarget::call(obj, methodInfo, args, true);
    if (result) {
      args[0] = PythonQtConv::ConvertPythonToQt(methodInfo->parameters().at(0), result, false, NULL, &returnValue);
      if (args[0]!=&returnValue) {
        if (args[0]==NULL) {
          PythonQt::priv()->handleVirtualOverloadReturnError("GetCompilationSongs", methodInfo, result);
        } else {
          returnValue = *((QList<Song >*)args[0]);
        }
      }
    }
    if (result) { Py_DECREF(result); } 
    Py_DECREF(obj);
    return returnValue;
  }
}
  return QList<Song >();
}
Song  PythonQtShell_LibraryBackendInterface::GetSongById(int  id)
{
if (_wrapper) {
  PyObject* obj = PyObject_GetAttrString((PyObject*)_wrapper, "GetSongById");
  PyErr_Clear();
  if (obj && !PythonQtSlotFunction_Check(obj)) {
    static const char* argumentList[] ={"Song" , "int"};
    static const PythonQtMethodInfo* methodInfo = PythonQtMethodInfo::getCachedMethodInfoFromArgumentList(2, argumentList);
      Song returnValue;
    void* args[2] = {NULL, (void*)&id};
    PyObject* result = PythonQtSignalTarget::call(obj, methodInfo, args, true);
    if (result) {
      args[0] = PythonQtConv::ConvertPythonToQt(methodInfo->parameters().at(0), result, false, NULL, &returnValue);
      if (args[0]!=&returnValue) {
        if (args[0]==NULL) {
          PythonQt::priv()->handleVirtualOverloadReturnError("GetSongById", methodInfo, result);
        } else {
          returnValue = *((Song*)args[0]);
        }
      }
    }
    if (result) { Py_DECREF(result); } 
    Py_DECREF(obj);
    return returnValue;
  }
}
  return Song();
}
Song  PythonQtShell_LibraryBackendInterface::GetSongByUrl(const QUrl&  url, qint64  beginning)
{
if (_wrapper) {
  PyObject* obj = PyObject_GetAttrString((PyObject*)_wrapper, "GetSongByUrl");
  PyErr_Clear();
  if (obj && !PythonQtSlotFunction_Check(obj)) {
    static const char* argumentList[] ={"Song" , "const QUrl&" , "qint64"};
    static const PythonQtMethodInfo* methodInfo = PythonQtMethodInfo::getCachedMethodInfoFromArgumentList(3, argumentList);
      Song returnValue;
    void* args[3] = {NULL, (void*)&url, (void*)&beginning};
    PyObject* result = PythonQtSignalTarget::call(obj, methodInfo, args, true);
    if (result) {
      args[0] = PythonQtConv::ConvertPythonToQt(methodInfo->parameters().at(0), result, false, NULL, &returnValue);
      if (args[0]!=&returnValue) {
        if (args[0]==NULL) {
          PythonQt::priv()->handleVirtualOverloadReturnError("GetSongByUrl", methodInfo, result);
        } else {
          returnValue = *((Song*)args[0]);
        }
      }
    }
    if (result) { Py_DECREF(result); } 
    Py_DECREF(obj);
    return returnValue;
  }
}
  return Song();
}
QList<Song >  PythonQtShell_LibraryBackendInterface::GetSongs(const QString&  artist, const QString&  album, const QueryOptions&  opt)
{
if (_wrapper) {
  PyObject* obj = PyObject_GetAttrString((PyObject*)_wrapper, "GetSongs");
  PyErr_Clear();
  if (obj && !PythonQtSlotFunction_Check(obj)) {
    static const char* argumentList[] ={"QList<Song >" , "const QString&" , "const QString&" , "const QueryOptions&"};
    static const PythonQtMethodInfo* methodInfo = PythonQtMethodInfo::getCachedMethodInfoFromArgumentList(4, argumentList);
      QList<Song > returnValue;
    void* args[4] = {NULL, (void*)&artist, (void*)&album, (void*)&opt};
    PyObject* result = PythonQtSignalTarget::call(obj, methodInfo, args, true);
    if (result) {
      args[0] = PythonQtConv::ConvertPythonToQt(methodInfo->parameters().at(0), result, false, NULL, &returnValue);
      if (args[0]!=&returnValue) {
        if (args[0]==NULL) {
          PythonQt::priv()->handleVirtualOverloadReturnError("GetSongs", methodInfo, result);
        } else {
          returnValue = *((QList<Song >*)args[0]);
        }
      }
    }
    if (result) { Py_DECREF(result); } 
    Py_DECREF(obj);
    return returnValue;
  }
}
  return QList<Song >();
}
QList<Song >  PythonQtShell_LibraryBackendInterface::GetSongsByUrl(const QUrl&  url)
{
if (_wrapper) {
  PyObject* obj = PyObject_GetAttrString((PyObject*)_wrapper, "GetSongsByUrl");
  PyErr_Clear();
  if (obj && !PythonQtSlotFunction_Check(obj)) {
    static const char* argumentList[] ={"QList<Song >" , "const QUrl&"};
    static const PythonQtMethodInfo* methodInfo = PythonQtMethodInfo::getCachedMethodInfoFromArgumentList(2, argumentList);
      QList<Song > returnValue;
    void* args[2] = {NULL, (void*)&url};
    PyObject* result = PythonQtSignalTarget::call(obj, methodInfo, args, true);
    if (result) {
      args[0] = PythonQtConv::ConvertPythonToQt(methodInfo->parameters().at(0), result, false, NULL, &returnValue);
      if (args[0]!=&returnValue) {
        if (args[0]==NULL) {
          PythonQt::priv()->handleVirtualOverloadReturnError("GetSongsByUrl", methodInfo, result);
        } else {
          returnValue = *((QList<Song >*)args[0]);
        }
      }
    }
    if (result) { Py_DECREF(result); } 
    Py_DECREF(obj);
    return returnValue;
  }
}
  return QList<Song >();
}
bool  PythonQtShell_LibraryBackendInterface::HasCompilations(const QueryOptions&  opt)
{
if (_wrapper) {
  PyObject* obj = PyObject_GetAttrString((PyObject*)_wrapper, "HasCompilations");
  PyErr_Clear();
  if (obj && !PythonQtSlotFunction_Check(obj)) {
    static const char* argumentList[] ={"bool" , "const QueryOptions&"};
    static const PythonQtMethodInfo* methodInfo = PythonQtMethodInfo::getCachedMethodInfoFromArgumentList(2, argumentList);
      bool returnValue;
    void* args[2] = {NULL, (void*)&opt};
    PyObject* result = PythonQtSignalTarget::call(obj, methodInfo, args, true);
    if (result) {
      args[0] = PythonQtConv::ConvertPythonToQt(methodInfo->parameters().at(0), result, false, NULL, &returnValue);
      if (args[0]!=&returnValue) {
        if (args[0]==NULL) {
          PythonQt::priv()->handleVirtualOverloadReturnError("HasCompilations", methodInfo, result);
        } else {
          returnValue = *((bool*)args[0]);
        }
      }
    }
    if (result) { Py_DECREF(result); } 
    Py_DECREF(obj);
    return returnValue;
  }
}
  return bool();
}
void PythonQtShell_LibraryBackendInterface::LoadDirectoriesAsync()
{
if (_wrapper) {
  PyObject* obj = PyObject_GetAttrString((PyObject*)_wrapper, "LoadDirectoriesAsync");
  PyErr_Clear();
  if (obj && !PythonQtSlotFunction_Check(obj)) {
    static const char* argumentList[] ={""};
    static const PythonQtMethodInfo* methodInfo = PythonQtMethodInfo::getCachedMethodInfoFromArgumentList(1, argumentList);
    void* args[1] = {NULL};
    PyObject* result = PythonQtSignalTarget::call(obj, methodInfo, args, true);
    if (result) { Py_DECREF(result); } 
    Py_DECREF(obj);
    return;
  }
}
  
}
void PythonQtShell_LibraryBackendInterface::RemoveDirectory(const Directory&  dir)
{
if (_wrapper) {
  PyObject* obj = PyObject_GetAttrString((PyObject*)_wrapper, "RemoveDirectory");
  PyErr_Clear();
  if (obj && !PythonQtSlotFunction_Check(obj)) {
    static const char* argumentList[] ={"" , "const Directory&"};
    static const PythonQtMethodInfo* methodInfo = PythonQtMethodInfo::getCachedMethodInfoFromArgumentList(2, argumentList);
    void* args[2] = {NULL, (void*)&dir};
    PyObject* result = PythonQtSignalTarget::call(obj, methodInfo, args, true);
    if (result) { Py_DECREF(result); } 
    Py_DECREF(obj);
    return;
  }
}
  
}
QList<Subdirectory >  PythonQtShell_LibraryBackendInterface::SubdirsInDirectory(int  id)
{
if (_wrapper) {
  PyObject* obj = PyObject_GetAttrString((PyObject*)_wrapper, "SubdirsInDirectory");
  PyErr_Clear();
  if (obj && !PythonQtSlotFunction_Check(obj)) {
    static const char* argumentList[] ={"QList<Subdirectory >" , "int"};
    static const PythonQtMethodInfo* methodInfo = PythonQtMethodInfo::getCachedMethodInfoFromArgumentList(2, argumentList);
      QList<Subdirectory > returnValue;
    void* args[2] = {NULL, (void*)&id};
    PyObject* result = PythonQtSignalTarget::call(obj, methodInfo, args, true);
    if (result) {
      args[0] = PythonQtConv::ConvertPythonToQt(methodInfo->parameters().at(0), result, false, NULL, &returnValue);
      if (args[0]!=&returnValue) {
        if (args[0]==NULL) {
          PythonQt::priv()->handleVirtualOverloadReturnError("SubdirsInDirectory", methodInfo, result);
        } else {
          returnValue = *((QList<Subdirectory >*)args[0]);
        }
      }
    }
    if (result) { Py_DECREF(result); } 
    Py_DECREF(obj);
    return returnValue;
  }
}
  return QList<Subdirectory >();
}
void PythonQtShell_LibraryBackendInterface::UpdateManualAlbumArtAsync(const QString&  artist, const QString&  album, const QString&  art)
{
if (_wrapper) {
  PyObject* obj = PyObject_GetAttrString((PyObject*)_wrapper, "UpdateManualAlbumArtAsync");
  PyErr_Clear();
  if (obj && !PythonQtSlotFunction_Check(obj)) {
    static const char* argumentList[] ={"" , "const QString&" , "const QString&" , "const QString&"};
    static const PythonQtMethodInfo* methodInfo = PythonQtMethodInfo::getCachedMethodInfoFromArgumentList(4, argumentList);
    void* args[4] = {NULL, (void*)&artist, (void*)&album, (void*)&art};
    PyObject* result = PythonQtSignalTarget::call(obj, methodInfo, args, true);
    if (result) { Py_DECREF(result); } 
    Py_DECREF(obj);
    return;
  }
}
  
}
void PythonQtShell_LibraryBackendInterface::UpdateTotalSongCountAsync()
{
if (_wrapper) {
  PyObject* obj = PyObject_GetAttrString((PyObject*)_wrapper, "UpdateTotalSongCountAsync");
  PyErr_Clear();
  if (obj && !PythonQtSlotFunction_Check(obj)) {
    static const char* argumentList[] ={""};
    static const PythonQtMethodInfo* methodInfo = PythonQtMethodInfo::getCachedMethodInfoFromArgumentList(1, argumentList);
    void* args[1] = {NULL};
    PyObject* result = PythonQtSignalTarget::call(obj, methodInfo, args, true);
    if (result) { Py_DECREF(result); } 
    Py_DECREF(obj);
    return;
  }
}
  
}
void PythonQtShell_LibraryBackendInterface::childEvent(QChildEvent*  arg__1)
{
if (_wrapper) {
  PyObject* obj = PyObject_GetAttrString((PyObject*)_wrapper, "childEvent");
  PyErr_Clear();
  if (obj && !PythonQtSlotFunction_Check(obj)) {
    static const char* argumentList[] ={"" , "QChildEvent*"};
    static const PythonQtMethodInfo* methodInfo = PythonQtMethodInfo::getCachedMethodInfoFromArgumentList(2, argumentList);
    void* args[2] = {NULL, (void*)&arg__1};
    PyObject* result = PythonQtSignalTarget::call(obj, methodInfo, args, true);
    if (result) { Py_DECREF(result); } 
    Py_DECREF(obj);
    return;
  }
}
  LibraryBackendInterface::childEvent(arg__1);
}
void PythonQtShell_LibraryBackendInterface::customEvent(QEvent*  arg__1)
{
if (_wrapper) {
  PyObject* obj = PyObject_GetAttrString((PyObject*)_wrapper, "customEvent");
  PyErr_Clear();
  if (obj && !PythonQtSlotFunction_Check(obj)) {
    static const char* argumentList[] ={"" , "QEvent*"};
    static const PythonQtMethodInfo* methodInfo = PythonQtMethodInfo::getCachedMethodInfoFromArgumentList(2, argumentList);
    void* args[2] = {NULL, (void*)&arg__1};
    PyObject* result = PythonQtSignalTarget::call(obj, methodInfo, args, true);
    if (result) { Py_DECREF(result); } 
    Py_DECREF(obj);
    return;
  }
}
  LibraryBackendInterface::customEvent(arg__1);
}
bool  PythonQtShell_LibraryBackendInterface::event(QEvent*  arg__1)
{
if (_wrapper) {
  PyObject* obj = PyObject_GetAttrString((PyObject*)_wrapper, "event");
  PyErr_Clear();
  if (obj && !PythonQtSlotFunction_Check(obj)) {
    static const char* argumentList[] ={"bool" , "QEvent*"};
    static const PythonQtMethodInfo* methodInfo = PythonQtMethodInfo::getCachedMethodInfoFromArgumentList(2, argumentList);
      bool returnValue;
    void* args[2] = {NULL, (void*)&arg__1};
    PyObject* result = PythonQtSignalTarget::call(obj, methodInfo, args, true);
    if (result) {
      args[0] = PythonQtConv::ConvertPythonToQt(methodInfo->parameters().at(0), result, false, NULL, &returnValue);
      if (args[0]!=&returnValue) {
        if (args[0]==NULL) {
          PythonQt::priv()->handleVirtualOverloadReturnError("event", methodInfo, result);
        } else {
          returnValue = *((bool*)args[0]);
        }
      }
    }
    if (result) { Py_DECREF(result); } 
    Py_DECREF(obj);
    return returnValue;
  }
}
  return LibraryBackendInterface::event(arg__1);
}
bool  PythonQtShell_LibraryBackendInterface::eventFilter(QObject*  arg__1, QEvent*  arg__2)
{
if (_wrapper) {
  PyObject* obj = PyObject_GetAttrString((PyObject*)_wrapper, "eventFilter");
  PyErr_Clear();
  if (obj && !PythonQtSlotFunction_Check(obj)) {
    static const char* argumentList[] ={"bool" , "QObject*" , "QEvent*"};
    static const PythonQtMethodInfo* methodInfo = PythonQtMethodInfo::getCachedMethodInfoFromArgumentList(3, argumentList);
      bool returnValue;
    void* args[3] = {NULL, (void*)&arg__1, (void*)&arg__2};
    PyObject* result = PythonQtSignalTarget::call(obj, methodInfo, args, true);
    if (result) {
      args[0] = PythonQtConv::ConvertPythonToQt(methodInfo->parameters().at(0), result, false, NULL, &returnValue);
      if (args[0]!=&returnValue) {
        if (args[0]==NULL) {
          PythonQt::priv()->handleVirtualOverloadReturnError("eventFilter", methodInfo, result);
        } else {
          returnValue = *((bool*)args[0]);
        }
      }
    }
    if (result) { Py_DECREF(result); } 
    Py_DECREF(obj);
    return returnValue;
  }
}
  return LibraryBackendInterface::eventFilter(arg__1, arg__2);
}
void PythonQtShell_LibraryBackendInterface::timerEvent(QTimerEvent*  arg__1)
{
if (_wrapper) {
  PyObject* obj = PyObject_GetAttrString((PyObject*)_wrapper, "timerEvent");
  PyErr_Clear();
  if (obj && !PythonQtSlotFunction_Check(obj)) {
    static const char* argumentList[] ={"" , "QTimerEvent*"};
    static const PythonQtMethodInfo* methodInfo = PythonQtMethodInfo::getCachedMethodInfoFromArgumentList(2, argumentList);
    void* args[2] = {NULL, (void*)&arg__1};
    PyObject* result = PythonQtSignalTarget::call(obj, methodInfo, args, true);
    if (result) { Py_DECREF(result); } 
    Py_DECREF(obj);
    return;
  }
}
  LibraryBackendInterface::timerEvent(arg__1);
}
LibraryBackendInterface* PythonQtWrapper_LibraryBackendInterface::new_LibraryBackendInterface(QObject*  parent)
{ 
return new PythonQtShell_LibraryBackendInterface(parent); }



LibraryBackendInterface::Album* PythonQtWrapper_LibraryBackendInterface_Album::new_LibraryBackendInterface_Album()
{ 
return new PythonQtShell_LibraryBackendInterface_Album(); }

LibraryBackendInterface::Album* PythonQtWrapper_LibraryBackendInterface_Album::new_LibraryBackendInterface_Album(const QString&  _artist, const QString&  _album_name, const QString&  _art_automatic, const QString&  _art_manual, const QUrl&  _first_url)
{ 
return new PythonQtShell_LibraryBackendInterface_Album(_artist, _album_name, _art_automatic, _art_manual, _first_url); }



LibraryQuery* PythonQtWrapper_LibraryQuery::new_LibraryQuery(const QueryOptions&  options)
{ 
return new LibraryQuery(options); }

void PythonQtWrapper_LibraryQuery::AddCompilationRequirement(LibraryQuery* theWrappedObject, bool  compilation)
{
  ( theWrappedObject->AddCompilationRequirement(compilation));
}

void PythonQtWrapper_LibraryQuery::AddWhere(LibraryQuery* theWrappedObject, const QString&  column, const QVariant&  value, const QString&  op)
{
  ( theWrappedObject->AddWhere(column, value, op));
}

bool  PythonQtWrapper_LibraryQuery::Next(LibraryQuery* theWrappedObject)
{
  return ( theWrappedObject->Next());
}

void PythonQtWrapper_LibraryQuery::SetColumnSpec(LibraryQuery* theWrappedObject, const QString&  spec)
{
  ( theWrappedObject->SetColumnSpec(spec));
}

void PythonQtWrapper_LibraryQuery::SetIncludeUnavailable(LibraryQuery* theWrappedObject, bool  include_unavailable)
{
  ( theWrappedObject->SetIncludeUnavailable(include_unavailable));
}

void PythonQtWrapper_LibraryQuery::SetLimit(LibraryQuery* theWrappedObject, int  limit)
{
  ( theWrappedObject->SetLimit(limit));
}

void PythonQtWrapper_LibraryQuery::SetOrderBy(LibraryQuery* theWrappedObject, const QString&  order_by)
{
  ( theWrappedObject->SetOrderBy(order_by));
}

QVariant  PythonQtWrapper_LibraryQuery::Value(LibraryQuery* theWrappedObject, int  column) const
{
  return ( theWrappedObject->Value(column));
}



void PythonQtShell_NetworkAccessManager::childEvent(QChildEvent*  arg__1)
{
if (_wrapper) {
  PyObject* obj = PyObject_GetAttrString((PyObject*)_wrapper, "childEvent");
  PyErr_Clear();
  if (obj && !PythonQtSlotFunction_Check(obj)) {
    static const char* argumentList[] ={"" , "QChildEvent*"};
    static const PythonQtMethodInfo* methodInfo = PythonQtMethodInfo::getCachedMethodInfoFromArgumentList(2, argumentList);
    void* args[2] = {NULL, (void*)&arg__1};
    PyObject* result = PythonQtSignalTarget::call(obj, methodInfo, args, true);
    if (result) { Py_DECREF(result); } 
    Py_DECREF(obj);
    return;
  }
}
  NetworkAccessManager::childEvent(arg__1);
}
QNetworkReply*  PythonQtShell_NetworkAccessManager::createRequest(QNetworkAccessManager::Operation  op, const QNetworkRequest&  request, QIODevice*  outgoingData)
{
if (_wrapper) {
  PyObject* obj = PyObject_GetAttrString((PyObject*)_wrapper, "createRequest");
  PyErr_Clear();
  if (obj && !PythonQtSlotFunction_Check(obj)) {
    static const char* argumentList[] ={"QNetworkReply*" , "QNetworkAccessManager::Operation" , "const QNetworkRequest&" , "QIODevice*"};
    static const PythonQtMethodInfo* methodInfo = PythonQtMethodInfo::getCachedMethodInfoFromArgumentList(4, argumentList);
      QNetworkReply* returnValue;
    void* args[4] = {NULL, (void*)&op, (void*)&request, (void*)&outgoingData};
    PyObject* result = PythonQtSignalTarget::call(obj, methodInfo, args, true);
    if (result) {
      args[0] = PythonQtConv::ConvertPythonToQt(methodInfo->parameters().at(0), result, false, NULL, &returnValue);
      if (args[0]!=&returnValue) {
        if (args[0]==NULL) {
          PythonQt::priv()->handleVirtualOverloadReturnError("createRequest", methodInfo, result);
        } else {
          returnValue = *((QNetworkReply**)args[0]);
        }
      }
    }
    if (result) { Py_DECREF(result); } 
    Py_DECREF(obj);
    return returnValue;
  }
}
  return NetworkAccessManager::createRequest(op, request, outgoingData);
}
void PythonQtShell_NetworkAccessManager::customEvent(QEvent*  arg__1)
{
if (_wrapper) {
  PyObject* obj = PyObject_GetAttrString((PyObject*)_wrapper, "customEvent");
  PyErr_Clear();
  if (obj && !PythonQtSlotFunction_Check(obj)) {
    static const char* argumentList[] ={"" , "QEvent*"};
    static const PythonQtMethodInfo* methodInfo = PythonQtMethodInfo::getCachedMethodInfoFromArgumentList(2, argumentList);
    void* args[2] = {NULL, (void*)&arg__1};
    PyObject* result = PythonQtSignalTarget::call(obj, methodInfo, args, true);
    if (result) { Py_DECREF(result); } 
    Py_DECREF(obj);
    return;
  }
}
  NetworkAccessManager::customEvent(arg__1);
}
bool  PythonQtShell_NetworkAccessManager::event(QEvent*  arg__1)
{
if (_wrapper) {
  PyObject* obj = PyObject_GetAttrString((PyObject*)_wrapper, "event");
  PyErr_Clear();
  if (obj && !PythonQtSlotFunction_Check(obj)) {
    static const char* argumentList[] ={"bool" , "QEvent*"};
    static const PythonQtMethodInfo* methodInfo = PythonQtMethodInfo::getCachedMethodInfoFromArgumentList(2, argumentList);
      bool returnValue;
    void* args[2] = {NULL, (void*)&arg__1};
    PyObject* result = PythonQtSignalTarget::call(obj, methodInfo, args, true);
    if (result) {
      args[0] = PythonQtConv::ConvertPythonToQt(methodInfo->parameters().at(0), result, false, NULL, &returnValue);
      if (args[0]!=&returnValue) {
        if (args[0]==NULL) {
          PythonQt::priv()->handleVirtualOverloadReturnError("event", methodInfo, result);
        } else {
          returnValue = *((bool*)args[0]);
        }
      }
    }
    if (result) { Py_DECREF(result); } 
    Py_DECREF(obj);
    return returnValue;
  }
}
  return NetworkAccessManager::event(arg__1);
}
bool  PythonQtShell_NetworkAccessManager::eventFilter(QObject*  arg__1, QEvent*  arg__2)
{
if (_wrapper) {
  PyObject* obj = PyObject_GetAttrString((PyObject*)_wrapper, "eventFilter");
  PyErr_Clear();
  if (obj && !PythonQtSlotFunction_Check(obj)) {
    static const char* argumentList[] ={"bool" , "QObject*" , "QEvent*"};
    static const PythonQtMethodInfo* methodInfo = PythonQtMethodInfo::getCachedMethodInfoFromArgumentList(3, argumentList);
      bool returnValue;
    void* args[3] = {NULL, (void*)&arg__1, (void*)&arg__2};
    PyObject* result = PythonQtSignalTarget::call(obj, methodInfo, args, true);
    if (result) {
      args[0] = PythonQtConv::ConvertPythonToQt(methodInfo->parameters().at(0), result, false, NULL, &returnValue);
      if (args[0]!=&returnValue) {
        if (args[0]==NULL) {
          PythonQt::priv()->handleVirtualOverloadReturnError("eventFilter", methodInfo, result);
        } else {
          returnValue = *((bool*)args[0]);
        }
      }
    }
    if (result) { Py_DECREF(result); } 
    Py_DECREF(obj);
    return returnValue;
  }
}
  return NetworkAccessManager::eventFilter(arg__1, arg__2);
}
void PythonQtShell_NetworkAccessManager::timerEvent(QTimerEvent*  arg__1)
{
if (_wrapper) {
  PyObject* obj = PyObject_GetAttrString((PyObject*)_wrapper, "timerEvent");
  PyErr_Clear();
  if (obj && !PythonQtSlotFunction_Check(obj)) {
    static const char* argumentList[] ={"" , "QTimerEvent*"};
    static const PythonQtMethodInfo* methodInfo = PythonQtMethodInfo::getCachedMethodInfoFromArgumentList(2, argumentList);
    void* args[2] = {NULL, (void*)&arg__1};
    PyObject* result = PythonQtSignalTarget::call(obj, methodInfo, args, true);
    if (result) { Py_DECREF(result); } 
    Py_DECREF(obj);
    return;
  }
}
  NetworkAccessManager::timerEvent(arg__1);
}
NetworkAccessManager* PythonQtWrapper_NetworkAccessManager::new_NetworkAccessManager(QObject*  parent)
{ 
return new PythonQtShell_NetworkAccessManager(parent); }

QNetworkReply*  PythonQtWrapper_NetworkAccessManager::createRequest(NetworkAccessManager* theWrappedObject, QNetworkAccessManager::Operation  op, const QNetworkRequest&  request, QIODevice*  outgoingData)
{
  return ( ((PythonQtPublicPromoter_NetworkAccessManager*)theWrappedObject)->promoted_createRequest(op, request, outgoingData));
}



void PythonQtShell_NetworkTimeouts::childEvent(QChildEvent*  arg__1)
{
if (_wrapper) {
  PyObject* obj = PyObject_GetAttrString((PyObject*)_wrapper, "childEvent");
  PyErr_Clear();
  if (obj && !PythonQtSlotFunction_Check(obj)) {
    static const char* argumentList[] ={"" , "QChildEvent*"};
    static const PythonQtMethodInfo* methodInfo = PythonQtMethodInfo::getCachedMethodInfoFromArgumentList(2, argumentList);
    void* args[2] = {NULL, (void*)&arg__1};
    PyObject* result = PythonQtSignalTarget::call(obj, methodInfo, args, true);
    if (result) { Py_DECREF(result); } 
    Py_DECREF(obj);
    return;
  }
}
  NetworkTimeouts::childEvent(arg__1);
}
void PythonQtShell_NetworkTimeouts::customEvent(QEvent*  arg__1)
{
if (_wrapper) {
  PyObject* obj = PyObject_GetAttrString((PyObject*)_wrapper, "customEvent");
  PyErr_Clear();
  if (obj && !PythonQtSlotFunction_Check(obj)) {
    static const char* argumentList[] ={"" , "QEvent*"};
    static const PythonQtMethodInfo* methodInfo = PythonQtMethodInfo::getCachedMethodInfoFromArgumentList(2, argumentList);
    void* args[2] = {NULL, (void*)&arg__1};
    PyObject* result = PythonQtSignalTarget::call(obj, methodInfo, args, true);
    if (result) { Py_DECREF(result); } 
    Py_DECREF(obj);
    return;
  }
}
  NetworkTimeouts::customEvent(arg__1);
}
bool  PythonQtShell_NetworkTimeouts::event(QEvent*  arg__1)
{
if (_wrapper) {
  PyObject* obj = PyObject_GetAttrString((PyObject*)_wrapper, "event");
  PyErr_Clear();
  if (obj && !PythonQtSlotFunction_Check(obj)) {
    static const char* argumentList[] ={"bool" , "QEvent*"};
    static const PythonQtMethodInfo* methodInfo = PythonQtMethodInfo::getCachedMethodInfoFromArgumentList(2, argumentList);
      bool returnValue;
    void* args[2] = {NULL, (void*)&arg__1};
    PyObject* result = PythonQtSignalTarget::call(obj, methodInfo, args, true);
    if (result) {
      args[0] = PythonQtConv::ConvertPythonToQt(methodInfo->parameters().at(0), result, false, NULL, &returnValue);
      if (args[0]!=&returnValue) {
        if (args[0]==NULL) {
          PythonQt::priv()->handleVirtualOverloadReturnError("event", methodInfo, result);
        } else {
          returnValue = *((bool*)args[0]);
        }
      }
    }
    if (result) { Py_DECREF(result); } 
    Py_DECREF(obj);
    return returnValue;
  }
}
  return NetworkTimeouts::event(arg__1);
}
bool  PythonQtShell_NetworkTimeouts::eventFilter(QObject*  arg__1, QEvent*  arg__2)
{
if (_wrapper) {
  PyObject* obj = PyObject_GetAttrString((PyObject*)_wrapper, "eventFilter");
  PyErr_Clear();
  if (obj && !PythonQtSlotFunction_Check(obj)) {
    static const char* argumentList[] ={"bool" , "QObject*" , "QEvent*"};
    static const PythonQtMethodInfo* methodInfo = PythonQtMethodInfo::getCachedMethodInfoFromArgumentList(3, argumentList);
      bool returnValue;
    void* args[3] = {NULL, (void*)&arg__1, (void*)&arg__2};
    PyObject* result = PythonQtSignalTarget::call(obj, methodInfo, args, true);
    if (result) {
      args[0] = PythonQtConv::ConvertPythonToQt(methodInfo->parameters().at(0), result, false, NULL, &returnValue);
      if (args[0]!=&returnValue) {
        if (args[0]==NULL) {
          PythonQt::priv()->handleVirtualOverloadReturnError("eventFilter", methodInfo, result);
        } else {
          returnValue = *((bool*)args[0]);
        }
      }
    }
    if (result) { Py_DECREF(result); } 
    Py_DECREF(obj);
    return returnValue;
  }
}
  return NetworkTimeouts::eventFilter(arg__1, arg__2);
}
void PythonQtShell_NetworkTimeouts::timerEvent(QTimerEvent*  arg__1)
{
if (_wrapper) {
  PyObject* obj = PyObject_GetAttrString((PyObject*)_wrapper, "timerEvent");
  PyErr_Clear();
  if (obj && !PythonQtSlotFunction_Check(obj)) {
    static const char* argumentList[] ={"" , "QTimerEvent*"};
    static const PythonQtMethodInfo* methodInfo = PythonQtMethodInfo::getCachedMethodInfoFromArgumentList(2, argumentList);
    void* args[2] = {NULL, (void*)&arg__1};
    PyObject* result = PythonQtSignalTarget::call(obj, methodInfo, args, true);
    if (result) { Py_DECREF(result); } 
    Py_DECREF(obj);
    return;
  }
}
  NetworkTimeouts::timerEvent(arg__1);
}
NetworkTimeouts* PythonQtWrapper_NetworkTimeouts::new_NetworkTimeouts(int  timeout_msec, QObject*  parent)
{ 
return new PythonQtShell_NetworkTimeouts(timeout_msec, parent); }

void PythonQtWrapper_NetworkTimeouts::AddReply(NetworkTimeouts* theWrappedObject, QNetworkReply*  reply)
{
  ( theWrappedObject->AddReply(reply));
}

void PythonQtWrapper_NetworkTimeouts::SetTimeout(NetworkTimeouts* theWrappedObject, int  msec)
{
  ( theWrappedObject->SetTimeout(msec));
}



Engine::State  PythonQtWrapper_Player::GetState(Player* theWrappedObject) const
{
  return ( theWrappedObject->GetState());
}

int  PythonQtWrapper_Player::GetVolume(Player* theWrappedObject) const
{
  return ( theWrappedObject->GetVolume());
}

void PythonQtWrapper_Player::Init(Player* theWrappedObject)
{
  ( theWrappedObject->Init());
}

void PythonQtWrapper_Player::RegisterUrlHandler(Player* theWrappedObject, UrlHandler*  handler)
{
  ( theWrappedObject->RegisterUrlHandler(handler));
}

void PythonQtWrapper_Player::UnregisterUrlHandler(Player* theWrappedObject, UrlHandler*  handler)
{
  ( theWrappedObject->UnregisterUrlHandler(handler));
}

Engine::Base*  PythonQtWrapper_Player::engine(Player* theWrappedObject) const
{
  return ( theWrappedObject->engine());
}



void PythonQtShell_PlaylistParser::childEvent(QChildEvent*  arg__1)
{
if (_wrapper) {
  PyObject* obj = PyObject_GetAttrString((PyObject*)_wrapper, "childEvent");
  PyErr_Clear();
  if (obj && !PythonQtSlotFunction_Check(obj)) {
    static const char* argumentList[] ={"" , "QChildEvent*"};
    static const PythonQtMethodInfo* methodInfo = PythonQtMethodInfo::getCachedMethodInfoFromArgumentList(2, argumentList);
    void* args[2] = {NULL, (void*)&arg__1};
    PyObject* result = PythonQtSignalTarget::call(obj, methodInfo, args, true);
    if (result) { Py_DECREF(result); } 
    Py_DECREF(obj);
    return;
  }
}
  PlaylistParser::childEvent(arg__1);
}
void PythonQtShell_PlaylistParser::customEvent(QEvent*  arg__1)
{
if (_wrapper) {
  PyObject* obj = PyObject_GetAttrString((PyObject*)_wrapper, "customEvent");
  PyErr_Clear();
  if (obj && !PythonQtSlotFunction_Check(obj)) {
    static const char* argumentList[] ={"" , "QEvent*"};
    static const PythonQtMethodInfo* methodInfo = PythonQtMethodInfo::getCachedMethodInfoFromArgumentList(2, argumentList);
    void* args[2] = {NULL, (void*)&arg__1};
    PyObject* result = PythonQtSignalTarget::call(obj, methodInfo, args, true);
    if (result) { Py_DECREF(result); } 
    Py_DECREF(obj);
    return;
  }
}
  PlaylistParser::customEvent(arg__1);
}
bool  PythonQtShell_PlaylistParser::event(QEvent*  arg__1)
{
if (_wrapper) {
  PyObject* obj = PyObject_GetAttrString((PyObject*)_wrapper, "event");
  PyErr_Clear();
  if (obj && !PythonQtSlotFunction_Check(obj)) {
    static const char* argumentList[] ={"bool" , "QEvent*"};
    static const PythonQtMethodInfo* methodInfo = PythonQtMethodInfo::getCachedMethodInfoFromArgumentList(2, argumentList);
      bool returnValue;
    void* args[2] = {NULL, (void*)&arg__1};
    PyObject* result = PythonQtSignalTarget::call(obj, methodInfo, args, true);
    if (result) {
      args[0] = PythonQtConv::ConvertPythonToQt(methodInfo->parameters().at(0), result, false, NULL, &returnValue);
      if (args[0]!=&returnValue) {
        if (args[0]==NULL) {
          PythonQt::priv()->handleVirtualOverloadReturnError("event", methodInfo, result);
        } else {
          returnValue = *((bool*)args[0]);
        }
      }
    }
    if (result) { Py_DECREF(result); } 
    Py_DECREF(obj);
    return returnValue;
  }
}
  return PlaylistParser::event(arg__1);
}
bool  PythonQtShell_PlaylistParser::eventFilter(QObject*  arg__1, QEvent*  arg__2)
{
if (_wrapper) {
  PyObject* obj = PyObject_GetAttrString((PyObject*)_wrapper, "eventFilter");
  PyErr_Clear();
  if (obj && !PythonQtSlotFunction_Check(obj)) {
    static const char* argumentList[] ={"bool" , "QObject*" , "QEvent*"};
    static const PythonQtMethodInfo* methodInfo = PythonQtMethodInfo::getCachedMethodInfoFromArgumentList(3, argumentList);
      bool returnValue;
    void* args[3] = {NULL, (void*)&arg__1, (void*)&arg__2};
    PyObject* result = PythonQtSignalTarget::call(obj, methodInfo, args, true);
    if (result) {
      args[0] = PythonQtConv::ConvertPythonToQt(methodInfo->parameters().at(0), result, false, NULL, &returnValue);
      if (args[0]!=&returnValue) {
        if (args[0]==NULL) {
          PythonQt::priv()->handleVirtualOverloadReturnError("eventFilter", methodInfo, result);
        } else {
          returnValue = *((bool*)args[0]);
        }
      }
    }
    if (result) { Py_DECREF(result); } 
    Py_DECREF(obj);
    return returnValue;
  }
}
  return PlaylistParser::eventFilter(arg__1, arg__2);
}
void PythonQtShell_PlaylistParser::timerEvent(QTimerEvent*  arg__1)
{
if (_wrapper) {
  PyObject* obj = PyObject_GetAttrString((PyObject*)_wrapper, "timerEvent");
  PyErr_Clear();
  if (obj && !PythonQtSlotFunction_Check(obj)) {
    static const char* argumentList[] ={"" , "QTimerEvent*"};
    static const PythonQtMethodInfo* methodInfo = PythonQtMethodInfo::getCachedMethodInfoFromArgumentList(2, argumentList);
    void* args[2] = {NULL, (void*)&arg__1};
    PyObject* result = PythonQtSignalTarget::call(obj, methodInfo, args, true);
    if (result) { Py_DECREF(result); } 
    Py_DECREF(obj);
    return;
  }
}
  PlaylistParser::timerEvent(arg__1);
}
PlaylistParser* PythonQtWrapper_PlaylistParser::new_PlaylistParser(LibraryBackendInterface*  library, QObject*  parent)
{ 
return new PythonQtShell_PlaylistParser(library, parent); }

QList<Song >  PythonQtWrapper_PlaylistParser::LoadFromDevice(PlaylistParser* theWrappedObject, QIODevice*  device, const QString&  path_hint, const QDir&  dir_hint) const
{
  return ( theWrappedObject->LoadFromDevice(device, path_hint, dir_hint));
}

QList<Song >  PythonQtWrapper_PlaylistParser::LoadFromFile(PlaylistParser* theWrappedObject, const QString&  filename) const
{
  return ( theWrappedObject->LoadFromFile(filename));
}

void PythonQtWrapper_PlaylistParser::Save(PlaylistParser* theWrappedObject, const QList<Song >&  songs, const QString&  filename) const
{
  ( theWrappedObject->Save(songs, filename));
}

QString  PythonQtWrapper_PlaylistParser::default_extension(PlaylistParser* theWrappedObject) const
{
  return ( theWrappedObject->default_extension());
}

QString  PythonQtWrapper_PlaylistParser::default_filter(PlaylistParser* theWrappedObject) const
{
  return ( theWrappedObject->default_filter());
}

QStringList  PythonQtWrapper_PlaylistParser::file_extensions(PlaylistParser* theWrappedObject) const
{
  return ( theWrappedObject->file_extensions());
}

QString  PythonQtWrapper_PlaylistParser::filters(PlaylistParser* theWrappedObject) const
{
  return ( theWrappedObject->filters());
}



QueryOptions* PythonQtWrapper_QueryOptions::new_QueryOptions()
{ 
return new QueryOptions(); }

bool  PythonQtWrapper_QueryOptions::Matches(QueryOptions* theWrappedObject, const Song&  song) const
{
  return ( theWrappedObject->Matches(song));
}

QString  PythonQtWrapper_QueryOptions::filter(QueryOptions* theWrappedObject) const
{
  return ( theWrappedObject->filter());
}

int  PythonQtWrapper_QueryOptions::max_age(QueryOptions* theWrappedObject) const
{
  return ( theWrappedObject->max_age());
}

QueryOptions::QueryMode  PythonQtWrapper_QueryOptions::query_mode(QueryOptions* theWrappedObject) const
{
  return ( theWrappedObject->query_mode());
}

void PythonQtWrapper_QueryOptions::set_filter(QueryOptions* theWrappedObject, const QString&  filter)
{
  ( theWrappedObject->set_filter(filter));
}

void PythonQtWrapper_QueryOptions::set_max_age(QueryOptions* theWrappedObject, int  max_age)
{
  ( theWrappedObject->set_max_age(max_age));
}

void PythonQtWrapper_QueryOptions::set_query_mode(QueryOptions* theWrappedObject, QueryOptions::QueryMode  query_mode)
{
  ( theWrappedObject->set_query_mode(query_mode));
}



void PythonQtWrapper_RadioModel::AddService(RadioModel* theWrappedObject, RadioService*  service)
{
  ( theWrappedObject->AddService(service));
}

bool  PythonQtWrapper_RadioModel::IsPlayable(RadioModel* theWrappedObject, const QModelIndex&  index) const
{
  return ( theWrappedObject->IsPlayable(index));
}

void PythonQtWrapper_RadioModel::ReloadSettings(RadioModel* theWrappedObject)
{
  ( theWrappedObject->ReloadSettings());
}

void PythonQtWrapper_RadioModel::RemoveService(RadioModel* theWrappedObject, RadioService*  service)
{
  ( theWrappedObject->RemoveService(service));
}

RadioService*  PythonQtWrapper_RadioModel::static_RadioModel_ServiceByName(const QString&  name)
{
  return (RadioModel::ServiceByName(name));
}

RadioService*  PythonQtWrapper_RadioModel::ServiceForIndex(RadioModel* theWrappedObject, const QModelIndex&  index) const
{
  return ( theWrappedObject->ServiceForIndex(index));
}

RadioService*  PythonQtWrapper_RadioModel::ServiceForItem(RadioModel* theWrappedObject, const QStandardItem*  item) const
{
  return ( theWrappedObject->ServiceForItem(item));
}

void PythonQtWrapper_RadioModel::ShowContextMenu(RadioModel* theWrappedObject, const QModelIndex&  merged_model_index, const QPoint&  global_pos)
{
  ( theWrappedObject->ShowContextMenu(merged_model_index, global_pos));
}

Qt::ItemFlags  PythonQtWrapper_RadioModel::flags(RadioModel* theWrappedObject, const QModelIndex&  index) const
{
  return ( ((PythonQtPublicPromoter_RadioModel*)theWrappedObject)->promoted_flags(index));
}

bool  PythonQtWrapper_RadioModel::hasChildren(RadioModel* theWrappedObject, const QModelIndex&  parent) const
{
  return ( ((PythonQtPublicPromoter_RadioModel*)theWrappedObject)->promoted_hasChildren(parent));
}

QMimeData*  PythonQtWrapper_RadioModel::mimeData(RadioModel* theWrappedObject, const QList<QModelIndex >&  indexes) const
{
  return ( ((PythonQtPublicPromoter_RadioModel*)theWrappedObject)->promoted_mimeData(indexes));
}

QStringList  PythonQtWrapper_RadioModel::mimeTypes(RadioModel* theWrappedObject) const
{
  return ( ((PythonQtPublicPromoter_RadioModel*)theWrappedObject)->promoted_mimeTypes());
}

int  PythonQtWrapper_RadioModel::rowCount(RadioModel* theWrappedObject, const QModelIndex&  parent) const
{
  return ( ((PythonQtPublicPromoter_RadioModel*)theWrappedObject)->promoted_rowCount(parent));
}

TaskManager*  PythonQtWrapper_RadioModel::task_manager(RadioModel* theWrappedObject) const
{
  return ( theWrappedObject->task_manager());
}



QStandardItem*  PythonQtShell_RadioService::CreateRootItem()
{
if (_wrapper) {
  PyObject* obj = PyObject_GetAttrString((PyObject*)_wrapper, "CreateRootItem");
  PyErr_Clear();
  if (obj && !PythonQtSlotFunction_Check(obj)) {
    static const char* argumentList[] ={"QStandardItem*"};
    static const PythonQtMethodInfo* methodInfo = PythonQtMethodInfo::getCachedMethodInfoFromArgumentList(1, argumentList);
      QStandardItem* returnValue;
    void* args[1] = {NULL};
    PyObject* result = PythonQtSignalTarget::call(obj, methodInfo, args, true);
    if (result) {
      args[0] = PythonQtConv::ConvertPythonToQt(methodInfo->parameters().at(0), result, false, NULL, &returnValue);
      if (args[0]!=&returnValue) {
        if (args[0]==NULL) {
          PythonQt::priv()->handleVirtualOverloadReturnError("CreateRootItem", methodInfo, result);
        } else {
          returnValue = *((QStandardItem**)args[0]);
        }
      }
    }
    if (result) { Py_DECREF(result); } 
    Py_DECREF(obj);
    return returnValue;
  }
}
  return 0;
}
QModelIndex  PythonQtShell_RadioService::GetCurrentIndex()
{
if (_wrapper) {
  PyObject* obj = PyObject_GetAttrString((PyObject*)_wrapper, "GetCurrentIndex");
  PyErr_Clear();
  if (obj && !PythonQtSlotFunction_Check(obj)) {
    static const char* argumentList[] ={"QModelIndex"};
    static const PythonQtMethodInfo* methodInfo = PythonQtMethodInfo::getCachedMethodInfoFromArgumentList(1, argumentList);
      QModelIndex returnValue;
    void* args[1] = {NULL};
    PyObject* result = PythonQtSignalTarget::call(obj, methodInfo, args, true);
    if (result) {
      args[0] = PythonQtConv::ConvertPythonToQt(methodInfo->parameters().at(0), result, false, NULL, &returnValue);
      if (args[0]!=&returnValue) {
        if (args[0]==NULL) {
          PythonQt::priv()->handleVirtualOverloadReturnError("GetCurrentIndex", methodInfo, result);
        } else {
          returnValue = *((QModelIndex*)args[0]);
        }
      }
    }
    if (result) { Py_DECREF(result); } 
    Py_DECREF(obj);
    return returnValue;
  }
}
  return QModelIndex();
}
QWidget*  PythonQtShell_RadioService::HeaderWidget() const
{
if (_wrapper) {
  PyObject* obj = PyObject_GetAttrString((PyObject*)_wrapper, "HeaderWidget");
  PyErr_Clear();
  if (obj && !PythonQtSlotFunction_Check(obj)) {
    static const char* argumentList[] ={"QWidget*"};
    static const PythonQtMethodInfo* methodInfo = PythonQtMethodInfo::getCachedMethodInfoFromArgumentList(1, argumentList);
      QWidget* returnValue;
    void* args[1] = {NULL};
    PyObject* result = PythonQtSignalTarget::call(obj, methodInfo, args, true);
    if (result) {
      args[0] = PythonQtConv::ConvertPythonToQt(methodInfo->parameters().at(0), result, false, NULL, &returnValue);
      if (args[0]!=&returnValue) {
        if (args[0]==NULL) {
          PythonQt::priv()->handleVirtualOverloadReturnError("HeaderWidget", methodInfo, result);
        } else {
          returnValue = *((QWidget**)args[0]);
        }
      }
    }
    if (result) { Py_DECREF(result); } 
    Py_DECREF(obj);
    return returnValue;
  }
}
  return RadioService::HeaderWidget();
}
QString  PythonQtShell_RadioService::Icon()
{
if (_wrapper) {
  PyObject* obj = PyObject_GetAttrString((PyObject*)_wrapper, "Icon");
  PyErr_Clear();
  if (obj && !PythonQtSlotFunction_Check(obj)) {
    static const char* argumentList[] ={"QString"};
    static const PythonQtMethodInfo* methodInfo = PythonQtMethodInfo::getCachedMethodInfoFromArgumentList(1, argumentList);
      QString returnValue;
    void* args[1] = {NULL};
    PyObject* result = PythonQtSignalTarget::call(obj, methodInfo, args, true);
    if (result) {
      args[0] = PythonQtConv::ConvertPythonToQt(methodInfo->parameters().at(0), result, false, NULL, &returnValue);
      if (args[0]!=&returnValue) {
        if (args[0]==NULL) {
          PythonQt::priv()->handleVirtualOverloadReturnError("Icon", methodInfo, result);
        } else {
          returnValue = *((QString*)args[0]);
        }
      }
    }
    if (result) { Py_DECREF(result); } 
    Py_DECREF(obj);
    return returnValue;
  }
}
  return RadioService::Icon();
}
void PythonQtShell_RadioService::ItemDoubleClicked(QStandardItem*  item)
{
if (_wrapper) {
  PyObject* obj = PyObject_GetAttrString((PyObject*)_wrapper, "ItemDoubleClicked");
  PyErr_Clear();
  if (obj && !PythonQtSlotFunction_Check(obj)) {
    static const char* argumentList[] ={"" , "QStandardItem*"};
    static const PythonQtMethodInfo* methodInfo = PythonQtMethodInfo::getCachedMethodInfoFromArgumentList(2, argumentList);
    void* args[2] = {NULL, (void*)&item};
    PyObject* result = PythonQtSignalTarget::call(obj, methodInfo, args, true);
    if (result) { Py_DECREF(result); } 
    Py_DECREF(obj);
    return;
  }
}
  RadioService::ItemDoubleClicked(item);
}
void PythonQtShell_RadioService::LazyPopulate(QStandardItem*  parent)
{
if (_wrapper) {
  PyObject* obj = PyObject_GetAttrString((PyObject*)_wrapper, "LazyPopulate");
  PyErr_Clear();
  if (obj && !PythonQtSlotFunction_Check(obj)) {
    static const char* argumentList[] ={"" , "QStandardItem*"};
    static const PythonQtMethodInfo* methodInfo = PythonQtMethodInfo::getCachedMethodInfoFromArgumentList(2, argumentList);
    void* args[2] = {NULL, (void*)&parent};
    PyObject* result = PythonQtSignalTarget::call(obj, methodInfo, args, true);
    if (result) { Py_DECREF(result); } 
    Py_DECREF(obj);
    return;
  }
}
  
}
void PythonQtShell_RadioService::ReloadSettings()
{
if (_wrapper) {
  PyObject* obj = PyObject_GetAttrString((PyObject*)_wrapper, "ReloadSettings");
  PyErr_Clear();
  if (obj && !PythonQtSlotFunction_Check(obj)) {
    static const char* argumentList[] ={""};
    static const PythonQtMethodInfo* methodInfo = PythonQtMethodInfo::getCachedMethodInfoFromArgumentList(1, argumentList);
    void* args[1] = {NULL};
    PyObject* result = PythonQtSignalTarget::call(obj, methodInfo, args, true);
    if (result) { Py_DECREF(result); } 
    Py_DECREF(obj);
    return;
  }
}
  RadioService::ReloadSettings();
}
void PythonQtShell_RadioService::ShowContextMenu(const QModelIndex&  index, const QPoint&  global_pos)
{
if (_wrapper) {
  PyObject* obj = PyObject_GetAttrString((PyObject*)_wrapper, "ShowContextMenu");
  PyErr_Clear();
  if (obj && !PythonQtSlotFunction_Check(obj)) {
    static const char* argumentList[] ={"" , "const QModelIndex&" , "const QPoint&"};
    static const PythonQtMethodInfo* methodInfo = PythonQtMethodInfo::getCachedMethodInfoFromArgumentList(3, argumentList);
    void* args[3] = {NULL, (void*)&index, (void*)&global_pos};
    PyObject* result = PythonQtSignalTarget::call(obj, methodInfo, args, true);
    if (result) { Py_DECREF(result); } 
    Py_DECREF(obj);
    return;
  }
}
  RadioService::ShowContextMenu(index, global_pos);
}
void PythonQtShell_RadioService::childEvent(QChildEvent*  arg__1)
{
if (_wrapper) {
  PyObject* obj = PyObject_GetAttrString((PyObject*)_wrapper, "childEvent");
  PyErr_Clear();
  if (obj && !PythonQtSlotFunction_Check(obj)) {
    static const char* argumentList[] ={"" , "QChildEvent*"};
    static const PythonQtMethodInfo* methodInfo = PythonQtMethodInfo::getCachedMethodInfoFromArgumentList(2, argumentList);
    void* args[2] = {NULL, (void*)&arg__1};
    PyObject* result = PythonQtSignalTarget::call(obj, methodInfo, args, true);
    if (result) { Py_DECREF(result); } 
    Py_DECREF(obj);
    return;
  }
}
  RadioService::childEvent(arg__1);
}
void PythonQtShell_RadioService::customEvent(QEvent*  arg__1)
{
if (_wrapper) {
  PyObject* obj = PyObject_GetAttrString((PyObject*)_wrapper, "customEvent");
  PyErr_Clear();
  if (obj && !PythonQtSlotFunction_Check(obj)) {
    static const char* argumentList[] ={"" , "QEvent*"};
    static const PythonQtMethodInfo* methodInfo = PythonQtMethodInfo::getCachedMethodInfoFromArgumentList(2, argumentList);
    void* args[2] = {NULL, (void*)&arg__1};
    PyObject* result = PythonQtSignalTarget::call(obj, methodInfo, args, true);
    if (result) { Py_DECREF(result); } 
    Py_DECREF(obj);
    return;
  }
}
  RadioService::customEvent(arg__1);
}
bool  PythonQtShell_RadioService::event(QEvent*  arg__1)
{
if (_wrapper) {
  PyObject* obj = PyObject_GetAttrString((PyObject*)_wrapper, "event");
  PyErr_Clear();
  if (obj && !PythonQtSlotFunction_Check(obj)) {
    static const char* argumentList[] ={"bool" , "QEvent*"};
    static const PythonQtMethodInfo* methodInfo = PythonQtMethodInfo::getCachedMethodInfoFromArgumentList(2, argumentList);
      bool returnValue;
    void* args[2] = {NULL, (void*)&arg__1};
    PyObject* result = PythonQtSignalTarget::call(obj, methodInfo, args, true);
    if (result) {
      args[0] = PythonQtConv::ConvertPythonToQt(methodInfo->parameters().at(0), result, false, NULL, &returnValue);
      if (args[0]!=&returnValue) {
        if (args[0]==NULL) {
          PythonQt::priv()->handleVirtualOverloadReturnError("event", methodInfo, result);
        } else {
          returnValue = *((bool*)args[0]);
        }
      }
    }
    if (result) { Py_DECREF(result); } 
    Py_DECREF(obj);
    return returnValue;
  }
}
  return RadioService::event(arg__1);
}
bool  PythonQtShell_RadioService::eventFilter(QObject*  arg__1, QEvent*  arg__2)
{
if (_wrapper) {
  PyObject* obj = PyObject_GetAttrString((PyObject*)_wrapper, "eventFilter");
  PyErr_Clear();
  if (obj && !PythonQtSlotFunction_Check(obj)) {
    static const char* argumentList[] ={"bool" , "QObject*" , "QEvent*"};
    static const PythonQtMethodInfo* methodInfo = PythonQtMethodInfo::getCachedMethodInfoFromArgumentList(3, argumentList);
      bool returnValue;
    void* args[3] = {NULL, (void*)&arg__1, (void*)&arg__2};
    PyObject* result = PythonQtSignalTarget::call(obj, methodInfo, args, true);
    if (result) {
      args[0] = PythonQtConv::ConvertPythonToQt(methodInfo->parameters().at(0), result, false, NULL, &returnValue);
      if (args[0]!=&returnValue) {
        if (args[0]==NULL) {
          PythonQt::priv()->handleVirtualOverloadReturnError("eventFilter", methodInfo, result);
        } else {
          returnValue = *((bool*)args[0]);
        }
      }
    }
    if (result) { Py_DECREF(result); } 
    Py_DECREF(obj);
    return returnValue;
  }
}
  return RadioService::eventFilter(arg__1, arg__2);
}
void PythonQtShell_RadioService::timerEvent(QTimerEvent*  arg__1)
{
if (_wrapper) {
  PyObject* obj = PyObject_GetAttrString((PyObject*)_wrapper, "timerEvent");
  PyErr_Clear();
  if (obj && !PythonQtSlotFunction_Check(obj)) {
    static const char* argumentList[] ={"" , "QTimerEvent*"};
    static const PythonQtMethodInfo* methodInfo = PythonQtMethodInfo::getCachedMethodInfoFromArgumentList(2, argumentList);
    void* args[2] = {NULL, (void*)&arg__1};
    PyObject* result = PythonQtSignalTarget::call(obj, methodInfo, args, true);
    if (result) { Py_DECREF(result); } 
    Py_DECREF(obj);
    return;
  }
}
  RadioService::timerEvent(arg__1);
}
RadioService* PythonQtWrapper_RadioService::new_RadioService(const QString&  name, RadioModel*  model)
{ 
return new PythonQtShell_RadioService(name, model); }

QWidget*  PythonQtWrapper_RadioService::HeaderWidget(RadioService* theWrappedObject) const
{
  return ( ((PythonQtPublicPromoter_RadioService*)theWrappedObject)->promoted_HeaderWidget());
}

QString  PythonQtWrapper_RadioService::Icon(RadioService* theWrappedObject)
{
  return ( ((PythonQtPublicPromoter_RadioService*)theWrappedObject)->promoted_Icon());
}

void PythonQtWrapper_RadioService::ItemDoubleClicked(RadioService* theWrappedObject, QStandardItem*  item)
{
  ( ((PythonQtPublicPromoter_RadioService*)theWrappedObject)->promoted_ItemDoubleClicked(item));
}

void PythonQtWrapper_RadioService::ReloadSettings(RadioService* theWrappedObject)
{
  ( ((PythonQtPublicPromoter_RadioService*)theWrappedObject)->promoted_ReloadSettings());
}

void PythonQtWrapper_RadioService::ShowContextMenu(RadioService* theWrappedObject, const QModelIndex&  index, const QPoint&  global_pos)
{
  ( ((PythonQtPublicPromoter_RadioService*)theWrappedObject)->promoted_ShowContextMenu(index, global_pos));
}

RadioModel*  PythonQtWrapper_RadioService::model(RadioService* theWrappedObject) const
{
  return ( theWrappedObject->model());
}

QString  PythonQtWrapper_RadioService::name(RadioService* theWrappedObject) const
{
  return ( theWrappedObject->name());
}



Song* PythonQtWrapper_Song::new_Song()
{ 
return new PythonQtShell_Song(); }

Song* PythonQtWrapper_Song::new_Song(const Song&  other)
{ 
return new PythonQtShell_Song(other); }

QString  PythonQtWrapper_Song::static_Song_Decode(const QString&  tag, const QTextCodec*  codec)
{
  return (Song::Decode(tag, codec));
}

bool  PythonQtWrapper_Song::HasProperMediaFile(Song* theWrappedObject) const
{
  return ( theWrappedObject->HasProperMediaFile());
}

void PythonQtWrapper_Song::Init(Song* theWrappedObject, const QString&  title, const QString&  artist, const QString&  album, qint64  beginning, qint64  end)
{
  ( theWrappedObject->Init(title, artist, album, beginning, end));
}

void PythonQtWrapper_Song::Init(Song* theWrappedObject, const QString&  title, const QString&  artist, const QString&  album, qint64  length_nanosec)
{
  ( theWrappedObject->Init(title, artist, album, length_nanosec));
}

void PythonQtWrapper_Song::InitFromFile(Song* theWrappedObject, const QString&  filename, int  directory_id)
{
  ( theWrappedObject->InitFromFile(filename, directory_id));
}

void PythonQtWrapper_Song::InitFromFilePartial(Song* theWrappedObject, const QString&  filename)
{
  ( theWrappedObject->InitFromFilePartial(filename));
}

bool  PythonQtWrapper_Song::IsEditable(Song* theWrappedObject) const
{
  return ( theWrappedObject->IsEditable());
}

bool  PythonQtWrapper_Song::IsMetadataEqual(Song* theWrappedObject, const Song&  other) const
{
  return ( theWrappedObject->IsMetadataEqual(other));
}

bool  PythonQtWrapper_Song::IsOnSameAlbum(Song* theWrappedObject, const Song&  other) const
{
  return ( theWrappedObject->IsOnSameAlbum(other));
}

QString  PythonQtWrapper_Song::static_Song_JoinSpec(const QString&  table)
{
  return (Song::JoinSpec(table));
}

QImage  PythonQtWrapper_Song::static_Song_LoadEmbeddedArt(const QString&  filename)
{
  return (Song::LoadEmbeddedArt(filename));
}

QString  PythonQtWrapper_Song::PrettyLength(Song* theWrappedObject) const
{
  return ( theWrappedObject->PrettyLength());
}

QString  PythonQtWrapper_Song::PrettyTitle(Song* theWrappedObject) const
{
  return ( theWrappedObject->PrettyTitle());
}

QString  PythonQtWrapper_Song::PrettyTitleWithArtist(Song* theWrappedObject) const
{
  return ( theWrappedObject->PrettyTitleWithArtist());
}

QString  PythonQtWrapper_Song::PrettyYear(Song* theWrappedObject) const
{
  return ( theWrappedObject->PrettyYear());
}

bool  PythonQtWrapper_Song::Save(Song* theWrappedObject) const
{
  return ( theWrappedObject->Save());
}

QString  PythonQtWrapper_Song::TextForFiletype(Song* theWrappedObject) const
{
  return ( theWrappedObject->TextForFiletype());
}

QString  PythonQtWrapper_Song::static_Song_TextForFiletype(Song::FileType  type)
{
  return (Song::TextForFiletype(type));
}

QString  PythonQtWrapper_Song::TitleWithCompilationArtist(Song* theWrappedObject) const
{
  return ( theWrappedObject->TitleWithCompilationArtist());
}

const QString*  PythonQtWrapper_Song::album(Song* theWrappedObject) const
{
  return &( theWrappedObject->album());
}

const QString*  PythonQtWrapper_Song::albumartist(Song* theWrappedObject) const
{
  return &( theWrappedObject->albumartist());
}

const QString*  PythonQtWrapper_Song::art_automatic(Song* theWrappedObject) const
{
  return &( theWrappedObject->art_automatic());
}

const QString*  PythonQtWrapper_Song::art_manual(Song* theWrappedObject) const
{
  return &( theWrappedObject->art_manual());
}

const QString*  PythonQtWrapper_Song::artist(Song* theWrappedObject) const
{
  return &( theWrappedObject->artist());
}

const QString*  PythonQtWrapper_Song::basefilename(Song* theWrappedObject) const
{
  return &( theWrappedObject->basefilename());
}

qint64  PythonQtWrapper_Song::beginning_nanosec(Song* theWrappedObject) const
{
  return ( theWrappedObject->beginning_nanosec());
}

int  PythonQtWrapper_Song::bitrate(Song* theWrappedObject) const
{
  return ( theWrappedObject->bitrate());
}

float  PythonQtWrapper_Song::bpm(Song* theWrappedObject) const
{
  return ( theWrappedObject->bpm());
}

const QString*  PythonQtWrapper_Song::comment(Song* theWrappedObject) const
{
  return &( theWrappedObject->comment());
}

const QString*  PythonQtWrapper_Song::composer(Song* theWrappedObject) const
{
  return &( theWrappedObject->composer());
}

uint  PythonQtWrapper_Song::ctime(Song* theWrappedObject) const
{
  return ( theWrappedObject->ctime());
}

const QString*  PythonQtWrapper_Song::cue_path(Song* theWrappedObject) const
{
  return &( theWrappedObject->cue_path());
}

int  PythonQtWrapper_Song::directory_id(Song* theWrappedObject) const
{
  return ( theWrappedObject->directory_id());
}

int  PythonQtWrapper_Song::disc(Song* theWrappedObject) const
{
  return ( theWrappedObject->disc());
}

qint64  PythonQtWrapper_Song::end_nanosec(Song* theWrappedObject) const
{
  return ( theWrappedObject->end_nanosec());
}

int  PythonQtWrapper_Song::filesize(Song* theWrappedObject) const
{
  return ( theWrappedObject->filesize());
}

Song::FileType  PythonQtWrapper_Song::filetype(Song* theWrappedObject) const
{
  return ( theWrappedObject->filetype());
}

const QString*  PythonQtWrapper_Song::genre(Song* theWrappedObject) const
{
  return &( theWrappedObject->genre());
}

bool  PythonQtWrapper_Song::has_cue(Song* theWrappedObject) const
{
  return ( theWrappedObject->has_cue());
}

bool  PythonQtWrapper_Song::has_embedded_cover(Song* theWrappedObject) const
{
  return ( theWrappedObject->has_embedded_cover());
}

bool  PythonQtWrapper_Song::has_manually_unset_cover(Song* theWrappedObject) const
{
  return ( theWrappedObject->has_manually_unset_cover());
}

int  PythonQtWrapper_Song::id(Song* theWrappedObject) const
{
  return ( theWrappedObject->id());
}

const QImage*  PythonQtWrapper_Song::image(Song* theWrappedObject) const
{
  return &( theWrappedObject->image());
}

bool  PythonQtWrapper_Song::is_compilation(Song* theWrappedObject) const
{
  return ( theWrappedObject->is_compilation());
}

bool  PythonQtWrapper_Song::is_stream(Song* theWrappedObject) const
{
  return ( theWrappedObject->is_stream());
}

bool  PythonQtWrapper_Song::is_unavailable(Song* theWrappedObject) const
{
  return ( theWrappedObject->is_unavailable());
}

bool  PythonQtWrapper_Song::is_valid(Song* theWrappedObject) const
{
  return ( theWrappedObject->is_valid());
}

int  PythonQtWrapper_Song::lastplayed(Song* theWrappedObject) const
{
  return ( theWrappedObject->lastplayed());
}

qint64  PythonQtWrapper_Song::length_nanosec(Song* theWrappedObject) const
{
  return ( theWrappedObject->length_nanosec());
}

void PythonQtWrapper_Song::manually_unset_cover(Song* theWrappedObject)
{
  ( theWrappedObject->manually_unset_cover());
}

uint  PythonQtWrapper_Song::mtime(Song* theWrappedObject) const
{
  return ( theWrappedObject->mtime());
}

bool  PythonQtWrapper_Song::__eq__(Song* theWrappedObject, const Song&  other) const
{
  return ( (*theWrappedObject)== other);
}

int  PythonQtWrapper_Song::playcount(Song* theWrappedObject) const
{
  return ( theWrappedObject->playcount());
}

float  PythonQtWrapper_Song::rating(Song* theWrappedObject) const
{
  return ( theWrappedObject->rating());
}

int  PythonQtWrapper_Song::samplerate(Song* theWrappedObject) const
{
  return ( theWrappedObject->samplerate());
}

int  PythonQtWrapper_Song::score(Song* theWrappedObject) const
{
  return ( theWrappedObject->score());
}

void PythonQtWrapper_Song::set_album(Song* theWrappedObject, const QString&  v)
{
  ( theWrappedObject->set_album(v));
}

void PythonQtWrapper_Song::set_albumartist(Song* theWrappedObject, const QString&  v)
{
  ( theWrappedObject->set_albumartist(v));
}

void PythonQtWrapper_Song::set_art_automatic(Song* theWrappedObject, const QString&  v)
{
  ( theWrappedObject->set_art_automatic(v));
}

void PythonQtWrapper_Song::set_art_manual(Song* theWrappedObject, const QString&  v)
{
  ( theWrappedObject->set_art_manual(v));
}

void PythonQtWrapper_Song::set_artist(Song* theWrappedObject, const QString&  v)
{
  ( theWrappedObject->set_artist(v));
}

void PythonQtWrapper_Song::set_basefilename(Song* theWrappedObject, const QString&  v)
{
  ( theWrappedObject->set_basefilename(v));
}

void PythonQtWrapper_Song::set_beginning_nanosec(Song* theWrappedObject, qint64  v)
{
  ( theWrappedObject->set_beginning_nanosec(v));
}

void PythonQtWrapper_Song::set_bitrate(Song* theWrappedObject, int  v)
{
  ( theWrappedObject->set_bitrate(v));
}

void PythonQtWrapper_Song::set_bpm(Song* theWrappedObject, float  v)
{
  ( theWrappedObject->set_bpm(v));
}

void PythonQtWrapper_Song::set_comment(Song* theWrappedObject, const QString&  v)
{
  ( theWrappedObject->set_comment(v));
}

void PythonQtWrapper_Song::set_compilation(Song* theWrappedObject, bool  v)
{
  ( theWrappedObject->set_compilation(v));
}

void PythonQtWrapper_Song::set_composer(Song* theWrappedObject, const QString&  v)
{
  ( theWrappedObject->set_composer(v));
}

void PythonQtWrapper_Song::set_ctime(Song* theWrappedObject, int  v)
{
  ( theWrappedObject->set_ctime(v));
}

void PythonQtWrapper_Song::set_cue_path(Song* theWrappedObject, const QString&  v)
{
  ( theWrappedObject->set_cue_path(v));
}

void PythonQtWrapper_Song::set_directory_id(Song* theWrappedObject, int  v)
{
  ( theWrappedObject->set_directory_id(v));
}

void PythonQtWrapper_Song::set_disc(Song* theWrappedObject, int  v)
{
  ( theWrappedObject->set_disc(v));
}

void PythonQtWrapper_Song::set_embedded_cover(Song* theWrappedObject)
{
  ( theWrappedObject->set_embedded_cover());
}

void PythonQtWrapper_Song::set_end_nanosec(Song* theWrappedObject, qint64  v)
{
  ( theWrappedObject->set_end_nanosec(v));
}

void PythonQtWrapper_Song::set_filesize(Song* theWrappedObject, int  v)
{
  ( theWrappedObject->set_filesize(v));
}

void PythonQtWrapper_Song::set_filetype(Song* theWrappedObject, Song::FileType  v)
{
  ( theWrappedObject->set_filetype(v));
}

void PythonQtWrapper_Song::set_forced_compilation_off(Song* theWrappedObject, bool  v)
{
  ( theWrappedObject->set_forced_compilation_off(v));
}

void PythonQtWrapper_Song::set_forced_compilation_on(Song* theWrappedObject, bool  v)
{
  ( theWrappedObject->set_forced_compilation_on(v));
}

void PythonQtWrapper_Song::set_genre(Song* theWrappedObject, const QString&  v)
{
  ( theWrappedObject->set_genre(v));
}

void PythonQtWrapper_Song::set_genre_id3(Song* theWrappedObject, int  id)
{
  ( theWrappedObject->set_genre_id3(id));
}

void PythonQtWrapper_Song::set_id(Song* theWrappedObject, int  id)
{
  ( theWrappedObject->set_id(id));
}

void PythonQtWrapper_Song::set_image(Song* theWrappedObject, const QImage&  i)
{
  ( theWrappedObject->set_image(i));
}

void PythonQtWrapper_Song::set_lastplayed(Song* theWrappedObject, int  v)
{
  ( theWrappedObject->set_lastplayed(v));
}

void PythonQtWrapper_Song::set_length_nanosec(Song* theWrappedObject, qint64  v)
{
  ( theWrappedObject->set_length_nanosec(v));
}

void PythonQtWrapper_Song::set_mtime(Song* theWrappedObject, int  v)
{
  ( theWrappedObject->set_mtime(v));
}

void PythonQtWrapper_Song::set_playcount(Song* theWrappedObject, int  v)
{
  ( theWrappedObject->set_playcount(v));
}

void PythonQtWrapper_Song::set_rating(Song* theWrappedObject, float  v)
{
  ( theWrappedObject->set_rating(v));
}

void PythonQtWrapper_Song::set_sampler(Song* theWrappedObject, bool  v)
{
  ( theWrappedObject->set_sampler(v));
}

void PythonQtWrapper_Song::set_samplerate(Song* theWrappedObject, int  v)
{
  ( theWrappedObject->set_samplerate(v));
}

void PythonQtWrapper_Song::set_score(Song* theWrappedObject, int  v)
{
  ( theWrappedObject->set_score(v));
}

void PythonQtWrapper_Song::set_skipcount(Song* theWrappedObject, int  v)
{
  ( theWrappedObject->set_skipcount(v));
}

void PythonQtWrapper_Song::set_title(Song* theWrappedObject, const QString&  v)
{
  ( theWrappedObject->set_title(v));
}

void PythonQtWrapper_Song::set_track(Song* theWrappedObject, int  v)
{
  ( theWrappedObject->set_track(v));
}

void PythonQtWrapper_Song::set_unavailable(Song* theWrappedObject, bool  v)
{
  ( theWrappedObject->set_unavailable(v));
}

void PythonQtWrapper_Song::set_url(Song* theWrappedObject, const QUrl&  v)
{
  ( theWrappedObject->set_url(v));
}

void PythonQtWrapper_Song::set_valid(Song* theWrappedObject, bool  v)
{
  ( theWrappedObject->set_valid(v));
}

void PythonQtWrapper_Song::set_year(Song* theWrappedObject, int  v)
{
  ( theWrappedObject->set_year(v));
}

int  PythonQtWrapper_Song::skipcount(Song* theWrappedObject) const
{
  return ( theWrappedObject->skipcount());
}

const QString*  PythonQtWrapper_Song::title(Song* theWrappedObject) const
{
  return &( theWrappedObject->title());
}

int  PythonQtWrapper_Song::track(Song* theWrappedObject) const
{
  return ( theWrappedObject->track());
}

const QUrl*  PythonQtWrapper_Song::url(Song* theWrappedObject) const
{
  return &( theWrappedObject->url());
}

int  PythonQtWrapper_Song::year(Song* theWrappedObject) const
{
  return ( theWrappedObject->year());
}



Subdirectory* PythonQtWrapper_Subdirectory::new_Subdirectory()
{ 
return new PythonQtShell_Subdirectory(); }



void PythonQtShell_TaskManager::childEvent(QChildEvent*  arg__1)
{
if (_wrapper) {
  PyObject* obj = PyObject_GetAttrString((PyObject*)_wrapper, "childEvent");
  PyErr_Clear();
  if (obj && !PythonQtSlotFunction_Check(obj)) {
    static const char* argumentList[] ={"" , "QChildEvent*"};
    static const PythonQtMethodInfo* methodInfo = PythonQtMethodInfo::getCachedMethodInfoFromArgumentList(2, argumentList);
    void* args[2] = {NULL, (void*)&arg__1};
    PyObject* result = PythonQtSignalTarget::call(obj, methodInfo, args, true);
    if (result) { Py_DECREF(result); } 
    Py_DECREF(obj);
    return;
  }
}
  TaskManager::childEvent(arg__1);
}
void PythonQtShell_TaskManager::customEvent(QEvent*  arg__1)
{
if (_wrapper) {
  PyObject* obj = PyObject_GetAttrString((PyObject*)_wrapper, "customEvent");
  PyErr_Clear();
  if (obj && !PythonQtSlotFunction_Check(obj)) {
    static const char* argumentList[] ={"" , "QEvent*"};
    static const PythonQtMethodInfo* methodInfo = PythonQtMethodInfo::getCachedMethodInfoFromArgumentList(2, argumentList);
    void* args[2] = {NULL, (void*)&arg__1};
    PyObject* result = PythonQtSignalTarget::call(obj, methodInfo, args, true);
    if (result) { Py_DECREF(result); } 
    Py_DECREF(obj);
    return;
  }
}
  TaskManager::customEvent(arg__1);
}
bool  PythonQtShell_TaskManager::event(QEvent*  arg__1)
{
if (_wrapper) {
  PyObject* obj = PyObject_GetAttrString((PyObject*)_wrapper, "event");
  PyErr_Clear();
  if (obj && !PythonQtSlotFunction_Check(obj)) {
    static const char* argumentList[] ={"bool" , "QEvent*"};
    static const PythonQtMethodInfo* methodInfo = PythonQtMethodInfo::getCachedMethodInfoFromArgumentList(2, argumentList);
      bool returnValue;
    void* args[2] = {NULL, (void*)&arg__1};
    PyObject* result = PythonQtSignalTarget::call(obj, methodInfo, args, true);
    if (result) {
      args[0] = PythonQtConv::ConvertPythonToQt(methodInfo->parameters().at(0), result, false, NULL, &returnValue);
      if (args[0]!=&returnValue) {
        if (args[0]==NULL) {
          PythonQt::priv()->handleVirtualOverloadReturnError("event", methodInfo, result);
        } else {
          returnValue = *((bool*)args[0]);
        }
      }
    }
    if (result) { Py_DECREF(result); } 
    Py_DECREF(obj);
    return returnValue;
  }
}
  return TaskManager::event(arg__1);
}
bool  PythonQtShell_TaskManager::eventFilter(QObject*  arg__1, QEvent*  arg__2)
{
if (_wrapper) {
  PyObject* obj = PyObject_GetAttrString((PyObject*)_wrapper, "eventFilter");
  PyErr_Clear();
  if (obj && !PythonQtSlotFunction_Check(obj)) {
    static const char* argumentList[] ={"bool" , "QObject*" , "QEvent*"};
    static const PythonQtMethodInfo* methodInfo = PythonQtMethodInfo::getCachedMethodInfoFromArgumentList(3, argumentList);
      bool returnValue;
    void* args[3] = {NULL, (void*)&arg__1, (void*)&arg__2};
    PyObject* result = PythonQtSignalTarget::call(obj, methodInfo, args, true);
    if (result) {
      args[0] = PythonQtConv::ConvertPythonToQt(methodInfo->parameters().at(0), result, false, NULL, &returnValue);
      if (args[0]!=&returnValue) {
        if (args[0]==NULL) {
          PythonQt::priv()->handleVirtualOverloadReturnError("eventFilter", methodInfo, result);
        } else {
          returnValue = *((bool*)args[0]);
        }
      }
    }
    if (result) { Py_DECREF(result); } 
    Py_DECREF(obj);
    return returnValue;
  }
}
  return TaskManager::eventFilter(arg__1, arg__2);
}
void PythonQtShell_TaskManager::timerEvent(QTimerEvent*  arg__1)
{
if (_wrapper) {
  PyObject* obj = PyObject_GetAttrString((PyObject*)_wrapper, "timerEvent");
  PyErr_Clear();
  if (obj && !PythonQtSlotFunction_Check(obj)) {
    static const char* argumentList[] ={"" , "QTimerEvent*"};
    static const PythonQtMethodInfo* methodInfo = PythonQtMethodInfo::getCachedMethodInfoFromArgumentList(2, argumentList);
    void* args[2] = {NULL, (void*)&arg__1};
    PyObject* result = PythonQtSignalTarget::call(obj, methodInfo, args, true);
    if (result) { Py_DECREF(result); } 
    Py_DECREF(obj);
    return;
  }
}
  TaskManager::timerEvent(arg__1);
}
TaskManager* PythonQtWrapper_TaskManager::new_TaskManager(QObject*  parent)
{ 
return new PythonQtShell_TaskManager(parent); }

QList<TaskManager_Task >  PythonQtWrapper_TaskManager::GetTasks(TaskManager* theWrappedObject)
{
  return ( theWrappedObject->GetTasks());
}

void PythonQtWrapper_TaskManager::SetTaskBlocksLibraryScans(TaskManager* theWrappedObject, int  id)
{
  ( theWrappedObject->SetTaskBlocksLibraryScans(id));
}

void PythonQtWrapper_TaskManager::SetTaskFinished(TaskManager* theWrappedObject, int  id)
{
  ( theWrappedObject->SetTaskFinished(id));
}

void PythonQtWrapper_TaskManager::SetTaskProgress(TaskManager* theWrappedObject, int  id, int  progress, int  max)
{
  ( theWrappedObject->SetTaskProgress(id, progress, max));
}

int  PythonQtWrapper_TaskManager::StartTask(TaskManager* theWrappedObject, const QString&  name)
{
  return ( theWrappedObject->StartTask(name));
}



TaskManager_Task* PythonQtWrapper_TaskManager_Task::new_TaskManager_Task()
{ 
return new PythonQtShell_TaskManager_Task(); }



qint64  PythonQtShell_ThreadSafeNetworkDiskCache::cacheSize() const
{
if (_wrapper) {
  PyObject* obj = PyObject_GetAttrString((PyObject*)_wrapper, "cacheSize");
  PyErr_Clear();
  if (obj && !PythonQtSlotFunction_Check(obj)) {
    static const char* argumentList[] ={"qint64"};
    static const PythonQtMethodInfo* methodInfo = PythonQtMethodInfo::getCachedMethodInfoFromArgumentList(1, argumentList);
      qint64 returnValue;
    void* args[1] = {NULL};
    PyObject* result = PythonQtSignalTarget::call(obj, methodInfo, args, true);
    if (result) {
      args[0] = PythonQtConv::ConvertPythonToQt(methodInfo->parameters().at(0), result, false, NULL, &returnValue);
      if (args[0]!=&returnValue) {
        if (args[0]==NULL) {
          PythonQt::priv()->handleVirtualOverloadReturnError("cacheSize", methodInfo, result);
        } else {
          returnValue = *((qint64*)args[0]);
        }
      }
    }
    if (result) { Py_DECREF(result); } 
    Py_DECREF(obj);
    return returnValue;
  }
}
  return ThreadSafeNetworkDiskCache::cacheSize();
}
void PythonQtShell_ThreadSafeNetworkDiskCache::childEvent(QChildEvent*  arg__1)
{
if (_wrapper) {
  PyObject* obj = PyObject_GetAttrString((PyObject*)_wrapper, "childEvent");
  PyErr_Clear();
  if (obj && !PythonQtSlotFunction_Check(obj)) {
    static const char* argumentList[] ={"" , "QChildEvent*"};
    static const PythonQtMethodInfo* methodInfo = PythonQtMethodInfo::getCachedMethodInfoFromArgumentList(2, argumentList);
    void* args[2] = {NULL, (void*)&arg__1};
    PyObject* result = PythonQtSignalTarget::call(obj, methodInfo, args, true);
    if (result) { Py_DECREF(result); } 
    Py_DECREF(obj);
    return;
  }
}
  ThreadSafeNetworkDiskCache::childEvent(arg__1);
}
void PythonQtShell_ThreadSafeNetworkDiskCache::clear()
{
if (_wrapper) {
  PyObject* obj = PyObject_GetAttrString((PyObject*)_wrapper, "clear");
  PyErr_Clear();
  if (obj && !PythonQtSlotFunction_Check(obj)) {
    static const char* argumentList[] ={""};
    static const PythonQtMethodInfo* methodInfo = PythonQtMethodInfo::getCachedMethodInfoFromArgumentList(1, argumentList);
    void* args[1] = {NULL};
    PyObject* result = PythonQtSignalTarget::call(obj, methodInfo, args, true);
    if (result) { Py_DECREF(result); } 
    Py_DECREF(obj);
    return;
  }
}
  ThreadSafeNetworkDiskCache::clear();
}
void PythonQtShell_ThreadSafeNetworkDiskCache::customEvent(QEvent*  arg__1)
{
if (_wrapper) {
  PyObject* obj = PyObject_GetAttrString((PyObject*)_wrapper, "customEvent");
  PyErr_Clear();
  if (obj && !PythonQtSlotFunction_Check(obj)) {
    static const char* argumentList[] ={"" , "QEvent*"};
    static const PythonQtMethodInfo* methodInfo = PythonQtMethodInfo::getCachedMethodInfoFromArgumentList(2, argumentList);
    void* args[2] = {NULL, (void*)&arg__1};
    PyObject* result = PythonQtSignalTarget::call(obj, methodInfo, args, true);
    if (result) { Py_DECREF(result); } 
    Py_DECREF(obj);
    return;
  }
}
  ThreadSafeNetworkDiskCache::customEvent(arg__1);
}
QIODevice*  PythonQtShell_ThreadSafeNetworkDiskCache::data(const QUrl&  url)
{
if (_wrapper) {
  PyObject* obj = PyObject_GetAttrString((PyObject*)_wrapper, "data");
  PyErr_Clear();
  if (obj && !PythonQtSlotFunction_Check(obj)) {
    static const char* argumentList[] ={"QIODevice*" , "const QUrl&"};
    static const PythonQtMethodInfo* methodInfo = PythonQtMethodInfo::getCachedMethodInfoFromArgumentList(2, argumentList);
      QIODevice* returnValue;
    void* args[2] = {NULL, (void*)&url};
    PyObject* result = PythonQtSignalTarget::call(obj, methodInfo, args, true);
    if (result) {
      args[0] = PythonQtConv::ConvertPythonToQt(methodInfo->parameters().at(0), result, false, NULL, &returnValue);
      if (args[0]!=&returnValue) {
        if (args[0]==NULL) {
          PythonQt::priv()->handleVirtualOverloadReturnError("data", methodInfo, result);
        } else {
          returnValue = *((QIODevice**)args[0]);
        }
      }
    }
    if (result) { Py_DECREF(result); } 
    Py_DECREF(obj);
    return returnValue;
  }
}
  return ThreadSafeNetworkDiskCache::data(url);
}
bool  PythonQtShell_ThreadSafeNetworkDiskCache::event(QEvent*  arg__1)
{
if (_wrapper) {
  PyObject* obj = PyObject_GetAttrString((PyObject*)_wrapper, "event");
  PyErr_Clear();
  if (obj && !PythonQtSlotFunction_Check(obj)) {
    static const char* argumentList[] ={"bool" , "QEvent*"};
    static const PythonQtMethodInfo* methodInfo = PythonQtMethodInfo::getCachedMethodInfoFromArgumentList(2, argumentList);
      bool returnValue;
    void* args[2] = {NULL, (void*)&arg__1};
    PyObject* result = PythonQtSignalTarget::call(obj, methodInfo, args, true);
    if (result) {
      args[0] = PythonQtConv::ConvertPythonToQt(methodInfo->parameters().at(0), result, false, NULL, &returnValue);
      if (args[0]!=&returnValue) {
        if (args[0]==NULL) {
          PythonQt::priv()->handleVirtualOverloadReturnError("event", methodInfo, result);
        } else {
          returnValue = *((bool*)args[0]);
        }
      }
    }
    if (result) { Py_DECREF(result); } 
    Py_DECREF(obj);
    return returnValue;
  }
}
  return ThreadSafeNetworkDiskCache::event(arg__1);
}
bool  PythonQtShell_ThreadSafeNetworkDiskCache::eventFilter(QObject*  arg__1, QEvent*  arg__2)
{
if (_wrapper) {
  PyObject* obj = PyObject_GetAttrString((PyObject*)_wrapper, "eventFilter");
  PyErr_Clear();
  if (obj && !PythonQtSlotFunction_Check(obj)) {
    static const char* argumentList[] ={"bool" , "QObject*" , "QEvent*"};
    static const PythonQtMethodInfo* methodInfo = PythonQtMethodInfo::getCachedMethodInfoFromArgumentList(3, argumentList);
      bool returnValue;
    void* args[3] = {NULL, (void*)&arg__1, (void*)&arg__2};
    PyObject* result = PythonQtSignalTarget::call(obj, methodInfo, args, true);
    if (result) {
      args[0] = PythonQtConv::ConvertPythonToQt(methodInfo->parameters().at(0), result, false, NULL, &returnValue);
      if (args[0]!=&returnValue) {
        if (args[0]==NULL) {
          PythonQt::priv()->handleVirtualOverloadReturnError("eventFilter", methodInfo, result);
        } else {
          returnValue = *((bool*)args[0]);
        }
      }
    }
    if (result) { Py_DECREF(result); } 
    Py_DECREF(obj);
    return returnValue;
  }
}
  return ThreadSafeNetworkDiskCache::eventFilter(arg__1, arg__2);
}
void PythonQtShell_ThreadSafeNetworkDiskCache::insert(QIODevice*  device)
{
if (_wrapper) {
  PyObject* obj = PyObject_GetAttrString((PyObject*)_wrapper, "insert");
  PyErr_Clear();
  if (obj && !PythonQtSlotFunction_Check(obj)) {
    static const char* argumentList[] ={"" , "QIODevice*"};
    static const PythonQtMethodInfo* methodInfo = PythonQtMethodInfo::getCachedMethodInfoFromArgumentList(2, argumentList);
    void* args[2] = {NULL, (void*)&device};
    PyObject* result = PythonQtSignalTarget::call(obj, methodInfo, args, true);
    if (result) { Py_DECREF(result); } 
    Py_DECREF(obj);
    return;
  }
}
  ThreadSafeNetworkDiskCache::insert(device);
}
QNetworkCacheMetaData  PythonQtShell_ThreadSafeNetworkDiskCache::metaData(const QUrl&  url)
{
if (_wrapper) {
  PyObject* obj = PyObject_GetAttrString((PyObject*)_wrapper, "metaData");
  PyErr_Clear();
  if (obj && !PythonQtSlotFunction_Check(obj)) {
    static const char* argumentList[] ={"QNetworkCacheMetaData" , "const QUrl&"};
    static const PythonQtMethodInfo* methodInfo = PythonQtMethodInfo::getCachedMethodInfoFromArgumentList(2, argumentList);
      QNetworkCacheMetaData returnValue;
    void* args[2] = {NULL, (void*)&url};
    PyObject* result = PythonQtSignalTarget::call(obj, methodInfo, args, true);
    if (result) {
      args[0] = PythonQtConv::ConvertPythonToQt(methodInfo->parameters().at(0), result, false, NULL, &returnValue);
      if (args[0]!=&returnValue) {
        if (args[0]==NULL) {
          PythonQt::priv()->handleVirtualOverloadReturnError("metaData", methodInfo, result);
        } else {
          returnValue = *((QNetworkCacheMetaData*)args[0]);
        }
      }
    }
    if (result) { Py_DECREF(result); } 
    Py_DECREF(obj);
    return returnValue;
  }
}
  return ThreadSafeNetworkDiskCache::metaData(url);
}
QIODevice*  PythonQtShell_ThreadSafeNetworkDiskCache::prepare(const QNetworkCacheMetaData&  metaData)
{
if (_wrapper) {
  PyObject* obj = PyObject_GetAttrString((PyObject*)_wrapper, "prepare");
  PyErr_Clear();
  if (obj && !PythonQtSlotFunction_Check(obj)) {
    static const char* argumentList[] ={"QIODevice*" , "const QNetworkCacheMetaData&"};
    static const PythonQtMethodInfo* methodInfo = PythonQtMethodInfo::getCachedMethodInfoFromArgumentList(2, argumentList);
      QIODevice* returnValue;
    void* args[2] = {NULL, (void*)&metaData};
    PyObject* result = PythonQtSignalTarget::call(obj, methodInfo, args, true);
    if (result) {
      args[0] = PythonQtConv::ConvertPythonToQt(methodInfo->parameters().at(0), result, false, NULL, &returnValue);
      if (args[0]!=&returnValue) {
        if (args[0]==NULL) {
          PythonQt::priv()->handleVirtualOverloadReturnError("prepare", methodInfo, result);
        } else {
          returnValue = *((QIODevice**)args[0]);
        }
      }
    }
    if (result) { Py_DECREF(result); } 
    Py_DECREF(obj);
    return returnValue;
  }
}
  return ThreadSafeNetworkDiskCache::prepare(metaData);
}
bool  PythonQtShell_ThreadSafeNetworkDiskCache::remove(const QUrl&  url)
{
if (_wrapper) {
  PyObject* obj = PyObject_GetAttrString((PyObject*)_wrapper, "remove");
  PyErr_Clear();
  if (obj && !PythonQtSlotFunction_Check(obj)) {
    static const char* argumentList[] ={"bool" , "const QUrl&"};
    static const PythonQtMethodInfo* methodInfo = PythonQtMethodInfo::getCachedMethodInfoFromArgumentList(2, argumentList);
      bool returnValue;
    void* args[2] = {NULL, (void*)&url};
    PyObject* result = PythonQtSignalTarget::call(obj, methodInfo, args, true);
    if (result) {
      args[0] = PythonQtConv::ConvertPythonToQt(methodInfo->parameters().at(0), result, false, NULL, &returnValue);
      if (args[0]!=&returnValue) {
        if (args[0]==NULL) {
          PythonQt::priv()->handleVirtualOverloadReturnError("remove", methodInfo, result);
        } else {
          returnValue = *((bool*)args[0]);
        }
      }
    }
    if (result) { Py_DECREF(result); } 
    Py_DECREF(obj);
    return returnValue;
  }
}
  return ThreadSafeNetworkDiskCache::remove(url);
}
void PythonQtShell_ThreadSafeNetworkDiskCache::timerEvent(QTimerEvent*  arg__1)
{
if (_wrapper) {
  PyObject* obj = PyObject_GetAttrString((PyObject*)_wrapper, "timerEvent");
  PyErr_Clear();
  if (obj && !PythonQtSlotFunction_Check(obj)) {
    static const char* argumentList[] ={"" , "QTimerEvent*"};
    static const PythonQtMethodInfo* methodInfo = PythonQtMethodInfo::getCachedMethodInfoFromArgumentList(2, argumentList);
    void* args[2] = {NULL, (void*)&arg__1};
    PyObject* result = PythonQtSignalTarget::call(obj, methodInfo, args, true);
    if (result) { Py_DECREF(result); } 
    Py_DECREF(obj);
    return;
  }
}
  ThreadSafeNetworkDiskCache::timerEvent(arg__1);
}
void PythonQtShell_ThreadSafeNetworkDiskCache::updateMetaData(const QNetworkCacheMetaData&  metaData)
{
if (_wrapper) {
  PyObject* obj = PyObject_GetAttrString((PyObject*)_wrapper, "updateMetaData");
  PyErr_Clear();
  if (obj && !PythonQtSlotFunction_Check(obj)) {
    static const char* argumentList[] ={"" , "const QNetworkCacheMetaData&"};
    static const PythonQtMethodInfo* methodInfo = PythonQtMethodInfo::getCachedMethodInfoFromArgumentList(2, argumentList);
    void* args[2] = {NULL, (void*)&metaData};
    PyObject* result = PythonQtSignalTarget::call(obj, methodInfo, args, true);
    if (result) { Py_DECREF(result); } 
    Py_DECREF(obj);
    return;
  }
}
  ThreadSafeNetworkDiskCache::updateMetaData(metaData);
}
ThreadSafeNetworkDiskCache* PythonQtWrapper_ThreadSafeNetworkDiskCache::new_ThreadSafeNetworkDiskCache(QObject*  parent)
{ 
return new PythonQtShell_ThreadSafeNetworkDiskCache(parent); }

qint64  PythonQtWrapper_ThreadSafeNetworkDiskCache::cacheSize(ThreadSafeNetworkDiskCache* theWrappedObject) const
{
  return ( ((PythonQtPublicPromoter_ThreadSafeNetworkDiskCache*)theWrappedObject)->promoted_cacheSize());
}

void PythonQtWrapper_ThreadSafeNetworkDiskCache::clear(ThreadSafeNetworkDiskCache* theWrappedObject)
{
  ( ((PythonQtPublicPromoter_ThreadSafeNetworkDiskCache*)theWrappedObject)->promoted_clear());
}

QIODevice*  PythonQtWrapper_ThreadSafeNetworkDiskCache::data(ThreadSafeNetworkDiskCache* theWrappedObject, const QUrl&  url)
{
  return ( ((PythonQtPublicPromoter_ThreadSafeNetworkDiskCache*)theWrappedObject)->promoted_data(url));
}

void PythonQtWrapper_ThreadSafeNetworkDiskCache::insert(ThreadSafeNetworkDiskCache* theWrappedObject, QIODevice*  device)
{
  ( ((PythonQtPublicPromoter_ThreadSafeNetworkDiskCache*)theWrappedObject)->promoted_insert(device));
}

QNetworkCacheMetaData  PythonQtWrapper_ThreadSafeNetworkDiskCache::metaData(ThreadSafeNetworkDiskCache* theWrappedObject, const QUrl&  url)
{
  return ( ((PythonQtPublicPromoter_ThreadSafeNetworkDiskCache*)theWrappedObject)->promoted_metaData(url));
}

QIODevice*  PythonQtWrapper_ThreadSafeNetworkDiskCache::prepare(ThreadSafeNetworkDiskCache* theWrappedObject, const QNetworkCacheMetaData&  metaData)
{
  return ( ((PythonQtPublicPromoter_ThreadSafeNetworkDiskCache*)theWrappedObject)->promoted_prepare(metaData));
}

bool  PythonQtWrapper_ThreadSafeNetworkDiskCache::remove(ThreadSafeNetworkDiskCache* theWrappedObject, const QUrl&  url)
{
  return ( ((PythonQtPublicPromoter_ThreadSafeNetworkDiskCache*)theWrappedObject)->promoted_remove(url));
}

void PythonQtWrapper_ThreadSafeNetworkDiskCache::updateMetaData(ThreadSafeNetworkDiskCache* theWrappedObject, const QNetworkCacheMetaData&  metaData)
{
  ( ((PythonQtPublicPromoter_ThreadSafeNetworkDiskCache*)theWrappedObject)->promoted_updateMetaData(metaData));
}



UrlHandler_LoadResult  PythonQtShell_UrlHandler::LoadNext(const QUrl&  url)
{
if (_wrapper) {
  PyObject* obj = PyObject_GetAttrString((PyObject*)_wrapper, "LoadNext");
  PyErr_Clear();
  if (obj && !PythonQtSlotFunction_Check(obj)) {
    static const char* argumentList[] ={"UrlHandler_LoadResult" , "const QUrl&"};
    static const PythonQtMethodInfo* methodInfo = PythonQtMethodInfo::getCachedMethodInfoFromArgumentList(2, argumentList);
      UrlHandler_LoadResult returnValue;
    void* args[2] = {NULL, (void*)&url};
    PyObject* result = PythonQtSignalTarget::call(obj, methodInfo, args, true);
    if (result) {
      args[0] = PythonQtConv::ConvertPythonToQt(methodInfo->parameters().at(0), result, false, NULL, &returnValue);
      if (args[0]!=&returnValue) {
        if (args[0]==NULL) {
          PythonQt::priv()->handleVirtualOverloadReturnError("LoadNext", methodInfo, result);
        } else {
          returnValue = *((UrlHandler_LoadResult*)args[0]);
        }
      }
    }
    if (result) { Py_DECREF(result); } 
    Py_DECREF(obj);
    return returnValue;
  }
}
  return UrlHandler::LoadNext(url);
}
UrlHandler_LoadResult  PythonQtShell_UrlHandler::StartLoading(const QUrl&  url)
{
if (_wrapper) {
  PyObject* obj = PyObject_GetAttrString((PyObject*)_wrapper, "StartLoading");
  PyErr_Clear();
  if (obj && !PythonQtSlotFunction_Check(obj)) {
    static const char* argumentList[] ={"UrlHandler_LoadResult" , "const QUrl&"};
    static const PythonQtMethodInfo* methodInfo = PythonQtMethodInfo::getCachedMethodInfoFromArgumentList(2, argumentList);
      UrlHandler_LoadResult returnValue;
    void* args[2] = {NULL, (void*)&url};
    PyObject* result = PythonQtSignalTarget::call(obj, methodInfo, args, true);
    if (result) {
      args[0] = PythonQtConv::ConvertPythonToQt(methodInfo->parameters().at(0), result, false, NULL, &returnValue);
      if (args[0]!=&returnValue) {
        if (args[0]==NULL) {
          PythonQt::priv()->handleVirtualOverloadReturnError("StartLoading", methodInfo, result);
        } else {
          returnValue = *((UrlHandler_LoadResult*)args[0]);
        }
      }
    }
    if (result) { Py_DECREF(result); } 
    Py_DECREF(obj);
    return returnValue;
  }
}
  return UrlHandler::StartLoading(url);
}
void PythonQtShell_UrlHandler::childEvent(QChildEvent*  arg__1)
{
if (_wrapper) {
  PyObject* obj = PyObject_GetAttrString((PyObject*)_wrapper, "childEvent");
  PyErr_Clear();
  if (obj && !PythonQtSlotFunction_Check(obj)) {
    static const char* argumentList[] ={"" , "QChildEvent*"};
    static const PythonQtMethodInfo* methodInfo = PythonQtMethodInfo::getCachedMethodInfoFromArgumentList(2, argumentList);
    void* args[2] = {NULL, (void*)&arg__1};
    PyObject* result = PythonQtSignalTarget::call(obj, methodInfo, args, true);
    if (result) { Py_DECREF(result); } 
    Py_DECREF(obj);
    return;
  }
}
  UrlHandler::childEvent(arg__1);
}
void PythonQtShell_UrlHandler::customEvent(QEvent*  arg__1)
{
if (_wrapper) {
  PyObject* obj = PyObject_GetAttrString((PyObject*)_wrapper, "customEvent");
  PyErr_Clear();
  if (obj && !PythonQtSlotFunction_Check(obj)) {
    static const char* argumentList[] ={"" , "QEvent*"};
    static const PythonQtMethodInfo* methodInfo = PythonQtMethodInfo::getCachedMethodInfoFromArgumentList(2, argumentList);
    void* args[2] = {NULL, (void*)&arg__1};
    PyObject* result = PythonQtSignalTarget::call(obj, methodInfo, args, true);
    if (result) { Py_DECREF(result); } 
    Py_DECREF(obj);
    return;
  }
}
  UrlHandler::customEvent(arg__1);
}
bool  PythonQtShell_UrlHandler::event(QEvent*  arg__1)
{
if (_wrapper) {
  PyObject* obj = PyObject_GetAttrString((PyObject*)_wrapper, "event");
  PyErr_Clear();
  if (obj && !PythonQtSlotFunction_Check(obj)) {
    static const char* argumentList[] ={"bool" , "QEvent*"};
    static const PythonQtMethodInfo* methodInfo = PythonQtMethodInfo::getCachedMethodInfoFromArgumentList(2, argumentList);
      bool returnValue;
    void* args[2] = {NULL, (void*)&arg__1};
    PyObject* result = PythonQtSignalTarget::call(obj, methodInfo, args, true);
    if (result) {
      args[0] = PythonQtConv::ConvertPythonToQt(methodInfo->parameters().at(0), result, false, NULL, &returnValue);
      if (args[0]!=&returnValue) {
        if (args[0]==NULL) {
          PythonQt::priv()->handleVirtualOverloadReturnError("event", methodInfo, result);
        } else {
          returnValue = *((bool*)args[0]);
        }
      }
    }
    if (result) { Py_DECREF(result); } 
    Py_DECREF(obj);
    return returnValue;
  }
}
  return UrlHandler::event(arg__1);
}
bool  PythonQtShell_UrlHandler::eventFilter(QObject*  arg__1, QEvent*  arg__2)
{
if (_wrapper) {
  PyObject* obj = PyObject_GetAttrString((PyObject*)_wrapper, "eventFilter");
  PyErr_Clear();
  if (obj && !PythonQtSlotFunction_Check(obj)) {
    static const char* argumentList[] ={"bool" , "QObject*" , "QEvent*"};
    static const PythonQtMethodInfo* methodInfo = PythonQtMethodInfo::getCachedMethodInfoFromArgumentList(3, argumentList);
      bool returnValue;
    void* args[3] = {NULL, (void*)&arg__1, (void*)&arg__2};
    PyObject* result = PythonQtSignalTarget::call(obj, methodInfo, args, true);
    if (result) {
      args[0] = PythonQtConv::ConvertPythonToQt(methodInfo->parameters().at(0), result, false, NULL, &returnValue);
      if (args[0]!=&returnValue) {
        if (args[0]==NULL) {
          PythonQt::priv()->handleVirtualOverloadReturnError("eventFilter", methodInfo, result);
        } else {
          returnValue = *((bool*)args[0]);
        }
      }
    }
    if (result) { Py_DECREF(result); } 
    Py_DECREF(obj);
    return returnValue;
  }
}
  return UrlHandler::eventFilter(arg__1, arg__2);
}
QString  PythonQtShell_UrlHandler::scheme() const
{
if (_wrapper) {
  PyObject* obj = PyObject_GetAttrString((PyObject*)_wrapper, "scheme");
  PyErr_Clear();
  if (obj && !PythonQtSlotFunction_Check(obj)) {
    static const char* argumentList[] ={"QString"};
    static const PythonQtMethodInfo* methodInfo = PythonQtMethodInfo::getCachedMethodInfoFromArgumentList(1, argumentList);
      QString returnValue;
    void* args[1] = {NULL};
    PyObject* result = PythonQtSignalTarget::call(obj, methodInfo, args, true);
    if (result) {
      args[0] = PythonQtConv::ConvertPythonToQt(methodInfo->parameters().at(0), result, false, NULL, &returnValue);
      if (args[0]!=&returnValue) {
        if (args[0]==NULL) {
          PythonQt::priv()->handleVirtualOverloadReturnError("scheme", methodInfo, result);
        } else {
          returnValue = *((QString*)args[0]);
        }
      }
    }
    if (result) { Py_DECREF(result); } 
    Py_DECREF(obj);
    return returnValue;
  }
}
  return QString();
}
void PythonQtShell_UrlHandler::timerEvent(QTimerEvent*  arg__1)
{
if (_wrapper) {
  PyObject* obj = PyObject_GetAttrString((PyObject*)_wrapper, "timerEvent");
  PyErr_Clear();
  if (obj && !PythonQtSlotFunction_Check(obj)) {
    static const char* argumentList[] ={"" , "QTimerEvent*"};
    static const PythonQtMethodInfo* methodInfo = PythonQtMethodInfo::getCachedMethodInfoFromArgumentList(2, argumentList);
    void* args[2] = {NULL, (void*)&arg__1};
    PyObject* result = PythonQtSignalTarget::call(obj, methodInfo, args, true);
    if (result) { Py_DECREF(result); } 
    Py_DECREF(obj);
    return;
  }
}
  UrlHandler::timerEvent(arg__1);
}
UrlHandler* PythonQtWrapper_UrlHandler::new_UrlHandler(QObject*  parent)
{ 
return new PythonQtShell_UrlHandler(parent); }

UrlHandler_LoadResult  PythonQtWrapper_UrlHandler::LoadNext(UrlHandler* theWrappedObject, const QUrl&  url)
{
  return ( ((PythonQtPublicPromoter_UrlHandler*)theWrappedObject)->promoted_LoadNext(url));
}

UrlHandler_LoadResult  PythonQtWrapper_UrlHandler::StartLoading(UrlHandler* theWrappedObject, const QUrl&  url)
{
  return ( ((PythonQtPublicPromoter_UrlHandler*)theWrappedObject)->promoted_StartLoading(url));
}



UrlHandler_LoadResult* PythonQtWrapper_UrlHandler_LoadResult::new_UrlHandler_LoadResult(const QUrl&  original_url, UrlHandler_LoadResult::Type  type, const QUrl&  media_url)
{ 
return new PythonQtShell_UrlHandler_LoadResult(original_url, type, media_url); }


