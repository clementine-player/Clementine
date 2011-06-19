#include "clementine0.h"
#include <PythonQtConversion.h>
#include <PythonQtMethodInfo.h>
#include <PythonQtSignalReceiver.h>
#include <QVariant>
#include <albumcoverfetcher.h>
#include <coverprovider.h>
#include <coverproviders.h>
#include <directory.h>
#include <librarybackend.h>
#include <libraryquery.h>
#include <player.h>
#include <playlist.h>
#include <playlistbackend.h>
#include <playlistcontainer.h>
#include <playlistitem.h>
#include <playlistmanager.h>
#include <playlistparser.h>
#include <playlistsequence.h>
#include <qabstractitemmodel.h>
#include <qabstractnetworkcache.h>
#include <qbytearray.h>
#include <qcoreevent.h>
#include <qcursor.h>
#include <qdatastream.h>
#include <qdir.h>
#include <qfont.h>
#include <qicon.h>
#include <qimage.h>
#include <qiodevice.h>
#include <qitemselectionmodel.h>
#include <qkeysequence.h>
#include <qlist.h>
#include <qlocale.h>
#include <qmargins.h>
#include <qnetworkrequest.h>
#include <qobject.h>
#include <qpaintdevice.h>
#include <qpalette.h>
#include <qpixmap.h>
#include <qpoint.h>
#include <qrect.h>
#include <qregion.h>
#include <qsize.h>
#include <qsizepolicy.h>
#include <qstandarditemmodel.h>
#include <qstringlist.h>
#include <qurl.h>
#include <qwidget.h>
#include <radiomodel.h>
#include <radioservice.h>
#include <song.h>
#include <specialplaylisttype.h>
#include <taskmanager.h>
#include <urlhandler.h>

void PythonQtWrapper_AlbumCoverFetcherSearch::Start(AlbumCoverFetcherSearch* theWrappedObject)
{
  ( theWrappedObject->Start());
}



void PythonQtShell_CoverProvider::CancelSearch(int  id)
{
if (_wrapper) {
  PyObject* obj = PyObject_GetAttrString((PyObject*)_wrapper, "CancelSearch");
  PyErr_Clear();
  if (obj && !PythonQtSlotFunction_Check(obj)) {
    static const char* argumentList[] ={"" , "int"};
    static const PythonQtMethodInfo* methodInfo = PythonQtMethodInfo::getCachedMethodInfoFromArgumentList(2, argumentList);
    void* args[2] = {NULL, (void*)&id};
    PyObject* result = PythonQtSignalTarget::call(obj, methodInfo, args, true);
    if (result) { Py_DECREF(result); } 
    Py_DECREF(obj);
    return;
  }
  Py_XDECREF(obj);
}
  CoverProvider::CancelSearch(id);
}
bool  PythonQtShell_CoverProvider::StartSearch(const QString&  query, int  id)
{
if (_wrapper) {
  PyObject* obj = PyObject_GetAttrString((PyObject*)_wrapper, "StartSearch");
  PyErr_Clear();
  if (obj && !PythonQtSlotFunction_Check(obj)) {
    static const char* argumentList[] ={"bool" , "const QString&" , "int"};
    static const PythonQtMethodInfo* methodInfo = PythonQtMethodInfo::getCachedMethodInfoFromArgumentList(3, argumentList);
      bool returnValue = 0;
    void* args[3] = {NULL, (void*)&query, (void*)&id};
    PyObject* result = PythonQtSignalTarget::call(obj, methodInfo, args, true);
    if (result) {
      args[0] = PythonQtConv::ConvertPythonToQt(methodInfo->parameters().at(0), result, false, NULL, &returnValue);
      if (args[0]!=&returnValue) {
        if (args[0]==NULL) {
          PythonQt::priv()->handleVirtualOverloadReturnError("StartSearch", methodInfo, result);
        } else {
          returnValue = *((bool*)args[0]);
        }
      }
    }
    if (result) { Py_DECREF(result); } 
    Py_DECREF(obj);
    return returnValue;
  }
  Py_XDECREF(obj);
}
  return bool();
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
  Py_XDECREF(obj);
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
  Py_XDECREF(obj);
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
      bool returnValue = 0;
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
  Py_XDECREF(obj);
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
      bool returnValue = 0;
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
  Py_XDECREF(obj);
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
  Py_XDECREF(obj);
}
  CoverProvider::timerEvent(arg__1);
}
CoverProvider* PythonQtWrapper_CoverProvider::new_CoverProvider(const QString&  name, QObject*  parent)
{ 
return new PythonQtShell_CoverProvider(name, parent); }

void PythonQtWrapper_CoverProvider::CancelSearch(CoverProvider* theWrappedObject, int  id)
{
  ( ((PythonQtPublicPromoter_CoverProvider*)theWrappedObject)->promoted_CancelSearch(id));
}

QString  PythonQtWrapper_CoverProvider::name(CoverProvider* theWrappedObject) const
{
  return ( theWrappedObject->name());
}



void PythonQtWrapper_CoverProviders::AddProvider(CoverProviders* theWrappedObject, CoverProvider*  provider)
{
  ( theWrappedObject->AddProvider(provider));
}

bool  PythonQtWrapper_CoverProviders::HasAnyProviders(CoverProviders* theWrappedObject) const
{
  return ( theWrappedObject->HasAnyProviders());
}

QList<CoverProvider* >  PythonQtWrapper_CoverProviders::List(CoverProviders* theWrappedObject) const
{
  return ( theWrappedObject->List());
}

int  PythonQtWrapper_CoverProviders::NextId(CoverProviders* theWrappedObject)
{
  return ( theWrappedObject->NextId());
}

void PythonQtWrapper_CoverProviders::RemoveProvider(CoverProviders* theWrappedObject, CoverProvider*  provider)
{
  ( theWrappedObject->RemoveProvider(provider));
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
  Py_XDECREF(obj);
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
  Py_XDECREF(obj);
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
      bool returnValue = 0;
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
  Py_XDECREF(obj);
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
  Py_XDECREF(obj);
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
  Py_XDECREF(obj);
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
  Py_XDECREF(obj);
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
  Py_XDECREF(obj);
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
  Py_XDECREF(obj);
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
  Py_XDECREF(obj);
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
  Py_XDECREF(obj);
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
  Py_XDECREF(obj);
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
  Py_XDECREF(obj);
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
  Py_XDECREF(obj);
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
  Py_XDECREF(obj);
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
  Py_XDECREF(obj);
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
  Py_XDECREF(obj);
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
      bool returnValue = 0;
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
  Py_XDECREF(obj);
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
  Py_XDECREF(obj);
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
  Py_XDECREF(obj);
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
  Py_XDECREF(obj);
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
  Py_XDECREF(obj);
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
  Py_XDECREF(obj);
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
  Py_XDECREF(obj);
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
  Py_XDECREF(obj);
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
      bool returnValue = 0;
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
  Py_XDECREF(obj);
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
      bool returnValue = 0;
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
  Py_XDECREF(obj);
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
  Py_XDECREF(obj);
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
  Py_XDECREF(obj);
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
  Py_XDECREF(obj);
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
      bool returnValue = 0;
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
  Py_XDECREF(obj);
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
  Py_XDECREF(obj);
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
  Py_XDECREF(obj);
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
  Py_XDECREF(obj);
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
  Py_XDECREF(obj);
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
  Py_XDECREF(obj);
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
  Py_XDECREF(obj);
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
  Py_XDECREF(obj);
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
  Py_XDECREF(obj);
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
  Py_XDECREF(obj);
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
  Py_XDECREF(obj);
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
  Py_XDECREF(obj);
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
  Py_XDECREF(obj);
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
  Py_XDECREF(obj);
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
      bool returnValue = 0;
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
  Py_XDECREF(obj);
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
  Py_XDECREF(obj);
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
  Py_XDECREF(obj);
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
  Py_XDECREF(obj);
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
  Py_XDECREF(obj);
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
  Py_XDECREF(obj);
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
  Py_XDECREF(obj);
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
  Py_XDECREF(obj);
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
      bool returnValue = 0;
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
  Py_XDECREF(obj);
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
      bool returnValue = 0;
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
  Py_XDECREF(obj);
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
  Py_XDECREF(obj);
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
  Py_XDECREF(obj);
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
      QNetworkReply* returnValue = 0;
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
  Py_XDECREF(obj);
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
  Py_XDECREF(obj);
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
      bool returnValue = 0;
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
  Py_XDECREF(obj);
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
      bool returnValue = 0;
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
  Py_XDECREF(obj);
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
  Py_XDECREF(obj);
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
  Py_XDECREF(obj);
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
  Py_XDECREF(obj);
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
      bool returnValue = 0;
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
  Py_XDECREF(obj);
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
      bool returnValue = 0;
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
  Py_XDECREF(obj);
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
  Py_XDECREF(obj);
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



void PythonQtShell_Player::CurrentMetadataChanged(const Song&  metadata)
{
if (_wrapper) {
  PyObject* obj = PyObject_GetAttrString((PyObject*)_wrapper, "CurrentMetadataChanged");
  PyErr_Clear();
  if (obj && !PythonQtSlotFunction_Check(obj)) {
    static const char* argumentList[] ={"" , "const Song&"};
    static const PythonQtMethodInfo* methodInfo = PythonQtMethodInfo::getCachedMethodInfoFromArgumentList(2, argumentList);
    void* args[2] = {NULL, (void*)&metadata};
    PyObject* result = PythonQtSignalTarget::call(obj, methodInfo, args, true);
    if (result) { Py_DECREF(result); } 
    Py_DECREF(obj);
    return;
  }
  Py_XDECREF(obj);
}
  Player::CurrentMetadataChanged(metadata);
}
PlaylistItemPtr  PythonQtShell_Player::GetCurrentItem() const
{
if (_wrapper) {
  PyObject* obj = PyObject_GetAttrString((PyObject*)_wrapper, "GetCurrentItem");
  PyErr_Clear();
  if (obj && !PythonQtSlotFunction_Check(obj)) {
    static const char* argumentList[] ={"PlaylistItemPtr"};
    static const PythonQtMethodInfo* methodInfo = PythonQtMethodInfo::getCachedMethodInfoFromArgumentList(1, argumentList);
      PlaylistItemPtr returnValue;
    void* args[1] = {NULL};
    PyObject* result = PythonQtSignalTarget::call(obj, methodInfo, args, true);
    if (result) {
      args[0] = PythonQtConv::ConvertPythonToQt(methodInfo->parameters().at(0), result, false, NULL, &returnValue);
      if (args[0]!=&returnValue) {
        if (args[0]==NULL) {
          PythonQt::priv()->handleVirtualOverloadReturnError("GetCurrentItem", methodInfo, result);
        } else {
          returnValue = *((PlaylistItemPtr*)args[0]);
        }
      }
    }
    if (result) { Py_DECREF(result); } 
    Py_DECREF(obj);
    return returnValue;
  }
  Py_XDECREF(obj);
}
  return Player::GetCurrentItem();
}
PlaylistItemPtr  PythonQtShell_Player::GetItemAt(int  pos) const
{
if (_wrapper) {
  PyObject* obj = PyObject_GetAttrString((PyObject*)_wrapper, "GetItemAt");
  PyErr_Clear();
  if (obj && !PythonQtSlotFunction_Check(obj)) {
    static const char* argumentList[] ={"PlaylistItemPtr" , "int"};
    static const PythonQtMethodInfo* methodInfo = PythonQtMethodInfo::getCachedMethodInfoFromArgumentList(2, argumentList);
      PlaylistItemPtr returnValue;
    void* args[2] = {NULL, (void*)&pos};
    PyObject* result = PythonQtSignalTarget::call(obj, methodInfo, args, true);
    if (result) {
      args[0] = PythonQtConv::ConvertPythonToQt(methodInfo->parameters().at(0), result, false, NULL, &returnValue);
      if (args[0]!=&returnValue) {
        if (args[0]==NULL) {
          PythonQt::priv()->handleVirtualOverloadReturnError("GetItemAt", methodInfo, result);
        } else {
          returnValue = *((PlaylistItemPtr*)args[0]);
        }
      }
    }
    if (result) { Py_DECREF(result); } 
    Py_DECREF(obj);
    return returnValue;
  }
  Py_XDECREF(obj);
}
  return Player::GetItemAt(pos);
}
Engine::State  PythonQtShell_Player::GetState() const
{
if (_wrapper) {
  PyObject* obj = PyObject_GetAttrString((PyObject*)_wrapper, "GetState");
  PyErr_Clear();
  if (obj && !PythonQtSlotFunction_Check(obj)) {
    static const char* argumentList[] ={"Engine::State"};
    static const PythonQtMethodInfo* methodInfo = PythonQtMethodInfo::getCachedMethodInfoFromArgumentList(1, argumentList);
      Engine::State returnValue = Engine::State(0);
    void* args[1] = {NULL};
    PyObject* result = PythonQtSignalTarget::call(obj, methodInfo, args, true);
    if (result) {
      args[0] = PythonQtConv::ConvertPythonToQt(methodInfo->parameters().at(0), result, false, NULL, &returnValue);
      if (args[0]!=&returnValue) {
        if (args[0]==NULL) {
          PythonQt::priv()->handleVirtualOverloadReturnError("GetState", methodInfo, result);
        } else {
          returnValue = *((Engine::State*)args[0]);
        }
      }
    }
    if (result) { Py_DECREF(result); } 
    Py_DECREF(obj);
    return returnValue;
  }
  Py_XDECREF(obj);
}
  return Player::GetState();
}
int  PythonQtShell_Player::GetVolume() const
{
if (_wrapper) {
  PyObject* obj = PyObject_GetAttrString((PyObject*)_wrapper, "GetVolume");
  PyErr_Clear();
  if (obj && !PythonQtSlotFunction_Check(obj)) {
    static const char* argumentList[] ={"int"};
    static const PythonQtMethodInfo* methodInfo = PythonQtMethodInfo::getCachedMethodInfoFromArgumentList(1, argumentList);
      int returnValue = 0;
    void* args[1] = {NULL};
    PyObject* result = PythonQtSignalTarget::call(obj, methodInfo, args, true);
    if (result) {
      args[0] = PythonQtConv::ConvertPythonToQt(methodInfo->parameters().at(0), result, false, NULL, &returnValue);
      if (args[0]!=&returnValue) {
        if (args[0]==NULL) {
          PythonQt::priv()->handleVirtualOverloadReturnError("GetVolume", methodInfo, result);
        } else {
          returnValue = *((int*)args[0]);
        }
      }
    }
    if (result) { Py_DECREF(result); } 
    Py_DECREF(obj);
    return returnValue;
  }
  Py_XDECREF(obj);
}
  return Player::GetVolume();
}
void PythonQtShell_Player::Mute()
{
if (_wrapper) {
  PyObject* obj = PyObject_GetAttrString((PyObject*)_wrapper, "Mute");
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
  Py_XDECREF(obj);
}
  Player::Mute();
}
void PythonQtShell_Player::Next()
{
if (_wrapper) {
  PyObject* obj = PyObject_GetAttrString((PyObject*)_wrapper, "Next");
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
  Py_XDECREF(obj);
}
  Player::Next();
}
void PythonQtShell_Player::Pause()
{
if (_wrapper) {
  PyObject* obj = PyObject_GetAttrString((PyObject*)_wrapper, "Pause");
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
  Py_XDECREF(obj);
}
  Player::Pause();
}
void PythonQtShell_Player::Play()
{
if (_wrapper) {
  PyObject* obj = PyObject_GetAttrString((PyObject*)_wrapper, "Play");
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
  Py_XDECREF(obj);
}
  Player::Play();
}
void PythonQtShell_Player::PlayAt(int  i, Engine::TrackChangeFlags  change, bool  reshuffle)
{
if (_wrapper) {
  PyObject* obj = PyObject_GetAttrString((PyObject*)_wrapper, "PlayAt");
  PyErr_Clear();
  if (obj && !PythonQtSlotFunction_Check(obj)) {
    static const char* argumentList[] ={"" , "int" , "Engine::TrackChangeFlags" , "bool"};
    static const PythonQtMethodInfo* methodInfo = PythonQtMethodInfo::getCachedMethodInfoFromArgumentList(4, argumentList);
    void* args[4] = {NULL, (void*)&i, (void*)&change, (void*)&reshuffle};
    PyObject* result = PythonQtSignalTarget::call(obj, methodInfo, args, true);
    if (result) { Py_DECREF(result); } 
    Py_DECREF(obj);
    return;
  }
  Py_XDECREF(obj);
}
  Player::PlayAt(i, change, reshuffle);
}
void PythonQtShell_Player::PlayPause()
{
if (_wrapper) {
  PyObject* obj = PyObject_GetAttrString((PyObject*)_wrapper, "PlayPause");
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
  Py_XDECREF(obj);
}
  Player::PlayPause();
}
void PythonQtShell_Player::Previous()
{
if (_wrapper) {
  PyObject* obj = PyObject_GetAttrString((PyObject*)_wrapper, "Previous");
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
  Py_XDECREF(obj);
}
  Player::Previous();
}
void PythonQtShell_Player::RegisterUrlHandler(UrlHandler*  handler)
{
if (_wrapper) {
  PyObject* obj = PyObject_GetAttrString((PyObject*)_wrapper, "RegisterUrlHandler");
  PyErr_Clear();
  if (obj && !PythonQtSlotFunction_Check(obj)) {
    static const char* argumentList[] ={"" , "UrlHandler*"};
    static const PythonQtMethodInfo* methodInfo = PythonQtMethodInfo::getCachedMethodInfoFromArgumentList(2, argumentList);
    void* args[2] = {NULL, (void*)&handler};
    PyObject* result = PythonQtSignalTarget::call(obj, methodInfo, args, true);
    if (result) { Py_DECREF(result); } 
    Py_DECREF(obj);
    return;
  }
  Py_XDECREF(obj);
}
  Player::RegisterUrlHandler(handler);
}
void PythonQtShell_Player::ReloadSettings()
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
  Py_XDECREF(obj);
}
  Player::ReloadSettings();
}
void PythonQtShell_Player::SeekBackward()
{
if (_wrapper) {
  PyObject* obj = PyObject_GetAttrString((PyObject*)_wrapper, "SeekBackward");
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
  Py_XDECREF(obj);
}
  Player::SeekBackward();
}
void PythonQtShell_Player::SeekForward()
{
if (_wrapper) {
  PyObject* obj = PyObject_GetAttrString((PyObject*)_wrapper, "SeekForward");
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
  Py_XDECREF(obj);
}
  Player::SeekForward();
}
void PythonQtShell_Player::SeekTo(int  seconds)
{
if (_wrapper) {
  PyObject* obj = PyObject_GetAttrString((PyObject*)_wrapper, "SeekTo");
  PyErr_Clear();
  if (obj && !PythonQtSlotFunction_Check(obj)) {
    static const char* argumentList[] ={"" , "int"};
    static const PythonQtMethodInfo* methodInfo = PythonQtMethodInfo::getCachedMethodInfoFromArgumentList(2, argumentList);
    void* args[2] = {NULL, (void*)&seconds};
    PyObject* result = PythonQtSignalTarget::call(obj, methodInfo, args, true);
    if (result) { Py_DECREF(result); } 
    Py_DECREF(obj);
    return;
  }
  Py_XDECREF(obj);
}
  Player::SeekTo(seconds);
}
void PythonQtShell_Player::SetVolume(int  value)
{
if (_wrapper) {
  PyObject* obj = PyObject_GetAttrString((PyObject*)_wrapper, "SetVolume");
  PyErr_Clear();
  if (obj && !PythonQtSlotFunction_Check(obj)) {
    static const char* argumentList[] ={"" , "int"};
    static const PythonQtMethodInfo* methodInfo = PythonQtMethodInfo::getCachedMethodInfoFromArgumentList(2, argumentList);
    void* args[2] = {NULL, (void*)&value};
    PyObject* result = PythonQtSignalTarget::call(obj, methodInfo, args, true);
    if (result) { Py_DECREF(result); } 
    Py_DECREF(obj);
    return;
  }
  Py_XDECREF(obj);
}
  Player::SetVolume(value);
}
void PythonQtShell_Player::ShowOSD()
{
if (_wrapper) {
  PyObject* obj = PyObject_GetAttrString((PyObject*)_wrapper, "ShowOSD");
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
  Py_XDECREF(obj);
}
  Player::ShowOSD();
}
void PythonQtShell_Player::Stop()
{
if (_wrapper) {
  PyObject* obj = PyObject_GetAttrString((PyObject*)_wrapper, "Stop");
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
  Py_XDECREF(obj);
}
  Player::Stop();
}
void PythonQtShell_Player::UnregisterUrlHandler(UrlHandler*  handler)
{
if (_wrapper) {
  PyObject* obj = PyObject_GetAttrString((PyObject*)_wrapper, "UnregisterUrlHandler");
  PyErr_Clear();
  if (obj && !PythonQtSlotFunction_Check(obj)) {
    static const char* argumentList[] ={"" , "UrlHandler*"};
    static const PythonQtMethodInfo* methodInfo = PythonQtMethodInfo::getCachedMethodInfoFromArgumentList(2, argumentList);
    void* args[2] = {NULL, (void*)&handler};
    PyObject* result = PythonQtSignalTarget::call(obj, methodInfo, args, true);
    if (result) { Py_DECREF(result); } 
    Py_DECREF(obj);
    return;
  }
  Py_XDECREF(obj);
}
  Player::UnregisterUrlHandler(handler);
}
void PythonQtShell_Player::VolumeDown()
{
if (_wrapper) {
  PyObject* obj = PyObject_GetAttrString((PyObject*)_wrapper, "VolumeDown");
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
  Py_XDECREF(obj);
}
  Player::VolumeDown();
}
void PythonQtShell_Player::VolumeUp()
{
if (_wrapper) {
  PyObject* obj = PyObject_GetAttrString((PyObject*)_wrapper, "VolumeUp");
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
  Py_XDECREF(obj);
}
  Player::VolumeUp();
}
void PythonQtShell_Player::childEvent(QChildEvent*  arg__1)
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
  Py_XDECREF(obj);
}
  Player::childEvent(arg__1);
}
void PythonQtShell_Player::customEvent(QEvent*  arg__1)
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
  Py_XDECREF(obj);
}
  Player::customEvent(arg__1);
}
Engine::Base*  PythonQtShell_Player::engine() const
{
if (_wrapper) {
  PyObject* obj = PyObject_GetAttrString((PyObject*)_wrapper, "engine");
  PyErr_Clear();
  if (obj && !PythonQtSlotFunction_Check(obj)) {
    static const char* argumentList[] ={"Engine::Base*"};
    static const PythonQtMethodInfo* methodInfo = PythonQtMethodInfo::getCachedMethodInfoFromArgumentList(1, argumentList);
      Engine::Base* returnValue = 0;
    void* args[1] = {NULL};
    PyObject* result = PythonQtSignalTarget::call(obj, methodInfo, args, true);
    if (result) {
      args[0] = PythonQtConv::ConvertPythonToQt(methodInfo->parameters().at(0), result, false, NULL, &returnValue);
      if (args[0]!=&returnValue) {
        if (args[0]==NULL) {
          PythonQt::priv()->handleVirtualOverloadReturnError("engine", methodInfo, result);
        } else {
          returnValue = *((Engine::Base**)args[0]);
        }
      }
    }
    if (result) { Py_DECREF(result); } 
    Py_DECREF(obj);
    return returnValue;
  }
  Py_XDECREF(obj);
}
  return Player::engine();
}
bool  PythonQtShell_Player::event(QEvent*  arg__1)
{
if (_wrapper) {
  PyObject* obj = PyObject_GetAttrString((PyObject*)_wrapper, "event");
  PyErr_Clear();
  if (obj && !PythonQtSlotFunction_Check(obj)) {
    static const char* argumentList[] ={"bool" , "QEvent*"};
    static const PythonQtMethodInfo* methodInfo = PythonQtMethodInfo::getCachedMethodInfoFromArgumentList(2, argumentList);
      bool returnValue = 0;
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
  Py_XDECREF(obj);
}
  return Player::event(arg__1);
}
bool  PythonQtShell_Player::eventFilter(QObject*  arg__1, QEvent*  arg__2)
{
if (_wrapper) {
  PyObject* obj = PyObject_GetAttrString((PyObject*)_wrapper, "eventFilter");
  PyErr_Clear();
  if (obj && !PythonQtSlotFunction_Check(obj)) {
    static const char* argumentList[] ={"bool" , "QObject*" , "QEvent*"};
    static const PythonQtMethodInfo* methodInfo = PythonQtMethodInfo::getCachedMethodInfoFromArgumentList(3, argumentList);
      bool returnValue = 0;
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
  Py_XDECREF(obj);
}
  return Player::eventFilter(arg__1, arg__2);
}
PlaylistManagerInterface*  PythonQtShell_Player::playlists() const
{
if (_wrapper) {
  PyObject* obj = PyObject_GetAttrString((PyObject*)_wrapper, "playlists");
  PyErr_Clear();
  if (obj && !PythonQtSlotFunction_Check(obj)) {
    static const char* argumentList[] ={"PlaylistManagerInterface*"};
    static const PythonQtMethodInfo* methodInfo = PythonQtMethodInfo::getCachedMethodInfoFromArgumentList(1, argumentList);
      PlaylistManagerInterface* returnValue = 0;
    void* args[1] = {NULL};
    PyObject* result = PythonQtSignalTarget::call(obj, methodInfo, args, true);
    if (result) {
      args[0] = PythonQtConv::ConvertPythonToQt(methodInfo->parameters().at(0), result, false, NULL, &returnValue);
      if (args[0]!=&returnValue) {
        if (args[0]==NULL) {
          PythonQt::priv()->handleVirtualOverloadReturnError("playlists", methodInfo, result);
        } else {
          returnValue = *((PlaylistManagerInterface**)args[0]);
        }
      }
    }
    if (result) { Py_DECREF(result); } 
    Py_DECREF(obj);
    return returnValue;
  }
  Py_XDECREF(obj);
}
  return Player::playlists();
}
void PythonQtShell_Player::timerEvent(QTimerEvent*  arg__1)
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
  Py_XDECREF(obj);
}
  Player::timerEvent(arg__1);
}
Player* PythonQtWrapper_Player::new_Player(PlaylistManagerInterface*  playlists, QObject*  parent)
{ 
return new PythonQtShell_Player(playlists, parent); }

PlaylistItemPtr  PythonQtWrapper_Player::GetCurrentItem(Player* theWrappedObject) const
{
  return ( ((PythonQtPublicPromoter_Player*)theWrappedObject)->promoted_GetCurrentItem());
}

PlaylistItemPtr  PythonQtWrapper_Player::GetItemAt(Player* theWrappedObject, int  pos) const
{
  return ( ((PythonQtPublicPromoter_Player*)theWrappedObject)->promoted_GetItemAt(pos));
}

Engine::State  PythonQtWrapper_Player::GetState(Player* theWrappedObject) const
{
  return ( ((PythonQtPublicPromoter_Player*)theWrappedObject)->promoted_GetState());
}

int  PythonQtWrapper_Player::GetVolume(Player* theWrappedObject) const
{
  return ( ((PythonQtPublicPromoter_Player*)theWrappedObject)->promoted_GetVolume());
}

void PythonQtWrapper_Player::Init(Player* theWrappedObject)
{
  ( theWrappedObject->Init());
}

void PythonQtWrapper_Player::RegisterUrlHandler(Player* theWrappedObject, UrlHandler*  handler)
{
  ( ((PythonQtPublicPromoter_Player*)theWrappedObject)->promoted_RegisterUrlHandler(handler));
}

void PythonQtWrapper_Player::UnregisterUrlHandler(Player* theWrappedObject, UrlHandler*  handler)
{
  ( ((PythonQtPublicPromoter_Player*)theWrappedObject)->promoted_UnregisterUrlHandler(handler));
}

Engine::Base*  PythonQtWrapper_Player::engine(Player* theWrappedObject) const
{
  return ( ((PythonQtPublicPromoter_Player*)theWrappedObject)->promoted_engine());
}

PlaylistManagerInterface*  PythonQtWrapper_Player::playlists(Player* theWrappedObject) const
{
  return ( ((PythonQtPublicPromoter_Player*)theWrappedObject)->promoted_playlists());
}



void PythonQtShell_PlayerInterface::CurrentMetadataChanged(const Song&  metadata)
{
if (_wrapper) {
  PyObject* obj = PyObject_GetAttrString((PyObject*)_wrapper, "CurrentMetadataChanged");
  PyErr_Clear();
  if (obj && !PythonQtSlotFunction_Check(obj)) {
    static const char* argumentList[] ={"" , "const Song&"};
    static const PythonQtMethodInfo* methodInfo = PythonQtMethodInfo::getCachedMethodInfoFromArgumentList(2, argumentList);
    void* args[2] = {NULL, (void*)&metadata};
    PyObject* result = PythonQtSignalTarget::call(obj, methodInfo, args, true);
    if (result) { Py_DECREF(result); } 
    Py_DECREF(obj);
    return;
  }
  Py_XDECREF(obj);
}
  
}
PlaylistItemPtr  PythonQtShell_PlayerInterface::GetCurrentItem() const
{
if (_wrapper) {
  PyObject* obj = PyObject_GetAttrString((PyObject*)_wrapper, "GetCurrentItem");
  PyErr_Clear();
  if (obj && !PythonQtSlotFunction_Check(obj)) {
    static const char* argumentList[] ={"PlaylistItemPtr"};
    static const PythonQtMethodInfo* methodInfo = PythonQtMethodInfo::getCachedMethodInfoFromArgumentList(1, argumentList);
      PlaylistItemPtr returnValue;
    void* args[1] = {NULL};
    PyObject* result = PythonQtSignalTarget::call(obj, methodInfo, args, true);
    if (result) {
      args[0] = PythonQtConv::ConvertPythonToQt(methodInfo->parameters().at(0), result, false, NULL, &returnValue);
      if (args[0]!=&returnValue) {
        if (args[0]==NULL) {
          PythonQt::priv()->handleVirtualOverloadReturnError("GetCurrentItem", methodInfo, result);
        } else {
          returnValue = *((PlaylistItemPtr*)args[0]);
        }
      }
    }
    if (result) { Py_DECREF(result); } 
    Py_DECREF(obj);
    return returnValue;
  }
  Py_XDECREF(obj);
}
  return PlaylistItemPtr();
}
PlaylistItemPtr  PythonQtShell_PlayerInterface::GetItemAt(int  pos) const
{
if (_wrapper) {
  PyObject* obj = PyObject_GetAttrString((PyObject*)_wrapper, "GetItemAt");
  PyErr_Clear();
  if (obj && !PythonQtSlotFunction_Check(obj)) {
    static const char* argumentList[] ={"PlaylistItemPtr" , "int"};
    static const PythonQtMethodInfo* methodInfo = PythonQtMethodInfo::getCachedMethodInfoFromArgumentList(2, argumentList);
      PlaylistItemPtr returnValue;
    void* args[2] = {NULL, (void*)&pos};
    PyObject* result = PythonQtSignalTarget::call(obj, methodInfo, args, true);
    if (result) {
      args[0] = PythonQtConv::ConvertPythonToQt(methodInfo->parameters().at(0), result, false, NULL, &returnValue);
      if (args[0]!=&returnValue) {
        if (args[0]==NULL) {
          PythonQt::priv()->handleVirtualOverloadReturnError("GetItemAt", methodInfo, result);
        } else {
          returnValue = *((PlaylistItemPtr*)args[0]);
        }
      }
    }
    if (result) { Py_DECREF(result); } 
    Py_DECREF(obj);
    return returnValue;
  }
  Py_XDECREF(obj);
}
  return PlaylistItemPtr();
}
Engine::State  PythonQtShell_PlayerInterface::GetState() const
{
if (_wrapper) {
  PyObject* obj = PyObject_GetAttrString((PyObject*)_wrapper, "GetState");
  PyErr_Clear();
  if (obj && !PythonQtSlotFunction_Check(obj)) {
    static const char* argumentList[] ={"Engine::State"};
    static const PythonQtMethodInfo* methodInfo = PythonQtMethodInfo::getCachedMethodInfoFromArgumentList(1, argumentList);
      Engine::State returnValue = Engine::State(0);
    void* args[1] = {NULL};
    PyObject* result = PythonQtSignalTarget::call(obj, methodInfo, args, true);
    if (result) {
      args[0] = PythonQtConv::ConvertPythonToQt(methodInfo->parameters().at(0), result, false, NULL, &returnValue);
      if (args[0]!=&returnValue) {
        if (args[0]==NULL) {
          PythonQt::priv()->handleVirtualOverloadReturnError("GetState", methodInfo, result);
        } else {
          returnValue = *((Engine::State*)args[0]);
        }
      }
    }
    if (result) { Py_DECREF(result); } 
    Py_DECREF(obj);
    return returnValue;
  }
  Py_XDECREF(obj);
}
  return Engine::State();
}
int  PythonQtShell_PlayerInterface::GetVolume() const
{
if (_wrapper) {
  PyObject* obj = PyObject_GetAttrString((PyObject*)_wrapper, "GetVolume");
  PyErr_Clear();
  if (obj && !PythonQtSlotFunction_Check(obj)) {
    static const char* argumentList[] ={"int"};
    static const PythonQtMethodInfo* methodInfo = PythonQtMethodInfo::getCachedMethodInfoFromArgumentList(1, argumentList);
      int returnValue = 0;
    void* args[1] = {NULL};
    PyObject* result = PythonQtSignalTarget::call(obj, methodInfo, args, true);
    if (result) {
      args[0] = PythonQtConv::ConvertPythonToQt(methodInfo->parameters().at(0), result, false, NULL, &returnValue);
      if (args[0]!=&returnValue) {
        if (args[0]==NULL) {
          PythonQt::priv()->handleVirtualOverloadReturnError("GetVolume", methodInfo, result);
        } else {
          returnValue = *((int*)args[0]);
        }
      }
    }
    if (result) { Py_DECREF(result); } 
    Py_DECREF(obj);
    return returnValue;
  }
  Py_XDECREF(obj);
}
  return int();
}
void PythonQtShell_PlayerInterface::Mute()
{
if (_wrapper) {
  PyObject* obj = PyObject_GetAttrString((PyObject*)_wrapper, "Mute");
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
  Py_XDECREF(obj);
}
  
}
void PythonQtShell_PlayerInterface::Next()
{
if (_wrapper) {
  PyObject* obj = PyObject_GetAttrString((PyObject*)_wrapper, "Next");
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
  Py_XDECREF(obj);
}
  
}
void PythonQtShell_PlayerInterface::Pause()
{
if (_wrapper) {
  PyObject* obj = PyObject_GetAttrString((PyObject*)_wrapper, "Pause");
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
  Py_XDECREF(obj);
}
  
}
void PythonQtShell_PlayerInterface::Play()
{
if (_wrapper) {
  PyObject* obj = PyObject_GetAttrString((PyObject*)_wrapper, "Play");
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
  Py_XDECREF(obj);
}
  
}
void PythonQtShell_PlayerInterface::PlayAt(int  i, Engine::TrackChangeFlags  change, bool  reshuffle)
{
if (_wrapper) {
  PyObject* obj = PyObject_GetAttrString((PyObject*)_wrapper, "PlayAt");
  PyErr_Clear();
  if (obj && !PythonQtSlotFunction_Check(obj)) {
    static const char* argumentList[] ={"" , "int" , "Engine::TrackChangeFlags" , "bool"};
    static const PythonQtMethodInfo* methodInfo = PythonQtMethodInfo::getCachedMethodInfoFromArgumentList(4, argumentList);
    void* args[4] = {NULL, (void*)&i, (void*)&change, (void*)&reshuffle};
    PyObject* result = PythonQtSignalTarget::call(obj, methodInfo, args, true);
    if (result) { Py_DECREF(result); } 
    Py_DECREF(obj);
    return;
  }
  Py_XDECREF(obj);
}
  
}
void PythonQtShell_PlayerInterface::PlayPause()
{
if (_wrapper) {
  PyObject* obj = PyObject_GetAttrString((PyObject*)_wrapper, "PlayPause");
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
  Py_XDECREF(obj);
}
  
}
void PythonQtShell_PlayerInterface::Previous()
{
if (_wrapper) {
  PyObject* obj = PyObject_GetAttrString((PyObject*)_wrapper, "Previous");
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
  Py_XDECREF(obj);
}
  
}
void PythonQtShell_PlayerInterface::RegisterUrlHandler(UrlHandler*  handler)
{
if (_wrapper) {
  PyObject* obj = PyObject_GetAttrString((PyObject*)_wrapper, "RegisterUrlHandler");
  PyErr_Clear();
  if (obj && !PythonQtSlotFunction_Check(obj)) {
    static const char* argumentList[] ={"" , "UrlHandler*"};
    static const PythonQtMethodInfo* methodInfo = PythonQtMethodInfo::getCachedMethodInfoFromArgumentList(2, argumentList);
    void* args[2] = {NULL, (void*)&handler};
    PyObject* result = PythonQtSignalTarget::call(obj, methodInfo, args, true);
    if (result) { Py_DECREF(result); } 
    Py_DECREF(obj);
    return;
  }
  Py_XDECREF(obj);
}
  
}
void PythonQtShell_PlayerInterface::ReloadSettings()
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
  Py_XDECREF(obj);
}
  
}
void PythonQtShell_PlayerInterface::SeekBackward()
{
if (_wrapper) {
  PyObject* obj = PyObject_GetAttrString((PyObject*)_wrapper, "SeekBackward");
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
  Py_XDECREF(obj);
}
  
}
void PythonQtShell_PlayerInterface::SeekForward()
{
if (_wrapper) {
  PyObject* obj = PyObject_GetAttrString((PyObject*)_wrapper, "SeekForward");
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
  Py_XDECREF(obj);
}
  
}
void PythonQtShell_PlayerInterface::SeekTo(int  seconds)
{
if (_wrapper) {
  PyObject* obj = PyObject_GetAttrString((PyObject*)_wrapper, "SeekTo");
  PyErr_Clear();
  if (obj && !PythonQtSlotFunction_Check(obj)) {
    static const char* argumentList[] ={"" , "int"};
    static const PythonQtMethodInfo* methodInfo = PythonQtMethodInfo::getCachedMethodInfoFromArgumentList(2, argumentList);
    void* args[2] = {NULL, (void*)&seconds};
    PyObject* result = PythonQtSignalTarget::call(obj, methodInfo, args, true);
    if (result) { Py_DECREF(result); } 
    Py_DECREF(obj);
    return;
  }
  Py_XDECREF(obj);
}
  
}
void PythonQtShell_PlayerInterface::SetVolume(int  value)
{
if (_wrapper) {
  PyObject* obj = PyObject_GetAttrString((PyObject*)_wrapper, "SetVolume");
  PyErr_Clear();
  if (obj && !PythonQtSlotFunction_Check(obj)) {
    static const char* argumentList[] ={"" , "int"};
    static const PythonQtMethodInfo* methodInfo = PythonQtMethodInfo::getCachedMethodInfoFromArgumentList(2, argumentList);
    void* args[2] = {NULL, (void*)&value};
    PyObject* result = PythonQtSignalTarget::call(obj, methodInfo, args, true);
    if (result) { Py_DECREF(result); } 
    Py_DECREF(obj);
    return;
  }
  Py_XDECREF(obj);
}
  
}
void PythonQtShell_PlayerInterface::ShowOSD()
{
if (_wrapper) {
  PyObject* obj = PyObject_GetAttrString((PyObject*)_wrapper, "ShowOSD");
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
  Py_XDECREF(obj);
}
  
}
void PythonQtShell_PlayerInterface::Stop()
{
if (_wrapper) {
  PyObject* obj = PyObject_GetAttrString((PyObject*)_wrapper, "Stop");
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
  Py_XDECREF(obj);
}
  
}
void PythonQtShell_PlayerInterface::UnregisterUrlHandler(UrlHandler*  handler)
{
if (_wrapper) {
  PyObject* obj = PyObject_GetAttrString((PyObject*)_wrapper, "UnregisterUrlHandler");
  PyErr_Clear();
  if (obj && !PythonQtSlotFunction_Check(obj)) {
    static const char* argumentList[] ={"" , "UrlHandler*"};
    static const PythonQtMethodInfo* methodInfo = PythonQtMethodInfo::getCachedMethodInfoFromArgumentList(2, argumentList);
    void* args[2] = {NULL, (void*)&handler};
    PyObject* result = PythonQtSignalTarget::call(obj, methodInfo, args, true);
    if (result) { Py_DECREF(result); } 
    Py_DECREF(obj);
    return;
  }
  Py_XDECREF(obj);
}
  
}
void PythonQtShell_PlayerInterface::VolumeDown()
{
if (_wrapper) {
  PyObject* obj = PyObject_GetAttrString((PyObject*)_wrapper, "VolumeDown");
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
  Py_XDECREF(obj);
}
  
}
void PythonQtShell_PlayerInterface::VolumeUp()
{
if (_wrapper) {
  PyObject* obj = PyObject_GetAttrString((PyObject*)_wrapper, "VolumeUp");
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
  Py_XDECREF(obj);
}
  
}
void PythonQtShell_PlayerInterface::childEvent(QChildEvent*  arg__1)
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
  Py_XDECREF(obj);
}
  PlayerInterface::childEvent(arg__1);
}
void PythonQtShell_PlayerInterface::customEvent(QEvent*  arg__1)
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
  Py_XDECREF(obj);
}
  PlayerInterface::customEvent(arg__1);
}
Engine::Base*  PythonQtShell_PlayerInterface::engine() const
{
if (_wrapper) {
  PyObject* obj = PyObject_GetAttrString((PyObject*)_wrapper, "engine");
  PyErr_Clear();
  if (obj && !PythonQtSlotFunction_Check(obj)) {
    static const char* argumentList[] ={"Engine::Base*"};
    static const PythonQtMethodInfo* methodInfo = PythonQtMethodInfo::getCachedMethodInfoFromArgumentList(1, argumentList);
      Engine::Base* returnValue = 0;
    void* args[1] = {NULL};
    PyObject* result = PythonQtSignalTarget::call(obj, methodInfo, args, true);
    if (result) {
      args[0] = PythonQtConv::ConvertPythonToQt(methodInfo->parameters().at(0), result, false, NULL, &returnValue);
      if (args[0]!=&returnValue) {
        if (args[0]==NULL) {
          PythonQt::priv()->handleVirtualOverloadReturnError("engine", methodInfo, result);
        } else {
          returnValue = *((Engine::Base**)args[0]);
        }
      }
    }
    if (result) { Py_DECREF(result); } 
    Py_DECREF(obj);
    return returnValue;
  }
  Py_XDECREF(obj);
}
  return 0;
}
bool  PythonQtShell_PlayerInterface::event(QEvent*  arg__1)
{
if (_wrapper) {
  PyObject* obj = PyObject_GetAttrString((PyObject*)_wrapper, "event");
  PyErr_Clear();
  if (obj && !PythonQtSlotFunction_Check(obj)) {
    static const char* argumentList[] ={"bool" , "QEvent*"};
    static const PythonQtMethodInfo* methodInfo = PythonQtMethodInfo::getCachedMethodInfoFromArgumentList(2, argumentList);
      bool returnValue = 0;
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
  Py_XDECREF(obj);
}
  return PlayerInterface::event(arg__1);
}
bool  PythonQtShell_PlayerInterface::eventFilter(QObject*  arg__1, QEvent*  arg__2)
{
if (_wrapper) {
  PyObject* obj = PyObject_GetAttrString((PyObject*)_wrapper, "eventFilter");
  PyErr_Clear();
  if (obj && !PythonQtSlotFunction_Check(obj)) {
    static const char* argumentList[] ={"bool" , "QObject*" , "QEvent*"};
    static const PythonQtMethodInfo* methodInfo = PythonQtMethodInfo::getCachedMethodInfoFromArgumentList(3, argumentList);
      bool returnValue = 0;
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
  Py_XDECREF(obj);
}
  return PlayerInterface::eventFilter(arg__1, arg__2);
}
PlaylistManagerInterface*  PythonQtShell_PlayerInterface::playlists() const
{
if (_wrapper) {
  PyObject* obj = PyObject_GetAttrString((PyObject*)_wrapper, "playlists");
  PyErr_Clear();
  if (obj && !PythonQtSlotFunction_Check(obj)) {
    static const char* argumentList[] ={"PlaylistManagerInterface*"};
    static const PythonQtMethodInfo* methodInfo = PythonQtMethodInfo::getCachedMethodInfoFromArgumentList(1, argumentList);
      PlaylistManagerInterface* returnValue = 0;
    void* args[1] = {NULL};
    PyObject* result = PythonQtSignalTarget::call(obj, methodInfo, args, true);
    if (result) {
      args[0] = PythonQtConv::ConvertPythonToQt(methodInfo->parameters().at(0), result, false, NULL, &returnValue);
      if (args[0]!=&returnValue) {
        if (args[0]==NULL) {
          PythonQt::priv()->handleVirtualOverloadReturnError("playlists", methodInfo, result);
        } else {
          returnValue = *((PlaylistManagerInterface**)args[0]);
        }
      }
    }
    if (result) { Py_DECREF(result); } 
    Py_DECREF(obj);
    return returnValue;
  }
  Py_XDECREF(obj);
}
  return 0;
}
void PythonQtShell_PlayerInterface::timerEvent(QTimerEvent*  arg__1)
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
  Py_XDECREF(obj);
}
  PlayerInterface::timerEvent(arg__1);
}
PlayerInterface* PythonQtWrapper_PlayerInterface::new_PlayerInterface(QObject*  parent)
{ 
return new PythonQtShell_PlayerInterface(parent); }



QModelIndex  PythonQtShell_Playlist::buddy(const QModelIndex&  index) const
{
if (_wrapper) {
  PyObject* obj = PyObject_GetAttrString((PyObject*)_wrapper, "buddy");
  PyErr_Clear();
  if (obj && !PythonQtSlotFunction_Check(obj)) {
    static const char* argumentList[] ={"QModelIndex" , "const QModelIndex&"};
    static const PythonQtMethodInfo* methodInfo = PythonQtMethodInfo::getCachedMethodInfoFromArgumentList(2, argumentList);
      QModelIndex returnValue;
    void* args[2] = {NULL, (void*)&index};
    PyObject* result = PythonQtSignalTarget::call(obj, methodInfo, args, true);
    if (result) {
      args[0] = PythonQtConv::ConvertPythonToQt(methodInfo->parameters().at(0), result, false, NULL, &returnValue);
      if (args[0]!=&returnValue) {
        if (args[0]==NULL) {
          PythonQt::priv()->handleVirtualOverloadReturnError("buddy", methodInfo, result);
        } else {
          returnValue = *((QModelIndex*)args[0]);
        }
      }
    }
    if (result) { Py_DECREF(result); } 
    Py_DECREF(obj);
    return returnValue;
  }
  Py_XDECREF(obj);
}
  return Playlist::buddy(index);
}
bool  PythonQtShell_Playlist::canFetchMore(const QModelIndex&  parent) const
{
if (_wrapper) {
  PyObject* obj = PyObject_GetAttrString((PyObject*)_wrapper, "canFetchMore");
  PyErr_Clear();
  if (obj && !PythonQtSlotFunction_Check(obj)) {
    static const char* argumentList[] ={"bool" , "const QModelIndex&"};
    static const PythonQtMethodInfo* methodInfo = PythonQtMethodInfo::getCachedMethodInfoFromArgumentList(2, argumentList);
      bool returnValue = 0;
    void* args[2] = {NULL, (void*)&parent};
    PyObject* result = PythonQtSignalTarget::call(obj, methodInfo, args, true);
    if (result) {
      args[0] = PythonQtConv::ConvertPythonToQt(methodInfo->parameters().at(0), result, false, NULL, &returnValue);
      if (args[0]!=&returnValue) {
        if (args[0]==NULL) {
          PythonQt::priv()->handleVirtualOverloadReturnError("canFetchMore", methodInfo, result);
        } else {
          returnValue = *((bool*)args[0]);
        }
      }
    }
    if (result) { Py_DECREF(result); } 
    Py_DECREF(obj);
    return returnValue;
  }
  Py_XDECREF(obj);
}
  return Playlist::canFetchMore(parent);
}
void PythonQtShell_Playlist::childEvent(QChildEvent*  arg__1)
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
  Py_XDECREF(obj);
}
  Playlist::childEvent(arg__1);
}
int  PythonQtShell_Playlist::columnCount(const QModelIndex&  arg__1) const
{
if (_wrapper) {
  PyObject* obj = PyObject_GetAttrString((PyObject*)_wrapper, "columnCount");
  PyErr_Clear();
  if (obj && !PythonQtSlotFunction_Check(obj)) {
    static const char* argumentList[] ={"int" , "const QModelIndex&"};
    static const PythonQtMethodInfo* methodInfo = PythonQtMethodInfo::getCachedMethodInfoFromArgumentList(2, argumentList);
      int returnValue = 0;
    void* args[2] = {NULL, (void*)&arg__1};
    PyObject* result = PythonQtSignalTarget::call(obj, methodInfo, args, true);
    if (result) {
      args[0] = PythonQtConv::ConvertPythonToQt(methodInfo->parameters().at(0), result, false, NULL, &returnValue);
      if (args[0]!=&returnValue) {
        if (args[0]==NULL) {
          PythonQt::priv()->handleVirtualOverloadReturnError("columnCount", methodInfo, result);
        } else {
          returnValue = *((int*)args[0]);
        }
      }
    }
    if (result) { Py_DECREF(result); } 
    Py_DECREF(obj);
    return returnValue;
  }
  Py_XDECREF(obj);
}
  return Playlist::columnCount(arg__1);
}
void PythonQtShell_Playlist::customEvent(QEvent*  arg__1)
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
  Py_XDECREF(obj);
}
  Playlist::customEvent(arg__1);
}
QVariant  PythonQtShell_Playlist::data(const QModelIndex&  index, int  role) const
{
if (_wrapper) {
  PyObject* obj = PyObject_GetAttrString((PyObject*)_wrapper, "data");
  PyErr_Clear();
  if (obj && !PythonQtSlotFunction_Check(obj)) {
    static const char* argumentList[] ={"QVariant" , "const QModelIndex&" , "int"};
    static const PythonQtMethodInfo* methodInfo = PythonQtMethodInfo::getCachedMethodInfoFromArgumentList(3, argumentList);
      QVariant returnValue;
    void* args[3] = {NULL, (void*)&index, (void*)&role};
    PyObject* result = PythonQtSignalTarget::call(obj, methodInfo, args, true);
    if (result) {
      args[0] = PythonQtConv::ConvertPythonToQt(methodInfo->parameters().at(0), result, false, NULL, &returnValue);
      if (args[0]!=&returnValue) {
        if (args[0]==NULL) {
          PythonQt::priv()->handleVirtualOverloadReturnError("data", methodInfo, result);
        } else {
          returnValue = *((QVariant*)args[0]);
        }
      }
    }
    if (result) { Py_DECREF(result); } 
    Py_DECREF(obj);
    return returnValue;
  }
  Py_XDECREF(obj);
}
  return Playlist::data(index, role);
}
bool  PythonQtShell_Playlist::dropMimeData(const QMimeData*  data, Qt::DropAction  action, int  row, int  column, const QModelIndex&  parent)
{
if (_wrapper) {
  PyObject* obj = PyObject_GetAttrString((PyObject*)_wrapper, "dropMimeData");
  PyErr_Clear();
  if (obj && !PythonQtSlotFunction_Check(obj)) {
    static const char* argumentList[] ={"bool" , "const QMimeData*" , "Qt::DropAction" , "int" , "int" , "const QModelIndex&"};
    static const PythonQtMethodInfo* methodInfo = PythonQtMethodInfo::getCachedMethodInfoFromArgumentList(6, argumentList);
      bool returnValue = 0;
    void* args[6] = {NULL, (void*)&data, (void*)&action, (void*)&row, (void*)&column, (void*)&parent};
    PyObject* result = PythonQtSignalTarget::call(obj, methodInfo, args, true);
    if (result) {
      args[0] = PythonQtConv::ConvertPythonToQt(methodInfo->parameters().at(0), result, false, NULL, &returnValue);
      if (args[0]!=&returnValue) {
        if (args[0]==NULL) {
          PythonQt::priv()->handleVirtualOverloadReturnError("dropMimeData", methodInfo, result);
        } else {
          returnValue = *((bool*)args[0]);
        }
      }
    }
    if (result) { Py_DECREF(result); } 
    Py_DECREF(obj);
    return returnValue;
  }
  Py_XDECREF(obj);
}
  return Playlist::dropMimeData(data, action, row, column, parent);
}
bool  PythonQtShell_Playlist::event(QEvent*  arg__1)
{
if (_wrapper) {
  PyObject* obj = PyObject_GetAttrString((PyObject*)_wrapper, "event");
  PyErr_Clear();
  if (obj && !PythonQtSlotFunction_Check(obj)) {
    static const char* argumentList[] ={"bool" , "QEvent*"};
    static const PythonQtMethodInfo* methodInfo = PythonQtMethodInfo::getCachedMethodInfoFromArgumentList(2, argumentList);
      bool returnValue = 0;
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
  Py_XDECREF(obj);
}
  return Playlist::event(arg__1);
}
bool  PythonQtShell_Playlist::eventFilter(QObject*  arg__1, QEvent*  arg__2)
{
if (_wrapper) {
  PyObject* obj = PyObject_GetAttrString((PyObject*)_wrapper, "eventFilter");
  PyErr_Clear();
  if (obj && !PythonQtSlotFunction_Check(obj)) {
    static const char* argumentList[] ={"bool" , "QObject*" , "QEvent*"};
    static const PythonQtMethodInfo* methodInfo = PythonQtMethodInfo::getCachedMethodInfoFromArgumentList(3, argumentList);
      bool returnValue = 0;
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
  Py_XDECREF(obj);
}
  return Playlist::eventFilter(arg__1, arg__2);
}
void PythonQtShell_Playlist::fetchMore(const QModelIndex&  parent)
{
if (_wrapper) {
  PyObject* obj = PyObject_GetAttrString((PyObject*)_wrapper, "fetchMore");
  PyErr_Clear();
  if (obj && !PythonQtSlotFunction_Check(obj)) {
    static const char* argumentList[] ={"" , "const QModelIndex&"};
    static const PythonQtMethodInfo* methodInfo = PythonQtMethodInfo::getCachedMethodInfoFromArgumentList(2, argumentList);
    void* args[2] = {NULL, (void*)&parent};
    PyObject* result = PythonQtSignalTarget::call(obj, methodInfo, args, true);
    if (result) { Py_DECREF(result); } 
    Py_DECREF(obj);
    return;
  }
  Py_XDECREF(obj);
}
  Playlist::fetchMore(parent);
}
Qt::ItemFlags  PythonQtShell_Playlist::flags(const QModelIndex&  index) const
{
if (_wrapper) {
  PyObject* obj = PyObject_GetAttrString((PyObject*)_wrapper, "flags");
  PyErr_Clear();
  if (obj && !PythonQtSlotFunction_Check(obj)) {
    static const char* argumentList[] ={"Qt::ItemFlags" , "const QModelIndex&"};
    static const PythonQtMethodInfo* methodInfo = PythonQtMethodInfo::getCachedMethodInfoFromArgumentList(2, argumentList);
      Qt::ItemFlags returnValue;
    void* args[2] = {NULL, (void*)&index};
    PyObject* result = PythonQtSignalTarget::call(obj, methodInfo, args, true);
    if (result) {
      args[0] = PythonQtConv::ConvertPythonToQt(methodInfo->parameters().at(0), result, false, NULL, &returnValue);
      if (args[0]!=&returnValue) {
        if (args[0]==NULL) {
          PythonQt::priv()->handleVirtualOverloadReturnError("flags", methodInfo, result);
        } else {
          returnValue = *((Qt::ItemFlags*)args[0]);
        }
      }
    }
    if (result) { Py_DECREF(result); } 
    Py_DECREF(obj);
    return returnValue;
  }
  Py_XDECREF(obj);
}
  return Playlist::flags(index);
}
QVariant  PythonQtShell_Playlist::headerData(int  section, Qt::Orientation  orientation, int  role) const
{
if (_wrapper) {
  PyObject* obj = PyObject_GetAttrString((PyObject*)_wrapper, "headerData");
  PyErr_Clear();
  if (obj && !PythonQtSlotFunction_Check(obj)) {
    static const char* argumentList[] ={"QVariant" , "int" , "Qt::Orientation" , "int"};
    static const PythonQtMethodInfo* methodInfo = PythonQtMethodInfo::getCachedMethodInfoFromArgumentList(4, argumentList);
      QVariant returnValue;
    void* args[4] = {NULL, (void*)&section, (void*)&orientation, (void*)&role};
    PyObject* result = PythonQtSignalTarget::call(obj, methodInfo, args, true);
    if (result) {
      args[0] = PythonQtConv::ConvertPythonToQt(methodInfo->parameters().at(0), result, false, NULL, &returnValue);
      if (args[0]!=&returnValue) {
        if (args[0]==NULL) {
          PythonQt::priv()->handleVirtualOverloadReturnError("headerData", methodInfo, result);
        } else {
          returnValue = *((QVariant*)args[0]);
        }
      }
    }
    if (result) { Py_DECREF(result); } 
    Py_DECREF(obj);
    return returnValue;
  }
  Py_XDECREF(obj);
}
  return Playlist::headerData(section, orientation, role);
}
QModelIndex  PythonQtShell_Playlist::index(int  row, int  column, const QModelIndex&  parent) const
{
if (_wrapper) {
  PyObject* obj = PyObject_GetAttrString((PyObject*)_wrapper, "index");
  PyErr_Clear();
  if (obj && !PythonQtSlotFunction_Check(obj)) {
    static const char* argumentList[] ={"QModelIndex" , "int" , "int" , "const QModelIndex&"};
    static const PythonQtMethodInfo* methodInfo = PythonQtMethodInfo::getCachedMethodInfoFromArgumentList(4, argumentList);
      QModelIndex returnValue;
    void* args[4] = {NULL, (void*)&row, (void*)&column, (void*)&parent};
    PyObject* result = PythonQtSignalTarget::call(obj, methodInfo, args, true);
    if (result) {
      args[0] = PythonQtConv::ConvertPythonToQt(methodInfo->parameters().at(0), result, false, NULL, &returnValue);
      if (args[0]!=&returnValue) {
        if (args[0]==NULL) {
          PythonQt::priv()->handleVirtualOverloadReturnError("index", methodInfo, result);
        } else {
          returnValue = *((QModelIndex*)args[0]);
        }
      }
    }
    if (result) { Py_DECREF(result); } 
    Py_DECREF(obj);
    return returnValue;
  }
  Py_XDECREF(obj);
}
  return Playlist::index(row, column, parent);
}
bool  PythonQtShell_Playlist::insertColumns(int  column, int  count, const QModelIndex&  parent)
{
if (_wrapper) {
  PyObject* obj = PyObject_GetAttrString((PyObject*)_wrapper, "insertColumns");
  PyErr_Clear();
  if (obj && !PythonQtSlotFunction_Check(obj)) {
    static const char* argumentList[] ={"bool" , "int" , "int" , "const QModelIndex&"};
    static const PythonQtMethodInfo* methodInfo = PythonQtMethodInfo::getCachedMethodInfoFromArgumentList(4, argumentList);
      bool returnValue = 0;
    void* args[4] = {NULL, (void*)&column, (void*)&count, (void*)&parent};
    PyObject* result = PythonQtSignalTarget::call(obj, methodInfo, args, true);
    if (result) {
      args[0] = PythonQtConv::ConvertPythonToQt(methodInfo->parameters().at(0), result, false, NULL, &returnValue);
      if (args[0]!=&returnValue) {
        if (args[0]==NULL) {
          PythonQt::priv()->handleVirtualOverloadReturnError("insertColumns", methodInfo, result);
        } else {
          returnValue = *((bool*)args[0]);
        }
      }
    }
    if (result) { Py_DECREF(result); } 
    Py_DECREF(obj);
    return returnValue;
  }
  Py_XDECREF(obj);
}
  return Playlist::insertColumns(column, count, parent);
}
bool  PythonQtShell_Playlist::insertRows(int  row, int  count, const QModelIndex&  parent)
{
if (_wrapper) {
  PyObject* obj = PyObject_GetAttrString((PyObject*)_wrapper, "insertRows");
  PyErr_Clear();
  if (obj && !PythonQtSlotFunction_Check(obj)) {
    static const char* argumentList[] ={"bool" , "int" , "int" , "const QModelIndex&"};
    static const PythonQtMethodInfo* methodInfo = PythonQtMethodInfo::getCachedMethodInfoFromArgumentList(4, argumentList);
      bool returnValue = 0;
    void* args[4] = {NULL, (void*)&row, (void*)&count, (void*)&parent};
    PyObject* result = PythonQtSignalTarget::call(obj, methodInfo, args, true);
    if (result) {
      args[0] = PythonQtConv::ConvertPythonToQt(methodInfo->parameters().at(0), result, false, NULL, &returnValue);
      if (args[0]!=&returnValue) {
        if (args[0]==NULL) {
          PythonQt::priv()->handleVirtualOverloadReturnError("insertRows", methodInfo, result);
        } else {
          returnValue = *((bool*)args[0]);
        }
      }
    }
    if (result) { Py_DECREF(result); } 
    Py_DECREF(obj);
    return returnValue;
  }
  Py_XDECREF(obj);
}
  return Playlist::insertRows(row, count, parent);
}
QMap<int , QVariant >  PythonQtShell_Playlist::itemData(const QModelIndex&  index) const
{
if (_wrapper) {
  PyObject* obj = PyObject_GetAttrString((PyObject*)_wrapper, "itemData");
  PyErr_Clear();
  if (obj && !PythonQtSlotFunction_Check(obj)) {
    static const char* argumentList[] ={"QMap<int , QVariant >" , "const QModelIndex&"};
    static const PythonQtMethodInfo* methodInfo = PythonQtMethodInfo::getCachedMethodInfoFromArgumentList(2, argumentList);
      QMap<int , QVariant > returnValue;
    void* args[2] = {NULL, (void*)&index};
    PyObject* result = PythonQtSignalTarget::call(obj, methodInfo, args, true);
    if (result) {
      args[0] = PythonQtConv::ConvertPythonToQt(methodInfo->parameters().at(0), result, false, NULL, &returnValue);
      if (args[0]!=&returnValue) {
        if (args[0]==NULL) {
          PythonQt::priv()->handleVirtualOverloadReturnError("itemData", methodInfo, result);
        } else {
          returnValue = *((QMap<int , QVariant >*)args[0]);
        }
      }
    }
    if (result) { Py_DECREF(result); } 
    Py_DECREF(obj);
    return returnValue;
  }
  Py_XDECREF(obj);
}
  return Playlist::itemData(index);
}
QList<QModelIndex >  PythonQtShell_Playlist::match(const QModelIndex&  start, int  role, const QVariant&  value, int  hits, Qt::MatchFlags  flags) const
{
if (_wrapper) {
  PyObject* obj = PyObject_GetAttrString((PyObject*)_wrapper, "match");
  PyErr_Clear();
  if (obj && !PythonQtSlotFunction_Check(obj)) {
    static const char* argumentList[] ={"QList<QModelIndex >" , "const QModelIndex&" , "int" , "const QVariant&" , "int" , "Qt::MatchFlags"};
    static const PythonQtMethodInfo* methodInfo = PythonQtMethodInfo::getCachedMethodInfoFromArgumentList(6, argumentList);
      QList<QModelIndex > returnValue;
    void* args[6] = {NULL, (void*)&start, (void*)&role, (void*)&value, (void*)&hits, (void*)&flags};
    PyObject* result = PythonQtSignalTarget::call(obj, methodInfo, args, true);
    if (result) {
      args[0] = PythonQtConv::ConvertPythonToQt(methodInfo->parameters().at(0), result, false, NULL, &returnValue);
      if (args[0]!=&returnValue) {
        if (args[0]==NULL) {
          PythonQt::priv()->handleVirtualOverloadReturnError("match", methodInfo, result);
        } else {
          returnValue = *((QList<QModelIndex >*)args[0]);
        }
      }
    }
    if (result) { Py_DECREF(result); } 
    Py_DECREF(obj);
    return returnValue;
  }
  Py_XDECREF(obj);
}
  return Playlist::match(start, role, value, hits, flags);
}
QMimeData*  PythonQtShell_Playlist::mimeData(const QList<QModelIndex >&  indexes) const
{
if (_wrapper) {
  PyObject* obj = PyObject_GetAttrString((PyObject*)_wrapper, "mimeData");
  PyErr_Clear();
  if (obj && !PythonQtSlotFunction_Check(obj)) {
    static const char* argumentList[] ={"QMimeData*" , "const QList<QModelIndex >&"};
    static const PythonQtMethodInfo* methodInfo = PythonQtMethodInfo::getCachedMethodInfoFromArgumentList(2, argumentList);
      QMimeData* returnValue = 0;
    void* args[2] = {NULL, (void*)&indexes};
    PyObject* result = PythonQtSignalTarget::call(obj, methodInfo, args, true);
    if (result) {
      args[0] = PythonQtConv::ConvertPythonToQt(methodInfo->parameters().at(0), result, false, NULL, &returnValue);
      if (args[0]!=&returnValue) {
        if (args[0]==NULL) {
          PythonQt::priv()->handleVirtualOverloadReturnError("mimeData", methodInfo, result);
        } else {
          returnValue = *((QMimeData**)args[0]);
        }
      }
    }
    if (result) { Py_DECREF(result); } 
    Py_DECREF(obj);
    return returnValue;
  }
  Py_XDECREF(obj);
}
  return Playlist::mimeData(indexes);
}
QStringList  PythonQtShell_Playlist::mimeTypes() const
{
if (_wrapper) {
  PyObject* obj = PyObject_GetAttrString((PyObject*)_wrapper, "mimeTypes");
  PyErr_Clear();
  if (obj && !PythonQtSlotFunction_Check(obj)) {
    static const char* argumentList[] ={"QStringList"};
    static const PythonQtMethodInfo* methodInfo = PythonQtMethodInfo::getCachedMethodInfoFromArgumentList(1, argumentList);
      QStringList returnValue;
    void* args[1] = {NULL};
    PyObject* result = PythonQtSignalTarget::call(obj, methodInfo, args, true);
    if (result) {
      args[0] = PythonQtConv::ConvertPythonToQt(methodInfo->parameters().at(0), result, false, NULL, &returnValue);
      if (args[0]!=&returnValue) {
        if (args[0]==NULL) {
          PythonQt::priv()->handleVirtualOverloadReturnError("mimeTypes", methodInfo, result);
        } else {
          returnValue = *((QStringList*)args[0]);
        }
      }
    }
    if (result) { Py_DECREF(result); } 
    Py_DECREF(obj);
    return returnValue;
  }
  Py_XDECREF(obj);
}
  return Playlist::mimeTypes();
}
bool  PythonQtShell_Playlist::removeColumns(int  column, int  count, const QModelIndex&  parent)
{
if (_wrapper) {
  PyObject* obj = PyObject_GetAttrString((PyObject*)_wrapper, "removeColumns");
  PyErr_Clear();
  if (obj && !PythonQtSlotFunction_Check(obj)) {
    static const char* argumentList[] ={"bool" , "int" , "int" , "const QModelIndex&"};
    static const PythonQtMethodInfo* methodInfo = PythonQtMethodInfo::getCachedMethodInfoFromArgumentList(4, argumentList);
      bool returnValue = 0;
    void* args[4] = {NULL, (void*)&column, (void*)&count, (void*)&parent};
    PyObject* result = PythonQtSignalTarget::call(obj, methodInfo, args, true);
    if (result) {
      args[0] = PythonQtConv::ConvertPythonToQt(methodInfo->parameters().at(0), result, false, NULL, &returnValue);
      if (args[0]!=&returnValue) {
        if (args[0]==NULL) {
          PythonQt::priv()->handleVirtualOverloadReturnError("removeColumns", methodInfo, result);
        } else {
          returnValue = *((bool*)args[0]);
        }
      }
    }
    if (result) { Py_DECREF(result); } 
    Py_DECREF(obj);
    return returnValue;
  }
  Py_XDECREF(obj);
}
  return Playlist::removeColumns(column, count, parent);
}
bool  PythonQtShell_Playlist::removeRows(int  row, int  count, const QModelIndex&  parent)
{
if (_wrapper) {
  PyObject* obj = PyObject_GetAttrString((PyObject*)_wrapper, "removeRows");
  PyErr_Clear();
  if (obj && !PythonQtSlotFunction_Check(obj)) {
    static const char* argumentList[] ={"bool" , "int" , "int" , "const QModelIndex&"};
    static const PythonQtMethodInfo* methodInfo = PythonQtMethodInfo::getCachedMethodInfoFromArgumentList(4, argumentList);
      bool returnValue = 0;
    void* args[4] = {NULL, (void*)&row, (void*)&count, (void*)&parent};
    PyObject* result = PythonQtSignalTarget::call(obj, methodInfo, args, true);
    if (result) {
      args[0] = PythonQtConv::ConvertPythonToQt(methodInfo->parameters().at(0), result, false, NULL, &returnValue);
      if (args[0]!=&returnValue) {
        if (args[0]==NULL) {
          PythonQt::priv()->handleVirtualOverloadReturnError("removeRows", methodInfo, result);
        } else {
          returnValue = *((bool*)args[0]);
        }
      }
    }
    if (result) { Py_DECREF(result); } 
    Py_DECREF(obj);
    return returnValue;
  }
  Py_XDECREF(obj);
}
  return Playlist::removeRows(row, count, parent);
}
void PythonQtShell_Playlist::revert()
{
if (_wrapper) {
  PyObject* obj = PyObject_GetAttrString((PyObject*)_wrapper, "revert");
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
  Py_XDECREF(obj);
}
  Playlist::revert();
}
int  PythonQtShell_Playlist::rowCount(const QModelIndex&  arg__1) const
{
if (_wrapper) {
  PyObject* obj = PyObject_GetAttrString((PyObject*)_wrapper, "rowCount");
  PyErr_Clear();
  if (obj && !PythonQtSlotFunction_Check(obj)) {
    static const char* argumentList[] ={"int" , "const QModelIndex&"};
    static const PythonQtMethodInfo* methodInfo = PythonQtMethodInfo::getCachedMethodInfoFromArgumentList(2, argumentList);
      int returnValue = 0;
    void* args[2] = {NULL, (void*)&arg__1};
    PyObject* result = PythonQtSignalTarget::call(obj, methodInfo, args, true);
    if (result) {
      args[0] = PythonQtConv::ConvertPythonToQt(methodInfo->parameters().at(0), result, false, NULL, &returnValue);
      if (args[0]!=&returnValue) {
        if (args[0]==NULL) {
          PythonQt::priv()->handleVirtualOverloadReturnError("rowCount", methodInfo, result);
        } else {
          returnValue = *((int*)args[0]);
        }
      }
    }
    if (result) { Py_DECREF(result); } 
    Py_DECREF(obj);
    return returnValue;
  }
  Py_XDECREF(obj);
}
  return Playlist::rowCount(arg__1);
}
bool  PythonQtShell_Playlist::setData(const QModelIndex&  index, const QVariant&  value, int  role)
{
if (_wrapper) {
  PyObject* obj = PyObject_GetAttrString((PyObject*)_wrapper, "setData");
  PyErr_Clear();
  if (obj && !PythonQtSlotFunction_Check(obj)) {
    static const char* argumentList[] ={"bool" , "const QModelIndex&" , "const QVariant&" , "int"};
    static const PythonQtMethodInfo* methodInfo = PythonQtMethodInfo::getCachedMethodInfoFromArgumentList(4, argumentList);
      bool returnValue = 0;
    void* args[4] = {NULL, (void*)&index, (void*)&value, (void*)&role};
    PyObject* result = PythonQtSignalTarget::call(obj, methodInfo, args, true);
    if (result) {
      args[0] = PythonQtConv::ConvertPythonToQt(methodInfo->parameters().at(0), result, false, NULL, &returnValue);
      if (args[0]!=&returnValue) {
        if (args[0]==NULL) {
          PythonQt::priv()->handleVirtualOverloadReturnError("setData", methodInfo, result);
        } else {
          returnValue = *((bool*)args[0]);
        }
      }
    }
    if (result) { Py_DECREF(result); } 
    Py_DECREF(obj);
    return returnValue;
  }
  Py_XDECREF(obj);
}
  return Playlist::setData(index, value, role);
}
bool  PythonQtShell_Playlist::setHeaderData(int  section, Qt::Orientation  orientation, const QVariant&  value, int  role)
{
if (_wrapper) {
  PyObject* obj = PyObject_GetAttrString((PyObject*)_wrapper, "setHeaderData");
  PyErr_Clear();
  if (obj && !PythonQtSlotFunction_Check(obj)) {
    static const char* argumentList[] ={"bool" , "int" , "Qt::Orientation" , "const QVariant&" , "int"};
    static const PythonQtMethodInfo* methodInfo = PythonQtMethodInfo::getCachedMethodInfoFromArgumentList(5, argumentList);
      bool returnValue = 0;
    void* args[5] = {NULL, (void*)&section, (void*)&orientation, (void*)&value, (void*)&role};
    PyObject* result = PythonQtSignalTarget::call(obj, methodInfo, args, true);
    if (result) {
      args[0] = PythonQtConv::ConvertPythonToQt(methodInfo->parameters().at(0), result, false, NULL, &returnValue);
      if (args[0]!=&returnValue) {
        if (args[0]==NULL) {
          PythonQt::priv()->handleVirtualOverloadReturnError("setHeaderData", methodInfo, result);
        } else {
          returnValue = *((bool*)args[0]);
        }
      }
    }
    if (result) { Py_DECREF(result); } 
    Py_DECREF(obj);
    return returnValue;
  }
  Py_XDECREF(obj);
}
  return Playlist::setHeaderData(section, orientation, value, role);
}
bool  PythonQtShell_Playlist::setItemData(const QModelIndex&  index, const QMap<int , QVariant >&  roles)
{
if (_wrapper) {
  PyObject* obj = PyObject_GetAttrString((PyObject*)_wrapper, "setItemData");
  PyErr_Clear();
  if (obj && !PythonQtSlotFunction_Check(obj)) {
    static const char* argumentList[] ={"bool" , "const QModelIndex&" , "const QMap<int , QVariant >&"};
    static const PythonQtMethodInfo* methodInfo = PythonQtMethodInfo::getCachedMethodInfoFromArgumentList(3, argumentList);
      bool returnValue = 0;
    void* args[3] = {NULL, (void*)&index, (void*)&roles};
    PyObject* result = PythonQtSignalTarget::call(obj, methodInfo, args, true);
    if (result) {
      args[0] = PythonQtConv::ConvertPythonToQt(methodInfo->parameters().at(0), result, false, NULL, &returnValue);
      if (args[0]!=&returnValue) {
        if (args[0]==NULL) {
          PythonQt::priv()->handleVirtualOverloadReturnError("setItemData", methodInfo, result);
        } else {
          returnValue = *((bool*)args[0]);
        }
      }
    }
    if (result) { Py_DECREF(result); } 
    Py_DECREF(obj);
    return returnValue;
  }
  Py_XDECREF(obj);
}
  return Playlist::setItemData(index, roles);
}
void PythonQtShell_Playlist::sort(int  column, Qt::SortOrder  order)
{
if (_wrapper) {
  PyObject* obj = PyObject_GetAttrString((PyObject*)_wrapper, "sort");
  PyErr_Clear();
  if (obj && !PythonQtSlotFunction_Check(obj)) {
    static const char* argumentList[] ={"" , "int" , "Qt::SortOrder"};
    static const PythonQtMethodInfo* methodInfo = PythonQtMethodInfo::getCachedMethodInfoFromArgumentList(3, argumentList);
    void* args[3] = {NULL, (void*)&column, (void*)&order};
    PyObject* result = PythonQtSignalTarget::call(obj, methodInfo, args, true);
    if (result) { Py_DECREF(result); } 
    Py_DECREF(obj);
    return;
  }
  Py_XDECREF(obj);
}
  Playlist::sort(column, order);
}
QSize  PythonQtShell_Playlist::span(const QModelIndex&  index) const
{
if (_wrapper) {
  PyObject* obj = PyObject_GetAttrString((PyObject*)_wrapper, "span");
  PyErr_Clear();
  if (obj && !PythonQtSlotFunction_Check(obj)) {
    static const char* argumentList[] ={"QSize" , "const QModelIndex&"};
    static const PythonQtMethodInfo* methodInfo = PythonQtMethodInfo::getCachedMethodInfoFromArgumentList(2, argumentList);
      QSize returnValue;
    void* args[2] = {NULL, (void*)&index};
    PyObject* result = PythonQtSignalTarget::call(obj, methodInfo, args, true);
    if (result) {
      args[0] = PythonQtConv::ConvertPythonToQt(methodInfo->parameters().at(0), result, false, NULL, &returnValue);
      if (args[0]!=&returnValue) {
        if (args[0]==NULL) {
          PythonQt::priv()->handleVirtualOverloadReturnError("span", methodInfo, result);
        } else {
          returnValue = *((QSize*)args[0]);
        }
      }
    }
    if (result) { Py_DECREF(result); } 
    Py_DECREF(obj);
    return returnValue;
  }
  Py_XDECREF(obj);
}
  return Playlist::span(index);
}
bool  PythonQtShell_Playlist::submit()
{
if (_wrapper) {
  PyObject* obj = PyObject_GetAttrString((PyObject*)_wrapper, "submit");
  PyErr_Clear();
  if (obj && !PythonQtSlotFunction_Check(obj)) {
    static const char* argumentList[] ={"bool"};
    static const PythonQtMethodInfo* methodInfo = PythonQtMethodInfo::getCachedMethodInfoFromArgumentList(1, argumentList);
      bool returnValue = 0;
    void* args[1] = {NULL};
    PyObject* result = PythonQtSignalTarget::call(obj, methodInfo, args, true);
    if (result) {
      args[0] = PythonQtConv::ConvertPythonToQt(methodInfo->parameters().at(0), result, false, NULL, &returnValue);
      if (args[0]!=&returnValue) {
        if (args[0]==NULL) {
          PythonQt::priv()->handleVirtualOverloadReturnError("submit", methodInfo, result);
        } else {
          returnValue = *((bool*)args[0]);
        }
      }
    }
    if (result) { Py_DECREF(result); } 
    Py_DECREF(obj);
    return returnValue;
  }
  Py_XDECREF(obj);
}
  return Playlist::submit();
}
Qt::DropActions  PythonQtShell_Playlist::supportedDropActions() const
{
if (_wrapper) {
  PyObject* obj = PyObject_GetAttrString((PyObject*)_wrapper, "supportedDropActions");
  PyErr_Clear();
  if (obj && !PythonQtSlotFunction_Check(obj)) {
    static const char* argumentList[] ={"Qt::DropActions"};
    static const PythonQtMethodInfo* methodInfo = PythonQtMethodInfo::getCachedMethodInfoFromArgumentList(1, argumentList);
      Qt::DropActions returnValue;
    void* args[1] = {NULL};
    PyObject* result = PythonQtSignalTarget::call(obj, methodInfo, args, true);
    if (result) {
      args[0] = PythonQtConv::ConvertPythonToQt(methodInfo->parameters().at(0), result, false, NULL, &returnValue);
      if (args[0]!=&returnValue) {
        if (args[0]==NULL) {
          PythonQt::priv()->handleVirtualOverloadReturnError("supportedDropActions", methodInfo, result);
        } else {
          returnValue = *((Qt::DropActions*)args[0]);
        }
      }
    }
    if (result) { Py_DECREF(result); } 
    Py_DECREF(obj);
    return returnValue;
  }
  Py_XDECREF(obj);
}
  return Playlist::supportedDropActions();
}
void PythonQtShell_Playlist::timerEvent(QTimerEvent*  arg__1)
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
  Py_XDECREF(obj);
}
  Playlist::timerEvent(arg__1);
}
Playlist* PythonQtWrapper_Playlist::new_Playlist(PlaylistBackend*  backend, TaskManager*  task_manager, LibraryBackend*  library, int  id, const QString&  special_type, QObject*  parent)
{ 
return new PythonQtShell_Playlist(backend, task_manager, library, id, special_type, parent); }

void PythonQtWrapper_Playlist::AddSongInsertVetoListener(Playlist* theWrappedObject, SongInsertVetoListener*  listener)
{
  ( theWrappedObject->AddSongInsertVetoListener(listener));
}

bool  PythonQtWrapper_Playlist::ApplyValidityOnCurrentSong(Playlist* theWrappedObject, const QUrl&  url, bool  valid)
{
  return ( theWrappedObject->ApplyValidityOnCurrentSong(url, valid));
}

bool  PythonQtWrapper_Playlist::static_Playlist_CompareItems(int  column, Qt::SortOrder  order, PlaylistItemPtr  a, PlaylistItemPtr  b)
{
  return (Playlist::CompareItems(column, order, a, b));
}

QList<PlaylistItemPtr >  PythonQtWrapper_Playlist::GetAllItems(Playlist* theWrappedObject) const
{
  return ( theWrappedObject->GetAllItems());
}

QList<Song >  PythonQtWrapper_Playlist::GetAllSongs(Playlist* theWrappedObject) const
{
  return ( theWrappedObject->GetAllSongs());
}

quint64  PythonQtWrapper_Playlist::GetTotalLength(Playlist* theWrappedObject) const
{
  return ( theWrappedObject->GetTotalLength());
}

void PythonQtWrapper_Playlist::InsertItems(Playlist* theWrappedObject, const QList<PlaylistItemPtr >&  items, int  pos, bool  play_now, bool  enqueue)
{
  ( theWrappedObject->InsertItems(items, pos, play_now, enqueue));
}

void PythonQtWrapper_Playlist::InsertLibraryItems(Playlist* theWrappedObject, const QList<Song >&  items, int  pos, bool  play_now, bool  enqueue)
{
  ( theWrappedObject->InsertLibraryItems(items, pos, play_now, enqueue));
}

void PythonQtWrapper_Playlist::InsertSmartPlaylist(Playlist* theWrappedObject, smart_playlists::GeneratorPtr  gen, int  pos, bool  play_now, bool  enqueue)
{
  ( theWrappedObject->InsertSmartPlaylist(gen, pos, play_now, enqueue));
}

void PythonQtWrapper_Playlist::InsertSongs(Playlist* theWrappedObject, const QList<Song >&  items, int  pos, bool  play_now, bool  enqueue)
{
  ( theWrappedObject->InsertSongs(items, pos, play_now, enqueue));
}

void PythonQtWrapper_Playlist::InsertSongsOrLibraryItems(Playlist* theWrappedObject, const QList<Song >&  items, int  pos, bool  play_now, bool  enqueue)
{
  ( theWrappedObject->InsertSongsOrLibraryItems(items, pos, play_now, enqueue));
}

void PythonQtWrapper_Playlist::InsertUrls(Playlist* theWrappedObject, const QList<QUrl >&  urls, int  pos, bool  play_now, bool  enqueue)
{
  ( theWrappedObject->InsertUrls(urls, pos, play_now, enqueue));
}

void PythonQtWrapper_Playlist::InvalidateDeletedSongs(Playlist* theWrappedObject)
{
  ( theWrappedObject->InvalidateDeletedSongs());
}

void PythonQtWrapper_Playlist::RateSong(Playlist* theWrappedObject, const QModelIndex&  index, double  rating)
{
  ( theWrappedObject->RateSong(index, rating));
}

void PythonQtWrapper_Playlist::ReloadItems(Playlist* theWrappedObject, const QList<int >&  rows)
{
  ( theWrappedObject->ReloadItems(rows));
}

void PythonQtWrapper_Playlist::RemoveDeletedSongs(Playlist* theWrappedObject)
{
  ( theWrappedObject->RemoveDeletedSongs());
}

void PythonQtWrapper_Playlist::RemoveItemsWithoutUndo(Playlist* theWrappedObject, const QList<int >&  indices)
{
  ( theWrappedObject->RemoveItemsWithoutUndo(indices));
}

void PythonQtWrapper_Playlist::RemoveSongInsertVetoListener(Playlist* theWrappedObject, SongInsertVetoListener*  listener)
{
  ( theWrappedObject->RemoveSongInsertVetoListener(listener));
}

void PythonQtWrapper_Playlist::Restore(Playlist* theWrappedObject)
{
  ( theWrappedObject->Restore());
}

void PythonQtWrapper_Playlist::Save(Playlist* theWrappedObject) const
{
  ( theWrappedObject->Save());
}

void PythonQtWrapper_Playlist::StopAfter(Playlist* theWrappedObject, int  row)
{
  ( theWrappedObject->StopAfter(row));
}

void PythonQtWrapper_Playlist::UpdateItems(Playlist* theWrappedObject, const QList<Song >&  songs)
{
  ( theWrappedObject->UpdateItems(songs));
}

int  PythonQtWrapper_Playlist::columnCount(Playlist* theWrappedObject, const QModelIndex&  arg__1) const
{
  return ( ((PythonQtPublicPromoter_Playlist*)theWrappedObject)->promoted_columnCount(arg__1));
}

QMap<int , Qt::Alignment >  PythonQtWrapper_Playlist::column_alignments(Playlist* theWrappedObject) const
{
  return ( theWrappedObject->column_alignments());
}

bool  PythonQtWrapper_Playlist::static_Playlist_column_is_editable(Playlist::Column  column)
{
  return (Playlist::column_is_editable(column));
}

QString  PythonQtWrapper_Playlist::static_Playlist_column_name(Playlist::Column  column)
{
  return (Playlist::column_name(column));
}

const QModelIndex  PythonQtWrapper_Playlist::current_index(Playlist* theWrappedObject) const
{
  return ( theWrappedObject->current_index());
}

PlaylistItemPtr  PythonQtWrapper_Playlist::current_item(Playlist* theWrappedObject) const
{
  return ( theWrappedObject->current_item());
}

Song  PythonQtWrapper_Playlist::current_item_metadata(Playlist* theWrappedObject) const
{
  return ( theWrappedObject->current_item_metadata());
}

int  PythonQtWrapper_Playlist::current_row(Playlist* theWrappedObject) const
{
  return ( theWrappedObject->current_row());
}

QVariant  PythonQtWrapper_Playlist::data(Playlist* theWrappedObject, const QModelIndex&  index, int  role) const
{
  return ( ((PythonQtPublicPromoter_Playlist*)theWrappedObject)->promoted_data(index, role));
}

bool  PythonQtWrapper_Playlist::dropMimeData(Playlist* theWrappedObject, const QMimeData*  data, Qt::DropAction  action, int  row, int  column, const QModelIndex&  parent)
{
  return ( ((PythonQtPublicPromoter_Playlist*)theWrappedObject)->promoted_dropMimeData(data, action, row, column, parent));
}

Qt::ItemFlags  PythonQtWrapper_Playlist::flags(Playlist* theWrappedObject, const QModelIndex&  index) const
{
  return ( ((PythonQtPublicPromoter_Playlist*)theWrappedObject)->promoted_flags(index));
}

Playlist::LastFMStatus  PythonQtWrapper_Playlist::get_lastfm_status(Playlist* theWrappedObject) const
{
  return ( theWrappedObject->get_lastfm_status());
}

const bool  PythonQtWrapper_Playlist::has_item_at(Playlist* theWrappedObject, int  index) const
{
  return ( theWrappedObject->has_item_at(index));
}

bool  PythonQtWrapper_Playlist::have_incremented_playcount(Playlist* theWrappedObject) const
{
  return ( theWrappedObject->have_incremented_playcount());
}

QVariant  PythonQtWrapper_Playlist::headerData(Playlist* theWrappedObject, int  section, Qt::Orientation  orientation, int  role) const
{
  return ( ((PythonQtPublicPromoter_Playlist*)theWrappedObject)->promoted_headerData(section, orientation, role));
}

int  PythonQtWrapper_Playlist::id(Playlist* theWrappedObject) const
{
  return ( theWrappedObject->id());
}

bool  PythonQtWrapper_Playlist::is_dynamic(Playlist* theWrappedObject) const
{
  return ( theWrappedObject->is_dynamic());
}

const PlaylistItemPtr*  PythonQtWrapper_Playlist::item_at(Playlist* theWrappedObject, int  index) const
{
  return &( theWrappedObject->item_at(index));
}

int  PythonQtWrapper_Playlist::last_played_row(Playlist* theWrappedObject) const
{
  return ( theWrappedObject->last_played_row());
}

QList<PlaylistItemPtr >  PythonQtWrapper_Playlist::library_items_by_id(Playlist* theWrappedObject, int  id) const
{
  return ( theWrappedObject->library_items_by_id(id));
}

QMimeData*  PythonQtWrapper_Playlist::mimeData(Playlist* theWrappedObject, const QList<QModelIndex >&  indexes) const
{
  return ( ((PythonQtPublicPromoter_Playlist*)theWrappedObject)->promoted_mimeData(indexes));
}

QStringList  PythonQtWrapper_Playlist::mimeTypes(Playlist* theWrappedObject) const
{
  return ( ((PythonQtPublicPromoter_Playlist*)theWrappedObject)->promoted_mimeTypes());
}

int  PythonQtWrapper_Playlist::next_row(Playlist* theWrappedObject) const
{
  return ( theWrappedObject->next_row());
}

int  PythonQtWrapper_Playlist::previous_row(Playlist* theWrappedObject) const
{
  return ( theWrappedObject->previous_row());
}

QSortFilterProxyModel*  PythonQtWrapper_Playlist::proxy(Playlist* theWrappedObject) const
{
  return ( theWrappedObject->proxy());
}

bool  PythonQtWrapper_Playlist::removeRows(Playlist* theWrappedObject, int  row, int  count, const QModelIndex&  parent)
{
  return ( ((PythonQtPublicPromoter_Playlist*)theWrappedObject)->promoted_removeRows(row, count, parent));
}

int  PythonQtWrapper_Playlist::rowCount(Playlist* theWrappedObject, const QModelIndex&  arg__1) const
{
  return ( ((PythonQtPublicPromoter_Playlist*)theWrappedObject)->promoted_rowCount(arg__1));
}

qint64  PythonQtWrapper_Playlist::scrobble_point_nanosec(Playlist* theWrappedObject) const
{
  return ( theWrappedObject->scrobble_point_nanosec());
}

PlaylistSequence*  PythonQtWrapper_Playlist::sequence(Playlist* theWrappedObject) const
{
  return ( theWrappedObject->sequence());
}

bool  PythonQtWrapper_Playlist::setData(Playlist* theWrappedObject, const QModelIndex&  index, const QVariant&  value, int  role)
{
  return ( ((PythonQtPublicPromoter_Playlist*)theWrappedObject)->promoted_setData(index, value, role));
}

void PythonQtWrapper_Playlist::set_column_align_center(Playlist* theWrappedObject, int  column)
{
  ( theWrappedObject->set_column_align_center(column));
}

void PythonQtWrapper_Playlist::set_column_align_left(Playlist* theWrappedObject, int  column)
{
  ( theWrappedObject->set_column_align_left(column));
}

void PythonQtWrapper_Playlist::set_column_align_right(Playlist* theWrappedObject, int  column)
{
  ( theWrappedObject->set_column_align_right(column));
}

void PythonQtWrapper_Playlist::set_column_alignments(Playlist* theWrappedObject, const QMap<int , Qt::Alignment >&  column_alignments)
{
  ( theWrappedObject->set_column_alignments(column_alignments));
}

bool  PythonQtWrapper_Playlist::static_Playlist_set_column_value(Song&  song, Playlist::Column  column, const QVariant&  value)
{
  return (Playlist::set_column_value(song, column, value));
}

void PythonQtWrapper_Playlist::set_have_incremented_playcount(Playlist* theWrappedObject)
{
  ( theWrappedObject->set_have_incremented_playcount());
}

void PythonQtWrapper_Playlist::set_lastfm_status(Playlist* theWrappedObject, Playlist::LastFMStatus  status)
{
  ( theWrappedObject->set_lastfm_status(status));
}

void PythonQtWrapper_Playlist::set_sequence(Playlist* theWrappedObject, PlaylistSequence*  v)
{
  ( theWrappedObject->set_sequence(v));
}

void PythonQtWrapper_Playlist::set_special_type(Playlist* theWrappedObject, const QString&  v)
{
  ( theWrappedObject->set_special_type(v));
}

void PythonQtWrapper_Playlist::sort(Playlist* theWrappedObject, int  column, Qt::SortOrder  order)
{
  ( ((PythonQtPublicPromoter_Playlist*)theWrappedObject)->promoted_sort(column, order));
}

QString  PythonQtWrapper_Playlist::special_type(Playlist* theWrappedObject) const
{
  return ( theWrappedObject->special_type());
}

bool  PythonQtWrapper_Playlist::stop_after_current(Playlist* theWrappedObject) const
{
  return ( theWrappedObject->stop_after_current());
}

Qt::DropActions  PythonQtWrapper_Playlist::supportedDropActions(Playlist* theWrappedObject) const
{
  return ( ((PythonQtPublicPromoter_Playlist*)theWrappedObject)->promoted_supportedDropActions());
}

QUndoStack*  PythonQtWrapper_Playlist::undo_stack(Playlist* theWrappedObject) const
{
  return ( theWrappedObject->undo_stack());
}



void PythonQtShell_PlaylistBackend::childEvent(QChildEvent*  arg__1)
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
  Py_XDECREF(obj);
}
  PlaylistBackend::childEvent(arg__1);
}
void PythonQtShell_PlaylistBackend::customEvent(QEvent*  arg__1)
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
  Py_XDECREF(obj);
}
  PlaylistBackend::customEvent(arg__1);
}
bool  PythonQtShell_PlaylistBackend::event(QEvent*  arg__1)
{
if (_wrapper) {
  PyObject* obj = PyObject_GetAttrString((PyObject*)_wrapper, "event");
  PyErr_Clear();
  if (obj && !PythonQtSlotFunction_Check(obj)) {
    static const char* argumentList[] ={"bool" , "QEvent*"};
    static const PythonQtMethodInfo* methodInfo = PythonQtMethodInfo::getCachedMethodInfoFromArgumentList(2, argumentList);
      bool returnValue = 0;
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
  Py_XDECREF(obj);
}
  return PlaylistBackend::event(arg__1);
}
bool  PythonQtShell_PlaylistBackend::eventFilter(QObject*  arg__1, QEvent*  arg__2)
{
if (_wrapper) {
  PyObject* obj = PyObject_GetAttrString((PyObject*)_wrapper, "eventFilter");
  PyErr_Clear();
  if (obj && !PythonQtSlotFunction_Check(obj)) {
    static const char* argumentList[] ={"bool" , "QObject*" , "QEvent*"};
    static const PythonQtMethodInfo* methodInfo = PythonQtMethodInfo::getCachedMethodInfoFromArgumentList(3, argumentList);
      bool returnValue = 0;
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
  Py_XDECREF(obj);
}
  return PlaylistBackend::eventFilter(arg__1, arg__2);
}
void PythonQtShell_PlaylistBackend::timerEvent(QTimerEvent*  arg__1)
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
  Py_XDECREF(obj);
}
  PlaylistBackend::timerEvent(arg__1);
}
PlaylistBackend* PythonQtWrapper_PlaylistBackend::new_PlaylistBackend(QObject*  parent)
{ 
return new PythonQtShell_PlaylistBackend(parent); }

int  PythonQtWrapper_PlaylistBackend::CreatePlaylist(PlaylistBackend* theWrappedObject, const QString&  name, const QString&  special_type)
{
  return ( theWrappedObject->CreatePlaylist(name, special_type));
}

void PythonQtWrapper_PlaylistBackend::RemovePlaylist(PlaylistBackend* theWrappedObject, int  id)
{
  ( theWrappedObject->RemovePlaylist(id));
}

void PythonQtWrapper_PlaylistBackend::RenamePlaylist(PlaylistBackend* theWrappedObject, int  id, const QString&  new_name)
{
  ( theWrappedObject->RenamePlaylist(id, new_name));
}

void PythonQtWrapper_PlaylistBackend::SavePlaylistAsync(PlaylistBackend* theWrappedObject, int  playlist, const QList<PlaylistItemPtr >&  items, int  last_played, smart_playlists::GeneratorPtr  dynamic)
{
  ( theWrappedObject->SavePlaylistAsync(playlist, items, last_played, dynamic));
}

void PythonQtWrapper_PlaylistBackend::SetLibrary(PlaylistBackend* theWrappedObject, LibraryBackend*  library)
{
  ( theWrappedObject->SetLibrary(library));
}

void PythonQtWrapper_PlaylistBackend::SetPlaylistOrder(PlaylistBackend* theWrappedObject, const QList<int >&  ids)
{
  ( theWrappedObject->SetPlaylistOrder(ids));
}



void PythonQtShell_PlaylistContainer::actionEvent(QActionEvent*  arg__1)
{
if (_wrapper) {
  PyObject* obj = PyObject_GetAttrString((PyObject*)_wrapper, "actionEvent");
  PyErr_Clear();
  if (obj && !PythonQtSlotFunction_Check(obj)) {
    static const char* argumentList[] ={"" , "QActionEvent*"};
    static const PythonQtMethodInfo* methodInfo = PythonQtMethodInfo::getCachedMethodInfoFromArgumentList(2, argumentList);
    void* args[2] = {NULL, (void*)&arg__1};
    PyObject* result = PythonQtSignalTarget::call(obj, methodInfo, args, true);
    if (result) { Py_DECREF(result); } 
    Py_DECREF(obj);
    return;
  }
  Py_XDECREF(obj);
}
  PlaylistContainer::actionEvent(arg__1);
}
void PythonQtShell_PlaylistContainer::changeEvent(QEvent*  arg__1)
{
if (_wrapper) {
  PyObject* obj = PyObject_GetAttrString((PyObject*)_wrapper, "changeEvent");
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
  Py_XDECREF(obj);
}
  PlaylistContainer::changeEvent(arg__1);
}
void PythonQtShell_PlaylistContainer::childEvent(QChildEvent*  arg__1)
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
  Py_XDECREF(obj);
}
  PlaylistContainer::childEvent(arg__1);
}
void PythonQtShell_PlaylistContainer::closeEvent(QCloseEvent*  arg__1)
{
if (_wrapper) {
  PyObject* obj = PyObject_GetAttrString((PyObject*)_wrapper, "closeEvent");
  PyErr_Clear();
  if (obj && !PythonQtSlotFunction_Check(obj)) {
    static const char* argumentList[] ={"" , "QCloseEvent*"};
    static const PythonQtMethodInfo* methodInfo = PythonQtMethodInfo::getCachedMethodInfoFromArgumentList(2, argumentList);
    void* args[2] = {NULL, (void*)&arg__1};
    PyObject* result = PythonQtSignalTarget::call(obj, methodInfo, args, true);
    if (result) { Py_DECREF(result); } 
    Py_DECREF(obj);
    return;
  }
  Py_XDECREF(obj);
}
  PlaylistContainer::closeEvent(arg__1);
}
void PythonQtShell_PlaylistContainer::contextMenuEvent(QContextMenuEvent*  arg__1)
{
if (_wrapper) {
  PyObject* obj = PyObject_GetAttrString((PyObject*)_wrapper, "contextMenuEvent");
  PyErr_Clear();
  if (obj && !PythonQtSlotFunction_Check(obj)) {
    static const char* argumentList[] ={"" , "QContextMenuEvent*"};
    static const PythonQtMethodInfo* methodInfo = PythonQtMethodInfo::getCachedMethodInfoFromArgumentList(2, argumentList);
    void* args[2] = {NULL, (void*)&arg__1};
    PyObject* result = PythonQtSignalTarget::call(obj, methodInfo, args, true);
    if (result) { Py_DECREF(result); } 
    Py_DECREF(obj);
    return;
  }
  Py_XDECREF(obj);
}
  PlaylistContainer::contextMenuEvent(arg__1);
}
void PythonQtShell_PlaylistContainer::customEvent(QEvent*  arg__1)
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
  Py_XDECREF(obj);
}
  PlaylistContainer::customEvent(arg__1);
}
int  PythonQtShell_PlaylistContainer::devType() const
{
if (_wrapper) {
  PyObject* obj = PyObject_GetAttrString((PyObject*)_wrapper, "devType");
  PyErr_Clear();
  if (obj && !PythonQtSlotFunction_Check(obj)) {
    static const char* argumentList[] ={"int"};
    static const PythonQtMethodInfo* methodInfo = PythonQtMethodInfo::getCachedMethodInfoFromArgumentList(1, argumentList);
      int returnValue = 0;
    void* args[1] = {NULL};
    PyObject* result = PythonQtSignalTarget::call(obj, methodInfo, args, true);
    if (result) {
      args[0] = PythonQtConv::ConvertPythonToQt(methodInfo->parameters().at(0), result, false, NULL, &returnValue);
      if (args[0]!=&returnValue) {
        if (args[0]==NULL) {
          PythonQt::priv()->handleVirtualOverloadReturnError("devType", methodInfo, result);
        } else {
          returnValue = *((int*)args[0]);
        }
      }
    }
    if (result) { Py_DECREF(result); } 
    Py_DECREF(obj);
    return returnValue;
  }
  Py_XDECREF(obj);
}
  return PlaylistContainer::devType();
}
void PythonQtShell_PlaylistContainer::dragEnterEvent(QDragEnterEvent*  arg__1)
{
if (_wrapper) {
  PyObject* obj = PyObject_GetAttrString((PyObject*)_wrapper, "dragEnterEvent");
  PyErr_Clear();
  if (obj && !PythonQtSlotFunction_Check(obj)) {
    static const char* argumentList[] ={"" , "QDragEnterEvent*"};
    static const PythonQtMethodInfo* methodInfo = PythonQtMethodInfo::getCachedMethodInfoFromArgumentList(2, argumentList);
    void* args[2] = {NULL, (void*)&arg__1};
    PyObject* result = PythonQtSignalTarget::call(obj, methodInfo, args, true);
    if (result) { Py_DECREF(result); } 
    Py_DECREF(obj);
    return;
  }
  Py_XDECREF(obj);
}
  PlaylistContainer::dragEnterEvent(arg__1);
}
void PythonQtShell_PlaylistContainer::dragLeaveEvent(QDragLeaveEvent*  arg__1)
{
if (_wrapper) {
  PyObject* obj = PyObject_GetAttrString((PyObject*)_wrapper, "dragLeaveEvent");
  PyErr_Clear();
  if (obj && !PythonQtSlotFunction_Check(obj)) {
    static const char* argumentList[] ={"" , "QDragLeaveEvent*"};
    static const PythonQtMethodInfo* methodInfo = PythonQtMethodInfo::getCachedMethodInfoFromArgumentList(2, argumentList);
    void* args[2] = {NULL, (void*)&arg__1};
    PyObject* result = PythonQtSignalTarget::call(obj, methodInfo, args, true);
    if (result) { Py_DECREF(result); } 
    Py_DECREF(obj);
    return;
  }
  Py_XDECREF(obj);
}
  PlaylistContainer::dragLeaveEvent(arg__1);
}
void PythonQtShell_PlaylistContainer::dragMoveEvent(QDragMoveEvent*  arg__1)
{
if (_wrapper) {
  PyObject* obj = PyObject_GetAttrString((PyObject*)_wrapper, "dragMoveEvent");
  PyErr_Clear();
  if (obj && !PythonQtSlotFunction_Check(obj)) {
    static const char* argumentList[] ={"" , "QDragMoveEvent*"};
    static const PythonQtMethodInfo* methodInfo = PythonQtMethodInfo::getCachedMethodInfoFromArgumentList(2, argumentList);
    void* args[2] = {NULL, (void*)&arg__1};
    PyObject* result = PythonQtSignalTarget::call(obj, methodInfo, args, true);
    if (result) { Py_DECREF(result); } 
    Py_DECREF(obj);
    return;
  }
  Py_XDECREF(obj);
}
  PlaylistContainer::dragMoveEvent(arg__1);
}
void PythonQtShell_PlaylistContainer::dropEvent(QDropEvent*  arg__1)
{
if (_wrapper) {
  PyObject* obj = PyObject_GetAttrString((PyObject*)_wrapper, "dropEvent");
  PyErr_Clear();
  if (obj && !PythonQtSlotFunction_Check(obj)) {
    static const char* argumentList[] ={"" , "QDropEvent*"};
    static const PythonQtMethodInfo* methodInfo = PythonQtMethodInfo::getCachedMethodInfoFromArgumentList(2, argumentList);
    void* args[2] = {NULL, (void*)&arg__1};
    PyObject* result = PythonQtSignalTarget::call(obj, methodInfo, args, true);
    if (result) { Py_DECREF(result); } 
    Py_DECREF(obj);
    return;
  }
  Py_XDECREF(obj);
}
  PlaylistContainer::dropEvent(arg__1);
}
void PythonQtShell_PlaylistContainer::enterEvent(QEvent*  arg__1)
{
if (_wrapper) {
  PyObject* obj = PyObject_GetAttrString((PyObject*)_wrapper, "enterEvent");
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
  Py_XDECREF(obj);
}
  PlaylistContainer::enterEvent(arg__1);
}
bool  PythonQtShell_PlaylistContainer::event(QEvent*  arg__1)
{
if (_wrapper) {
  PyObject* obj = PyObject_GetAttrString((PyObject*)_wrapper, "event");
  PyErr_Clear();
  if (obj && !PythonQtSlotFunction_Check(obj)) {
    static const char* argumentList[] ={"bool" , "QEvent*"};
    static const PythonQtMethodInfo* methodInfo = PythonQtMethodInfo::getCachedMethodInfoFromArgumentList(2, argumentList);
      bool returnValue = 0;
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
  Py_XDECREF(obj);
}
  return PlaylistContainer::event(arg__1);
}
bool  PythonQtShell_PlaylistContainer::eventFilter(QObject*  arg__1, QEvent*  arg__2)
{
if (_wrapper) {
  PyObject* obj = PyObject_GetAttrString((PyObject*)_wrapper, "eventFilter");
  PyErr_Clear();
  if (obj && !PythonQtSlotFunction_Check(obj)) {
    static const char* argumentList[] ={"bool" , "QObject*" , "QEvent*"};
    static const PythonQtMethodInfo* methodInfo = PythonQtMethodInfo::getCachedMethodInfoFromArgumentList(3, argumentList);
      bool returnValue = 0;
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
  Py_XDECREF(obj);
}
  return PlaylistContainer::eventFilter(arg__1, arg__2);
}
void PythonQtShell_PlaylistContainer::focusInEvent(QFocusEvent*  arg__1)
{
if (_wrapper) {
  PyObject* obj = PyObject_GetAttrString((PyObject*)_wrapper, "focusInEvent");
  PyErr_Clear();
  if (obj && !PythonQtSlotFunction_Check(obj)) {
    static const char* argumentList[] ={"" , "QFocusEvent*"};
    static const PythonQtMethodInfo* methodInfo = PythonQtMethodInfo::getCachedMethodInfoFromArgumentList(2, argumentList);
    void* args[2] = {NULL, (void*)&arg__1};
    PyObject* result = PythonQtSignalTarget::call(obj, methodInfo, args, true);
    if (result) { Py_DECREF(result); } 
    Py_DECREF(obj);
    return;
  }
  Py_XDECREF(obj);
}
  PlaylistContainer::focusInEvent(arg__1);
}
bool  PythonQtShell_PlaylistContainer::focusNextPrevChild(bool  next)
{
if (_wrapper) {
  PyObject* obj = PyObject_GetAttrString((PyObject*)_wrapper, "focusNextPrevChild");
  PyErr_Clear();
  if (obj && !PythonQtSlotFunction_Check(obj)) {
    static const char* argumentList[] ={"bool" , "bool"};
    static const PythonQtMethodInfo* methodInfo = PythonQtMethodInfo::getCachedMethodInfoFromArgumentList(2, argumentList);
      bool returnValue = 0;
    void* args[2] = {NULL, (void*)&next};
    PyObject* result = PythonQtSignalTarget::call(obj, methodInfo, args, true);
    if (result) {
      args[0] = PythonQtConv::ConvertPythonToQt(methodInfo->parameters().at(0), result, false, NULL, &returnValue);
      if (args[0]!=&returnValue) {
        if (args[0]==NULL) {
          PythonQt::priv()->handleVirtualOverloadReturnError("focusNextPrevChild", methodInfo, result);
        } else {
          returnValue = *((bool*)args[0]);
        }
      }
    }
    if (result) { Py_DECREF(result); } 
    Py_DECREF(obj);
    return returnValue;
  }
  Py_XDECREF(obj);
}
  return PlaylistContainer::focusNextPrevChild(next);
}
void PythonQtShell_PlaylistContainer::focusOutEvent(QFocusEvent*  arg__1)
{
if (_wrapper) {
  PyObject* obj = PyObject_GetAttrString((PyObject*)_wrapper, "focusOutEvent");
  PyErr_Clear();
  if (obj && !PythonQtSlotFunction_Check(obj)) {
    static const char* argumentList[] ={"" , "QFocusEvent*"};
    static const PythonQtMethodInfo* methodInfo = PythonQtMethodInfo::getCachedMethodInfoFromArgumentList(2, argumentList);
    void* args[2] = {NULL, (void*)&arg__1};
    PyObject* result = PythonQtSignalTarget::call(obj, methodInfo, args, true);
    if (result) { Py_DECREF(result); } 
    Py_DECREF(obj);
    return;
  }
  Py_XDECREF(obj);
}
  PlaylistContainer::focusOutEvent(arg__1);
}
int  PythonQtShell_PlaylistContainer::heightForWidth(int  arg__1) const
{
if (_wrapper) {
  PyObject* obj = PyObject_GetAttrString((PyObject*)_wrapper, "heightForWidth");
  PyErr_Clear();
  if (obj && !PythonQtSlotFunction_Check(obj)) {
    static const char* argumentList[] ={"int" , "int"};
    static const PythonQtMethodInfo* methodInfo = PythonQtMethodInfo::getCachedMethodInfoFromArgumentList(2, argumentList);
      int returnValue = 0;
    void* args[2] = {NULL, (void*)&arg__1};
    PyObject* result = PythonQtSignalTarget::call(obj, methodInfo, args, true);
    if (result) {
      args[0] = PythonQtConv::ConvertPythonToQt(methodInfo->parameters().at(0), result, false, NULL, &returnValue);
      if (args[0]!=&returnValue) {
        if (args[0]==NULL) {
          PythonQt::priv()->handleVirtualOverloadReturnError("heightForWidth", methodInfo, result);
        } else {
          returnValue = *((int*)args[0]);
        }
      }
    }
    if (result) { Py_DECREF(result); } 
    Py_DECREF(obj);
    return returnValue;
  }
  Py_XDECREF(obj);
}
  return PlaylistContainer::heightForWidth(arg__1);
}
void PythonQtShell_PlaylistContainer::hideEvent(QHideEvent*  arg__1)
{
if (_wrapper) {
  PyObject* obj = PyObject_GetAttrString((PyObject*)_wrapper, "hideEvent");
  PyErr_Clear();
  if (obj && !PythonQtSlotFunction_Check(obj)) {
    static const char* argumentList[] ={"" , "QHideEvent*"};
    static const PythonQtMethodInfo* methodInfo = PythonQtMethodInfo::getCachedMethodInfoFromArgumentList(2, argumentList);
    void* args[2] = {NULL, (void*)&arg__1};
    PyObject* result = PythonQtSignalTarget::call(obj, methodInfo, args, true);
    if (result) { Py_DECREF(result); } 
    Py_DECREF(obj);
    return;
  }
  Py_XDECREF(obj);
}
  PlaylistContainer::hideEvent(arg__1);
}
void PythonQtShell_PlaylistContainer::inputMethodEvent(QInputMethodEvent*  arg__1)
{
if (_wrapper) {
  PyObject* obj = PyObject_GetAttrString((PyObject*)_wrapper, "inputMethodEvent");
  PyErr_Clear();
  if (obj && !PythonQtSlotFunction_Check(obj)) {
    static const char* argumentList[] ={"" , "QInputMethodEvent*"};
    static const PythonQtMethodInfo* methodInfo = PythonQtMethodInfo::getCachedMethodInfoFromArgumentList(2, argumentList);
    void* args[2] = {NULL, (void*)&arg__1};
    PyObject* result = PythonQtSignalTarget::call(obj, methodInfo, args, true);
    if (result) { Py_DECREF(result); } 
    Py_DECREF(obj);
    return;
  }
  Py_XDECREF(obj);
}
  PlaylistContainer::inputMethodEvent(arg__1);
}
QVariant  PythonQtShell_PlaylistContainer::inputMethodQuery(Qt::InputMethodQuery  arg__1) const
{
if (_wrapper) {
  PyObject* obj = PyObject_GetAttrString((PyObject*)_wrapper, "inputMethodQuery");
  PyErr_Clear();
  if (obj && !PythonQtSlotFunction_Check(obj)) {
    static const char* argumentList[] ={"QVariant" , "Qt::InputMethodQuery"};
    static const PythonQtMethodInfo* methodInfo = PythonQtMethodInfo::getCachedMethodInfoFromArgumentList(2, argumentList);
      QVariant returnValue;
    void* args[2] = {NULL, (void*)&arg__1};
    PyObject* result = PythonQtSignalTarget::call(obj, methodInfo, args, true);
    if (result) {
      args[0] = PythonQtConv::ConvertPythonToQt(methodInfo->parameters().at(0), result, false, NULL, &returnValue);
      if (args[0]!=&returnValue) {
        if (args[0]==NULL) {
          PythonQt::priv()->handleVirtualOverloadReturnError("inputMethodQuery", methodInfo, result);
        } else {
          returnValue = *((QVariant*)args[0]);
        }
      }
    }
    if (result) { Py_DECREF(result); } 
    Py_DECREF(obj);
    return returnValue;
  }
  Py_XDECREF(obj);
}
  return PlaylistContainer::inputMethodQuery(arg__1);
}
void PythonQtShell_PlaylistContainer::keyPressEvent(QKeyEvent*  arg__1)
{
if (_wrapper) {
  PyObject* obj = PyObject_GetAttrString((PyObject*)_wrapper, "keyPressEvent");
  PyErr_Clear();
  if (obj && !PythonQtSlotFunction_Check(obj)) {
    static const char* argumentList[] ={"" , "QKeyEvent*"};
    static const PythonQtMethodInfo* methodInfo = PythonQtMethodInfo::getCachedMethodInfoFromArgumentList(2, argumentList);
    void* args[2] = {NULL, (void*)&arg__1};
    PyObject* result = PythonQtSignalTarget::call(obj, methodInfo, args, true);
    if (result) { Py_DECREF(result); } 
    Py_DECREF(obj);
    return;
  }
  Py_XDECREF(obj);
}
  PlaylistContainer::keyPressEvent(arg__1);
}
void PythonQtShell_PlaylistContainer::keyReleaseEvent(QKeyEvent*  arg__1)
{
if (_wrapper) {
  PyObject* obj = PyObject_GetAttrString((PyObject*)_wrapper, "keyReleaseEvent");
  PyErr_Clear();
  if (obj && !PythonQtSlotFunction_Check(obj)) {
    static const char* argumentList[] ={"" , "QKeyEvent*"};
    static const PythonQtMethodInfo* methodInfo = PythonQtMethodInfo::getCachedMethodInfoFromArgumentList(2, argumentList);
    void* args[2] = {NULL, (void*)&arg__1};
    PyObject* result = PythonQtSignalTarget::call(obj, methodInfo, args, true);
    if (result) { Py_DECREF(result); } 
    Py_DECREF(obj);
    return;
  }
  Py_XDECREF(obj);
}
  PlaylistContainer::keyReleaseEvent(arg__1);
}
void PythonQtShell_PlaylistContainer::languageChange()
{
if (_wrapper) {
  PyObject* obj = PyObject_GetAttrString((PyObject*)_wrapper, "languageChange");
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
  Py_XDECREF(obj);
}
  PlaylistContainer::languageChange();
}
void PythonQtShell_PlaylistContainer::leaveEvent(QEvent*  arg__1)
{
if (_wrapper) {
  PyObject* obj = PyObject_GetAttrString((PyObject*)_wrapper, "leaveEvent");
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
  Py_XDECREF(obj);
}
  PlaylistContainer::leaveEvent(arg__1);
}
int  PythonQtShell_PlaylistContainer::metric(QPaintDevice::PaintDeviceMetric  arg__1) const
{
if (_wrapper) {
  PyObject* obj = PyObject_GetAttrString((PyObject*)_wrapper, "metric");
  PyErr_Clear();
  if (obj && !PythonQtSlotFunction_Check(obj)) {
    static const char* argumentList[] ={"int" , "QPaintDevice::PaintDeviceMetric"};
    static const PythonQtMethodInfo* methodInfo = PythonQtMethodInfo::getCachedMethodInfoFromArgumentList(2, argumentList);
      int returnValue = 0;
    void* args[2] = {NULL, (void*)&arg__1};
    PyObject* result = PythonQtSignalTarget::call(obj, methodInfo, args, true);
    if (result) {
      args[0] = PythonQtConv::ConvertPythonToQt(methodInfo->parameters().at(0), result, false, NULL, &returnValue);
      if (args[0]!=&returnValue) {
        if (args[0]==NULL) {
          PythonQt::priv()->handleVirtualOverloadReturnError("metric", methodInfo, result);
        } else {
          returnValue = *((int*)args[0]);
        }
      }
    }
    if (result) { Py_DECREF(result); } 
    Py_DECREF(obj);
    return returnValue;
  }
  Py_XDECREF(obj);
}
  return PlaylistContainer::metric(arg__1);
}
QSize  PythonQtShell_PlaylistContainer::minimumSizeHint() const
{
if (_wrapper) {
  PyObject* obj = PyObject_GetAttrString((PyObject*)_wrapper, "getMinimumSizeHint");
  PyErr_Clear();
  if (obj && !PythonQtSlotFunction_Check(obj)) {
    static const char* argumentList[] ={"QSize"};
    static const PythonQtMethodInfo* methodInfo = PythonQtMethodInfo::getCachedMethodInfoFromArgumentList(1, argumentList);
      QSize returnValue;
    void* args[1] = {NULL};
    PyObject* result = PythonQtSignalTarget::call(obj, methodInfo, args, true);
    if (result) {
      args[0] = PythonQtConv::ConvertPythonToQt(methodInfo->parameters().at(0), result, false, NULL, &returnValue);
      if (args[0]!=&returnValue) {
        if (args[0]==NULL) {
          PythonQt::priv()->handleVirtualOverloadReturnError("getMinimumSizeHint", methodInfo, result);
        } else {
          returnValue = *((QSize*)args[0]);
        }
      }
    }
    if (result) { Py_DECREF(result); } 
    Py_DECREF(obj);
    return returnValue;
  }
  Py_XDECREF(obj);
}
  return PlaylistContainer::minimumSizeHint();
}
void PythonQtShell_PlaylistContainer::mouseDoubleClickEvent(QMouseEvent*  arg__1)
{
if (_wrapper) {
  PyObject* obj = PyObject_GetAttrString((PyObject*)_wrapper, "mouseDoubleClickEvent");
  PyErr_Clear();
  if (obj && !PythonQtSlotFunction_Check(obj)) {
    static const char* argumentList[] ={"" , "QMouseEvent*"};
    static const PythonQtMethodInfo* methodInfo = PythonQtMethodInfo::getCachedMethodInfoFromArgumentList(2, argumentList);
    void* args[2] = {NULL, (void*)&arg__1};
    PyObject* result = PythonQtSignalTarget::call(obj, methodInfo, args, true);
    if (result) { Py_DECREF(result); } 
    Py_DECREF(obj);
    return;
  }
  Py_XDECREF(obj);
}
  PlaylistContainer::mouseDoubleClickEvent(arg__1);
}
void PythonQtShell_PlaylistContainer::mouseMoveEvent(QMouseEvent*  arg__1)
{
if (_wrapper) {
  PyObject* obj = PyObject_GetAttrString((PyObject*)_wrapper, "mouseMoveEvent");
  PyErr_Clear();
  if (obj && !PythonQtSlotFunction_Check(obj)) {
    static const char* argumentList[] ={"" , "QMouseEvent*"};
    static const PythonQtMethodInfo* methodInfo = PythonQtMethodInfo::getCachedMethodInfoFromArgumentList(2, argumentList);
    void* args[2] = {NULL, (void*)&arg__1};
    PyObject* result = PythonQtSignalTarget::call(obj, methodInfo, args, true);
    if (result) { Py_DECREF(result); } 
    Py_DECREF(obj);
    return;
  }
  Py_XDECREF(obj);
}
  PlaylistContainer::mouseMoveEvent(arg__1);
}
void PythonQtShell_PlaylistContainer::mousePressEvent(QMouseEvent*  arg__1)
{
if (_wrapper) {
  PyObject* obj = PyObject_GetAttrString((PyObject*)_wrapper, "mousePressEvent");
  PyErr_Clear();
  if (obj && !PythonQtSlotFunction_Check(obj)) {
    static const char* argumentList[] ={"" , "QMouseEvent*"};
    static const PythonQtMethodInfo* methodInfo = PythonQtMethodInfo::getCachedMethodInfoFromArgumentList(2, argumentList);
    void* args[2] = {NULL, (void*)&arg__1};
    PyObject* result = PythonQtSignalTarget::call(obj, methodInfo, args, true);
    if (result) { Py_DECREF(result); } 
    Py_DECREF(obj);
    return;
  }
  Py_XDECREF(obj);
}
  PlaylistContainer::mousePressEvent(arg__1);
}
void PythonQtShell_PlaylistContainer::mouseReleaseEvent(QMouseEvent*  arg__1)
{
if (_wrapper) {
  PyObject* obj = PyObject_GetAttrString((PyObject*)_wrapper, "mouseReleaseEvent");
  PyErr_Clear();
  if (obj && !PythonQtSlotFunction_Check(obj)) {
    static const char* argumentList[] ={"" , "QMouseEvent*"};
    static const PythonQtMethodInfo* methodInfo = PythonQtMethodInfo::getCachedMethodInfoFromArgumentList(2, argumentList);
    void* args[2] = {NULL, (void*)&arg__1};
    PyObject* result = PythonQtSignalTarget::call(obj, methodInfo, args, true);
    if (result) { Py_DECREF(result); } 
    Py_DECREF(obj);
    return;
  }
  Py_XDECREF(obj);
}
  PlaylistContainer::mouseReleaseEvent(arg__1);
}
void PythonQtShell_PlaylistContainer::moveEvent(QMoveEvent*  arg__1)
{
if (_wrapper) {
  PyObject* obj = PyObject_GetAttrString((PyObject*)_wrapper, "moveEvent");
  PyErr_Clear();
  if (obj && !PythonQtSlotFunction_Check(obj)) {
    static const char* argumentList[] ={"" , "QMoveEvent*"};
    static const PythonQtMethodInfo* methodInfo = PythonQtMethodInfo::getCachedMethodInfoFromArgumentList(2, argumentList);
    void* args[2] = {NULL, (void*)&arg__1};
    PyObject* result = PythonQtSignalTarget::call(obj, methodInfo, args, true);
    if (result) { Py_DECREF(result); } 
    Py_DECREF(obj);
    return;
  }
  Py_XDECREF(obj);
}
  PlaylistContainer::moveEvent(arg__1);
}
QPaintEngine*  PythonQtShell_PlaylistContainer::paintEngine() const
{
if (_wrapper) {
  PyObject* obj = PyObject_GetAttrString((PyObject*)_wrapper, "paintEngine");
  PyErr_Clear();
  if (obj && !PythonQtSlotFunction_Check(obj)) {
    static const char* argumentList[] ={"QPaintEngine*"};
    static const PythonQtMethodInfo* methodInfo = PythonQtMethodInfo::getCachedMethodInfoFromArgumentList(1, argumentList);
      QPaintEngine* returnValue = 0;
    void* args[1] = {NULL};
    PyObject* result = PythonQtSignalTarget::call(obj, methodInfo, args, true);
    if (result) {
      args[0] = PythonQtConv::ConvertPythonToQt(methodInfo->parameters().at(0), result, false, NULL, &returnValue);
      if (args[0]!=&returnValue) {
        if (args[0]==NULL) {
          PythonQt::priv()->handleVirtualOverloadReturnError("paintEngine", methodInfo, result);
        } else {
          returnValue = *((QPaintEngine**)args[0]);
        }
      }
    }
    if (result) { Py_DECREF(result); } 
    Py_DECREF(obj);
    return returnValue;
  }
  Py_XDECREF(obj);
}
  return PlaylistContainer::paintEngine();
}
void PythonQtShell_PlaylistContainer::paintEvent(QPaintEvent*  arg__1)
{
if (_wrapper) {
  PyObject* obj = PyObject_GetAttrString((PyObject*)_wrapper, "paintEvent");
  PyErr_Clear();
  if (obj && !PythonQtSlotFunction_Check(obj)) {
    static const char* argumentList[] ={"" , "QPaintEvent*"};
    static const PythonQtMethodInfo* methodInfo = PythonQtMethodInfo::getCachedMethodInfoFromArgumentList(2, argumentList);
    void* args[2] = {NULL, (void*)&arg__1};
    PyObject* result = PythonQtSignalTarget::call(obj, methodInfo, args, true);
    if (result) { Py_DECREF(result); } 
    Py_DECREF(obj);
    return;
  }
  Py_XDECREF(obj);
}
  PlaylistContainer::paintEvent(arg__1);
}
void PythonQtShell_PlaylistContainer::resizeEvent(QResizeEvent*  arg__1)
{
if (_wrapper) {
  PyObject* obj = PyObject_GetAttrString((PyObject*)_wrapper, "resizeEvent");
  PyErr_Clear();
  if (obj && !PythonQtSlotFunction_Check(obj)) {
    static const char* argumentList[] ={"" , "QResizeEvent*"};
    static const PythonQtMethodInfo* methodInfo = PythonQtMethodInfo::getCachedMethodInfoFromArgumentList(2, argumentList);
    void* args[2] = {NULL, (void*)&arg__1};
    PyObject* result = PythonQtSignalTarget::call(obj, methodInfo, args, true);
    if (result) { Py_DECREF(result); } 
    Py_DECREF(obj);
    return;
  }
  Py_XDECREF(obj);
}
  PlaylistContainer::resizeEvent(arg__1);
}
void PythonQtShell_PlaylistContainer::showEvent(QShowEvent*  arg__1)
{
if (_wrapper) {
  PyObject* obj = PyObject_GetAttrString((PyObject*)_wrapper, "showEvent");
  PyErr_Clear();
  if (obj && !PythonQtSlotFunction_Check(obj)) {
    static const char* argumentList[] ={"" , "QShowEvent*"};
    static const PythonQtMethodInfo* methodInfo = PythonQtMethodInfo::getCachedMethodInfoFromArgumentList(2, argumentList);
    void* args[2] = {NULL, (void*)&arg__1};
    PyObject* result = PythonQtSignalTarget::call(obj, methodInfo, args, true);
    if (result) { Py_DECREF(result); } 
    Py_DECREF(obj);
    return;
  }
  Py_XDECREF(obj);
}
  PlaylistContainer::showEvent(arg__1);
}
QSize  PythonQtShell_PlaylistContainer::sizeHint() const
{
if (_wrapper) {
  PyObject* obj = PyObject_GetAttrString((PyObject*)_wrapper, "getSizeHint");
  PyErr_Clear();
  if (obj && !PythonQtSlotFunction_Check(obj)) {
    static const char* argumentList[] ={"QSize"};
    static const PythonQtMethodInfo* methodInfo = PythonQtMethodInfo::getCachedMethodInfoFromArgumentList(1, argumentList);
      QSize returnValue;
    void* args[1] = {NULL};
    PyObject* result = PythonQtSignalTarget::call(obj, methodInfo, args, true);
    if (result) {
      args[0] = PythonQtConv::ConvertPythonToQt(methodInfo->parameters().at(0), result, false, NULL, &returnValue);
      if (args[0]!=&returnValue) {
        if (args[0]==NULL) {
          PythonQt::priv()->handleVirtualOverloadReturnError("getSizeHint", methodInfo, result);
        } else {
          returnValue = *((QSize*)args[0]);
        }
      }
    }
    if (result) { Py_DECREF(result); } 
    Py_DECREF(obj);
    return returnValue;
  }
  Py_XDECREF(obj);
}
  return PlaylistContainer::sizeHint();
}
void PythonQtShell_PlaylistContainer::tabletEvent(QTabletEvent*  arg__1)
{
if (_wrapper) {
  PyObject* obj = PyObject_GetAttrString((PyObject*)_wrapper, "tabletEvent");
  PyErr_Clear();
  if (obj && !PythonQtSlotFunction_Check(obj)) {
    static const char* argumentList[] ={"" , "QTabletEvent*"};
    static const PythonQtMethodInfo* methodInfo = PythonQtMethodInfo::getCachedMethodInfoFromArgumentList(2, argumentList);
    void* args[2] = {NULL, (void*)&arg__1};
    PyObject* result = PythonQtSignalTarget::call(obj, methodInfo, args, true);
    if (result) { Py_DECREF(result); } 
    Py_DECREF(obj);
    return;
  }
  Py_XDECREF(obj);
}
  PlaylistContainer::tabletEvent(arg__1);
}
void PythonQtShell_PlaylistContainer::timerEvent(QTimerEvent*  arg__1)
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
  Py_XDECREF(obj);
}
  PlaylistContainer::timerEvent(arg__1);
}
void PythonQtShell_PlaylistContainer::wheelEvent(QWheelEvent*  arg__1)
{
if (_wrapper) {
  PyObject* obj = PyObject_GetAttrString((PyObject*)_wrapper, "wheelEvent");
  PyErr_Clear();
  if (obj && !PythonQtSlotFunction_Check(obj)) {
    static const char* argumentList[] ={"" , "QWheelEvent*"};
    static const PythonQtMethodInfo* methodInfo = PythonQtMethodInfo::getCachedMethodInfoFromArgumentList(2, argumentList);
    void* args[2] = {NULL, (void*)&arg__1};
    PyObject* result = PythonQtSignalTarget::call(obj, methodInfo, args, true);
    if (result) { Py_DECREF(result); } 
    Py_DECREF(obj);
    return;
  }
  Py_XDECREF(obj);
}
  PlaylistContainer::wheelEvent(arg__1);
}
PlaylistContainer* PythonQtWrapper_PlaylistContainer::new_PlaylistContainer(QWidget*  parent)
{ 
return new PythonQtShell_PlaylistContainer(parent); }

void PythonQtWrapper_PlaylistContainer::SetActions(PlaylistContainer* theWrappedObject, QAction*  new_playlist, QAction*  save_playlist, QAction*  load_playlist, QAction*  next_playlist, QAction*  previous_playlist)
{
  ( theWrappedObject->SetActions(new_playlist, save_playlist, load_playlist, next_playlist, previous_playlist));
}

void PythonQtWrapper_PlaylistContainer::SetManager(PlaylistContainer* theWrappedObject, PlaylistManager*  manager)
{
  ( theWrappedObject->SetManager(manager));
}

void PythonQtWrapper_PlaylistContainer::resizeEvent(PlaylistContainer* theWrappedObject, QResizeEvent*  arg__1)
{
  ( ((PythonQtPublicPromoter_PlaylistContainer*)theWrappedObject)->promoted_resizeEvent(arg__1));
}



PlaylistItemPtr* PythonQtWrapper_PlaylistItemPtr::new_PlaylistItemPtr()
{ 
return new PythonQtShell_PlaylistItemPtr(); }



void PythonQtShell_PlaylistManager::ChangePlaylistOrder(const QList<int >&  ids)
{
if (_wrapper) {
  PyObject* obj = PyObject_GetAttrString((PyObject*)_wrapper, "ChangePlaylistOrder");
  PyErr_Clear();
  if (obj && !PythonQtSlotFunction_Check(obj)) {
    static const char* argumentList[] ={"" , "const QList<int >&"};
    static const PythonQtMethodInfo* methodInfo = PythonQtMethodInfo::getCachedMethodInfoFromArgumentList(2, argumentList);
    void* args[2] = {NULL, (void*)&ids};
    PyObject* result = PythonQtSignalTarget::call(obj, methodInfo, args, true);
    if (result) { Py_DECREF(result); } 
    Py_DECREF(obj);
    return;
  }
  Py_XDECREF(obj);
}
  PlaylistManager::ChangePlaylistOrder(ids);
}
void PythonQtShell_PlaylistManager::ClearCurrent()
{
if (_wrapper) {
  PyObject* obj = PyObject_GetAttrString((PyObject*)_wrapper, "ClearCurrent");
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
  Py_XDECREF(obj);
}
  PlaylistManager::ClearCurrent();
}
QList<Playlist* >  PythonQtShell_PlaylistManager::GetAllPlaylists() const
{
if (_wrapper) {
  PyObject* obj = PyObject_GetAttrString((PyObject*)_wrapper, "GetAllPlaylists");
  PyErr_Clear();
  if (obj && !PythonQtSlotFunction_Check(obj)) {
    static const char* argumentList[] ={"QList<Playlist* >"};
    static const PythonQtMethodInfo* methodInfo = PythonQtMethodInfo::getCachedMethodInfoFromArgumentList(1, argumentList);
      QList<Playlist* > returnValue;
    void* args[1] = {NULL};
    PyObject* result = PythonQtSignalTarget::call(obj, methodInfo, args, true);
    if (result) {
      args[0] = PythonQtConv::ConvertPythonToQt(methodInfo->parameters().at(0), result, false, NULL, &returnValue);
      if (args[0]!=&returnValue) {
        if (args[0]==NULL) {
          PythonQt::priv()->handleVirtualOverloadReturnError("GetAllPlaylists", methodInfo, result);
        } else {
          returnValue = *((QList<Playlist* >*)args[0]);
        }
      }
    }
    if (result) { Py_DECREF(result); } 
    Py_DECREF(obj);
    return returnValue;
  }
  Py_XDECREF(obj);
}
  return PlaylistManager::GetAllPlaylists();
}
SpecialPlaylistType*  PythonQtShell_PlaylistManager::GetPlaylistType(const QString&  type) const
{
if (_wrapper) {
  PyObject* obj = PyObject_GetAttrString((PyObject*)_wrapper, "GetPlaylistType");
  PyErr_Clear();
  if (obj && !PythonQtSlotFunction_Check(obj)) {
    static const char* argumentList[] ={"SpecialPlaylistType*" , "const QString&"};
    static const PythonQtMethodInfo* methodInfo = PythonQtMethodInfo::getCachedMethodInfoFromArgumentList(2, argumentList);
      SpecialPlaylistType* returnValue = 0;
    void* args[2] = {NULL, (void*)&type};
    PyObject* result = PythonQtSignalTarget::call(obj, methodInfo, args, true);
    if (result) {
      args[0] = PythonQtConv::ConvertPythonToQt(methodInfo->parameters().at(0), result, false, NULL, &returnValue);
      if (args[0]!=&returnValue) {
        if (args[0]==NULL) {
          PythonQt::priv()->handleVirtualOverloadReturnError("GetPlaylistType", methodInfo, result);
        } else {
          returnValue = *((SpecialPlaylistType**)args[0]);
        }
      }
    }
    if (result) { Py_DECREF(result); } 
    Py_DECREF(obj);
    return returnValue;
  }
  Py_XDECREF(obj);
}
  return PlaylistManager::GetPlaylistType(type);
}
void PythonQtShell_PlaylistManager::InvalidateDeletedSongs()
{
if (_wrapper) {
  PyObject* obj = PyObject_GetAttrString((PyObject*)_wrapper, "InvalidateDeletedSongs");
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
  Py_XDECREF(obj);
}
  PlaylistManager::InvalidateDeletedSongs();
}
void PythonQtShell_PlaylistManager::Load(const QString&  filename)
{
if (_wrapper) {
  PyObject* obj = PyObject_GetAttrString((PyObject*)_wrapper, "Load");
  PyErr_Clear();
  if (obj && !PythonQtSlotFunction_Check(obj)) {
    static const char* argumentList[] ={"" , "const QString&"};
    static const PythonQtMethodInfo* methodInfo = PythonQtMethodInfo::getCachedMethodInfoFromArgumentList(2, argumentList);
    void* args[2] = {NULL, (void*)&filename};
    PyObject* result = PythonQtSignalTarget::call(obj, methodInfo, args, true);
    if (result) { Py_DECREF(result); } 
    Py_DECREF(obj);
    return;
  }
  Py_XDECREF(obj);
}
  PlaylistManager::Load(filename);
}
void PythonQtShell_PlaylistManager::New(const QString&  name, const QList<Song >&  songs, const QString&  special_type)
{
if (_wrapper) {
  PyObject* obj = PyObject_GetAttrString((PyObject*)_wrapper, "New");
  PyErr_Clear();
  if (obj && !PythonQtSlotFunction_Check(obj)) {
    static const char* argumentList[] ={"" , "const QString&" , "const QList<Song >&" , "const QString&"};
    static const PythonQtMethodInfo* methodInfo = PythonQtMethodInfo::getCachedMethodInfoFromArgumentList(4, argumentList);
    void* args[4] = {NULL, (void*)&name, (void*)&songs, (void*)&special_type};
    PyObject* result = PythonQtSignalTarget::call(obj, methodInfo, args, true);
    if (result) { Py_DECREF(result); } 
    Py_DECREF(obj);
    return;
  }
  Py_XDECREF(obj);
}
  PlaylistManager::New(name, songs, special_type);
}
void PythonQtShell_PlaylistManager::PlaySmartPlaylist(smart_playlists::GeneratorPtr  generator, bool  as_new, bool  clear)
{
if (_wrapper) {
  PyObject* obj = PyObject_GetAttrString((PyObject*)_wrapper, "PlaySmartPlaylist");
  PyErr_Clear();
  if (obj && !PythonQtSlotFunction_Check(obj)) {
    static const char* argumentList[] ={"" , "smart_playlists::GeneratorPtr" , "bool" , "bool"};
    static const PythonQtMethodInfo* methodInfo = PythonQtMethodInfo::getCachedMethodInfoFromArgumentList(4, argumentList);
    void* args[4] = {NULL, (void*)&generator, (void*)&as_new, (void*)&clear};
    PyObject* result = PythonQtSignalTarget::call(obj, methodInfo, args, true);
    if (result) { Py_DECREF(result); } 
    Py_DECREF(obj);
    return;
  }
  Py_XDECREF(obj);
}
  PlaylistManager::PlaySmartPlaylist(generator, as_new, clear);
}
void PythonQtShell_PlaylistManager::RateCurrentSong(double  rating)
{
if (_wrapper) {
  PyObject* obj = PyObject_GetAttrString((PyObject*)_wrapper, "RateCurrentSong");
  PyErr_Clear();
  if (obj && !PythonQtSlotFunction_Check(obj)) {
    static const char* argumentList[] ={"" , "double"};
    static const PythonQtMethodInfo* methodInfo = PythonQtMethodInfo::getCachedMethodInfoFromArgumentList(2, argumentList);
    void* args[2] = {NULL, (void*)&rating};
    PyObject* result = PythonQtSignalTarget::call(obj, methodInfo, args, true);
    if (result) { Py_DECREF(result); } 
    Py_DECREF(obj);
    return;
  }
  Py_XDECREF(obj);
}
  PlaylistManager::RateCurrentSong(rating);
}
void PythonQtShell_PlaylistManager::RateCurrentSong(int  rating)
{
if (_wrapper) {
  PyObject* obj = PyObject_GetAttrString((PyObject*)_wrapper, "RateCurrentSong");
  PyErr_Clear();
  if (obj && !PythonQtSlotFunction_Check(obj)) {
    static const char* argumentList[] ={"" , "int"};
    static const PythonQtMethodInfo* methodInfo = PythonQtMethodInfo::getCachedMethodInfoFromArgumentList(2, argumentList);
    void* args[2] = {NULL, (void*)&rating};
    PyObject* result = PythonQtSignalTarget::call(obj, methodInfo, args, true);
    if (result) { Py_DECREF(result); } 
    Py_DECREF(obj);
    return;
  }
  Py_XDECREF(obj);
}
  PlaylistManager::RateCurrentSong(rating);
}
void PythonQtShell_PlaylistManager::RegisterSpecialPlaylistType(SpecialPlaylistType*  type)
{
if (_wrapper) {
  PyObject* obj = PyObject_GetAttrString((PyObject*)_wrapper, "RegisterSpecialPlaylistType");
  PyErr_Clear();
  if (obj && !PythonQtSlotFunction_Check(obj)) {
    static const char* argumentList[] ={"" , "SpecialPlaylistType*"};
    static const PythonQtMethodInfo* methodInfo = PythonQtMethodInfo::getCachedMethodInfoFromArgumentList(2, argumentList);
    void* args[2] = {NULL, (void*)&type};
    PyObject* result = PythonQtSignalTarget::call(obj, methodInfo, args, true);
    if (result) { Py_DECREF(result); } 
    Py_DECREF(obj);
    return;
  }
  Py_XDECREF(obj);
}
  PlaylistManager::RegisterSpecialPlaylistType(type);
}
void PythonQtShell_PlaylistManager::Remove(int  id)
{
if (_wrapper) {
  PyObject* obj = PyObject_GetAttrString((PyObject*)_wrapper, "Remove");
  PyErr_Clear();
  if (obj && !PythonQtSlotFunction_Check(obj)) {
    static const char* argumentList[] ={"" , "int"};
    static const PythonQtMethodInfo* methodInfo = PythonQtMethodInfo::getCachedMethodInfoFromArgumentList(2, argumentList);
    void* args[2] = {NULL, (void*)&id};
    PyObject* result = PythonQtSignalTarget::call(obj, methodInfo, args, true);
    if (result) { Py_DECREF(result); } 
    Py_DECREF(obj);
    return;
  }
  Py_XDECREF(obj);
}
  PlaylistManager::Remove(id);
}
void PythonQtShell_PlaylistManager::RemoveDeletedSongs()
{
if (_wrapper) {
  PyObject* obj = PyObject_GetAttrString((PyObject*)_wrapper, "RemoveDeletedSongs");
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
  Py_XDECREF(obj);
}
  PlaylistManager::RemoveDeletedSongs();
}
void PythonQtShell_PlaylistManager::Rename(int  id, const QString&  new_name)
{
if (_wrapper) {
  PyObject* obj = PyObject_GetAttrString((PyObject*)_wrapper, "Rename");
  PyErr_Clear();
  if (obj && !PythonQtSlotFunction_Check(obj)) {
    static const char* argumentList[] ={"" , "int" , "const QString&"};
    static const PythonQtMethodInfo* methodInfo = PythonQtMethodInfo::getCachedMethodInfoFromArgumentList(3, argumentList);
    void* args[3] = {NULL, (void*)&id, (void*)&new_name};
    PyObject* result = PythonQtSignalTarget::call(obj, methodInfo, args, true);
    if (result) { Py_DECREF(result); } 
    Py_DECREF(obj);
    return;
  }
  Py_XDECREF(obj);
}
  PlaylistManager::Rename(id, new_name);
}
void PythonQtShell_PlaylistManager::Save(int  id, const QString&  filename)
{
if (_wrapper) {
  PyObject* obj = PyObject_GetAttrString((PyObject*)_wrapper, "Save");
  PyErr_Clear();
  if (obj && !PythonQtSlotFunction_Check(obj)) {
    static const char* argumentList[] ={"" , "int" , "const QString&"};
    static const PythonQtMethodInfo* methodInfo = PythonQtMethodInfo::getCachedMethodInfoFromArgumentList(3, argumentList);
    void* args[3] = {NULL, (void*)&id, (void*)&filename};
    PyObject* result = PythonQtSignalTarget::call(obj, methodInfo, args, true);
    if (result) { Py_DECREF(result); } 
    Py_DECREF(obj);
    return;
  }
  Py_XDECREF(obj);
}
  PlaylistManager::Save(id, filename);
}
void PythonQtShell_PlaylistManager::SelectionChanged(const QItemSelection&  selection)
{
if (_wrapper) {
  PyObject* obj = PyObject_GetAttrString((PyObject*)_wrapper, "SelectionChanged");
  PyErr_Clear();
  if (obj && !PythonQtSlotFunction_Check(obj)) {
    static const char* argumentList[] ={"" , "const QItemSelection&"};
    static const PythonQtMethodInfo* methodInfo = PythonQtMethodInfo::getCachedMethodInfoFromArgumentList(2, argumentList);
    void* args[2] = {NULL, (void*)&selection};
    PyObject* result = PythonQtSignalTarget::call(obj, methodInfo, args, true);
    if (result) { Py_DECREF(result); } 
    Py_DECREF(obj);
    return;
  }
  Py_XDECREF(obj);
}
  PlaylistManager::SelectionChanged(selection);
}
void PythonQtShell_PlaylistManager::SetActivePaused()
{
if (_wrapper) {
  PyObject* obj = PyObject_GetAttrString((PyObject*)_wrapper, "SetActivePaused");
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
  Py_XDECREF(obj);
}
  PlaylistManager::SetActivePaused();
}
void PythonQtShell_PlaylistManager::SetActivePlaying()
{
if (_wrapper) {
  PyObject* obj = PyObject_GetAttrString((PyObject*)_wrapper, "SetActivePlaying");
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
  Py_XDECREF(obj);
}
  PlaylistManager::SetActivePlaying();
}
void PythonQtShell_PlaylistManager::SetActivePlaylist(int  id)
{
if (_wrapper) {
  PyObject* obj = PyObject_GetAttrString((PyObject*)_wrapper, "SetActivePlaylist");
  PyErr_Clear();
  if (obj && !PythonQtSlotFunction_Check(obj)) {
    static const char* argumentList[] ={"" , "int"};
    static const PythonQtMethodInfo* methodInfo = PythonQtMethodInfo::getCachedMethodInfoFromArgumentList(2, argumentList);
    void* args[2] = {NULL, (void*)&id};
    PyObject* result = PythonQtSignalTarget::call(obj, methodInfo, args, true);
    if (result) { Py_DECREF(result); } 
    Py_DECREF(obj);
    return;
  }
  Py_XDECREF(obj);
}
  PlaylistManager::SetActivePlaylist(id);
}
void PythonQtShell_PlaylistManager::SetActiveStopped()
{
if (_wrapper) {
  PyObject* obj = PyObject_GetAttrString((PyObject*)_wrapper, "SetActiveStopped");
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
  Py_XDECREF(obj);
}
  PlaylistManager::SetActiveStopped();
}
void PythonQtShell_PlaylistManager::SetActiveStreamMetadata(const QUrl&  url, const Song&  song)
{
if (_wrapper) {
  PyObject* obj = PyObject_GetAttrString((PyObject*)_wrapper, "SetActiveStreamMetadata");
  PyErr_Clear();
  if (obj && !PythonQtSlotFunction_Check(obj)) {
    static const char* argumentList[] ={"" , "const QUrl&" , "const Song&"};
    static const PythonQtMethodInfo* methodInfo = PythonQtMethodInfo::getCachedMethodInfoFromArgumentList(3, argumentList);
    void* args[3] = {NULL, (void*)&url, (void*)&song};
    PyObject* result = PythonQtSignalTarget::call(obj, methodInfo, args, true);
    if (result) { Py_DECREF(result); } 
    Py_DECREF(obj);
    return;
  }
  Py_XDECREF(obj);
}
  PlaylistManager::SetActiveStreamMetadata(url, song);
}
void PythonQtShell_PlaylistManager::SetActiveToCurrent()
{
if (_wrapper) {
  PyObject* obj = PyObject_GetAttrString((PyObject*)_wrapper, "SetActiveToCurrent");
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
  Py_XDECREF(obj);
}
  PlaylistManager::SetActiveToCurrent();
}
void PythonQtShell_PlaylistManager::SetCurrentPlaylist(int  id)
{
if (_wrapper) {
  PyObject* obj = PyObject_GetAttrString((PyObject*)_wrapper, "SetCurrentPlaylist");
  PyErr_Clear();
  if (obj && !PythonQtSlotFunction_Check(obj)) {
    static const char* argumentList[] ={"" , "int"};
    static const PythonQtMethodInfo* methodInfo = PythonQtMethodInfo::getCachedMethodInfoFromArgumentList(2, argumentList);
    void* args[2] = {NULL, (void*)&id};
    PyObject* result = PythonQtSignalTarget::call(obj, methodInfo, args, true);
    if (result) { Py_DECREF(result); } 
    Py_DECREF(obj);
    return;
  }
  Py_XDECREF(obj);
}
  PlaylistManager::SetCurrentPlaylist(id);
}
void PythonQtShell_PlaylistManager::ShuffleCurrent()
{
if (_wrapper) {
  PyObject* obj = PyObject_GetAttrString((PyObject*)_wrapper, "ShuffleCurrent");
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
  Py_XDECREF(obj);
}
  PlaylistManager::ShuffleCurrent();
}
void PythonQtShell_PlaylistManager::SongChangeRequestProcessed(const QUrl&  url, bool  valid)
{
if (_wrapper) {
  PyObject* obj = PyObject_GetAttrString((PyObject*)_wrapper, "SongChangeRequestProcessed");
  PyErr_Clear();
  if (obj && !PythonQtSlotFunction_Check(obj)) {
    static const char* argumentList[] ={"" , "const QUrl&" , "bool"};
    static const PythonQtMethodInfo* methodInfo = PythonQtMethodInfo::getCachedMethodInfoFromArgumentList(3, argumentList);
    void* args[3] = {NULL, (void*)&url, (void*)&valid};
    PyObject* result = PythonQtSignalTarget::call(obj, methodInfo, args, true);
    if (result) { Py_DECREF(result); } 
    Py_DECREF(obj);
    return;
  }
  Py_XDECREF(obj);
}
  PlaylistManager::SongChangeRequestProcessed(url, valid);
}
void PythonQtShell_PlaylistManager::UnregisterSpecialPlaylistType(SpecialPlaylistType*  type)
{
if (_wrapper) {
  PyObject* obj = PyObject_GetAttrString((PyObject*)_wrapper, "UnregisterSpecialPlaylistType");
  PyErr_Clear();
  if (obj && !PythonQtSlotFunction_Check(obj)) {
    static const char* argumentList[] ={"" , "SpecialPlaylistType*"};
    static const PythonQtMethodInfo* methodInfo = PythonQtMethodInfo::getCachedMethodInfoFromArgumentList(2, argumentList);
    void* args[2] = {NULL, (void*)&type};
    PyObject* result = PythonQtSignalTarget::call(obj, methodInfo, args, true);
    if (result) { Py_DECREF(result); } 
    Py_DECREF(obj);
    return;
  }
  Py_XDECREF(obj);
}
  PlaylistManager::UnregisterSpecialPlaylistType(type);
}
Playlist*  PythonQtShell_PlaylistManager::active() const
{
if (_wrapper) {
  PyObject* obj = PyObject_GetAttrString((PyObject*)_wrapper, "active");
  PyErr_Clear();
  if (obj && !PythonQtSlotFunction_Check(obj)) {
    static const char* argumentList[] ={"Playlist*"};
    static const PythonQtMethodInfo* methodInfo = PythonQtMethodInfo::getCachedMethodInfoFromArgumentList(1, argumentList);
      Playlist* returnValue = 0;
    void* args[1] = {NULL};
    PyObject* result = PythonQtSignalTarget::call(obj, methodInfo, args, true);
    if (result) {
      args[0] = PythonQtConv::ConvertPythonToQt(methodInfo->parameters().at(0), result, false, NULL, &returnValue);
      if (args[0]!=&returnValue) {
        if (args[0]==NULL) {
          PythonQt::priv()->handleVirtualOverloadReturnError("active", methodInfo, result);
        } else {
          returnValue = *((Playlist**)args[0]);
        }
      }
    }
    if (result) { Py_DECREF(result); } 
    Py_DECREF(obj);
    return returnValue;
  }
  Py_XDECREF(obj);
}
  return PlaylistManager::active();
}
int  PythonQtShell_PlaylistManager::active_id() const
{
if (_wrapper) {
  PyObject* obj = PyObject_GetAttrString((PyObject*)_wrapper, "active_id");
  PyErr_Clear();
  if (obj && !PythonQtSlotFunction_Check(obj)) {
    static const char* argumentList[] ={"int"};
    static const PythonQtMethodInfo* methodInfo = PythonQtMethodInfo::getCachedMethodInfoFromArgumentList(1, argumentList);
      int returnValue = 0;
    void* args[1] = {NULL};
    PyObject* result = PythonQtSignalTarget::call(obj, methodInfo, args, true);
    if (result) {
      args[0] = PythonQtConv::ConvertPythonToQt(methodInfo->parameters().at(0), result, false, NULL, &returnValue);
      if (args[0]!=&returnValue) {
        if (args[0]==NULL) {
          PythonQt::priv()->handleVirtualOverloadReturnError("active_id", methodInfo, result);
        } else {
          returnValue = *((int*)args[0]);
        }
      }
    }
    if (result) { Py_DECREF(result); } 
    Py_DECREF(obj);
    return returnValue;
  }
  Py_XDECREF(obj);
}
  return PlaylistManager::active_id();
}
QItemSelection  PythonQtShell_PlaylistManager::active_selection() const
{
if (_wrapper) {
  PyObject* obj = PyObject_GetAttrString((PyObject*)_wrapper, "active_selection");
  PyErr_Clear();
  if (obj && !PythonQtSlotFunction_Check(obj)) {
    static const char* argumentList[] ={"QItemSelection"};
    static const PythonQtMethodInfo* methodInfo = PythonQtMethodInfo::getCachedMethodInfoFromArgumentList(1, argumentList);
      QItemSelection returnValue;
    void* args[1] = {NULL};
    PyObject* result = PythonQtSignalTarget::call(obj, methodInfo, args, true);
    if (result) {
      args[0] = PythonQtConv::ConvertPythonToQt(methodInfo->parameters().at(0), result, false, NULL, &returnValue);
      if (args[0]!=&returnValue) {
        if (args[0]==NULL) {
          PythonQt::priv()->handleVirtualOverloadReturnError("active_selection", methodInfo, result);
        } else {
          returnValue = *((QItemSelection*)args[0]);
        }
      }
    }
    if (result) { Py_DECREF(result); } 
    Py_DECREF(obj);
    return returnValue;
  }
  Py_XDECREF(obj);
}
  return PlaylistManager::active_selection();
}
void PythonQtShell_PlaylistManager::childEvent(QChildEvent*  arg__1)
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
  Py_XDECREF(obj);
}
  PlaylistManager::childEvent(arg__1);
}
Playlist*  PythonQtShell_PlaylistManager::current() const
{
if (_wrapper) {
  PyObject* obj = PyObject_GetAttrString((PyObject*)_wrapper, "current");
  PyErr_Clear();
  if (obj && !PythonQtSlotFunction_Check(obj)) {
    static const char* argumentList[] ={"Playlist*"};
    static const PythonQtMethodInfo* methodInfo = PythonQtMethodInfo::getCachedMethodInfoFromArgumentList(1, argumentList);
      Playlist* returnValue = 0;
    void* args[1] = {NULL};
    PyObject* result = PythonQtSignalTarget::call(obj, methodInfo, args, true);
    if (result) {
      args[0] = PythonQtConv::ConvertPythonToQt(methodInfo->parameters().at(0), result, false, NULL, &returnValue);
      if (args[0]!=&returnValue) {
        if (args[0]==NULL) {
          PythonQt::priv()->handleVirtualOverloadReturnError("current", methodInfo, result);
        } else {
          returnValue = *((Playlist**)args[0]);
        }
      }
    }
    if (result) { Py_DECREF(result); } 
    Py_DECREF(obj);
    return returnValue;
  }
  Py_XDECREF(obj);
}
  return PlaylistManager::current();
}
int  PythonQtShell_PlaylistManager::current_id() const
{
if (_wrapper) {
  PyObject* obj = PyObject_GetAttrString((PyObject*)_wrapper, "current_id");
  PyErr_Clear();
  if (obj && !PythonQtSlotFunction_Check(obj)) {
    static const char* argumentList[] ={"int"};
    static const PythonQtMethodInfo* methodInfo = PythonQtMethodInfo::getCachedMethodInfoFromArgumentList(1, argumentList);
      int returnValue = 0;
    void* args[1] = {NULL};
    PyObject* result = PythonQtSignalTarget::call(obj, methodInfo, args, true);
    if (result) {
      args[0] = PythonQtConv::ConvertPythonToQt(methodInfo->parameters().at(0), result, false, NULL, &returnValue);
      if (args[0]!=&returnValue) {
        if (args[0]==NULL) {
          PythonQt::priv()->handleVirtualOverloadReturnError("current_id", methodInfo, result);
        } else {
          returnValue = *((int*)args[0]);
        }
      }
    }
    if (result) { Py_DECREF(result); } 
    Py_DECREF(obj);
    return returnValue;
  }
  Py_XDECREF(obj);
}
  return PlaylistManager::current_id();
}
QItemSelection  PythonQtShell_PlaylistManager::current_selection() const
{
if (_wrapper) {
  PyObject* obj = PyObject_GetAttrString((PyObject*)_wrapper, "current_selection");
  PyErr_Clear();
  if (obj && !PythonQtSlotFunction_Check(obj)) {
    static const char* argumentList[] ={"QItemSelection"};
    static const PythonQtMethodInfo* methodInfo = PythonQtMethodInfo::getCachedMethodInfoFromArgumentList(1, argumentList);
      QItemSelection returnValue;
    void* args[1] = {NULL};
    PyObject* result = PythonQtSignalTarget::call(obj, methodInfo, args, true);
    if (result) {
      args[0] = PythonQtConv::ConvertPythonToQt(methodInfo->parameters().at(0), result, false, NULL, &returnValue);
      if (args[0]!=&returnValue) {
        if (args[0]==NULL) {
          PythonQt::priv()->handleVirtualOverloadReturnError("current_selection", methodInfo, result);
        } else {
          returnValue = *((QItemSelection*)args[0]);
        }
      }
    }
    if (result) { Py_DECREF(result); } 
    Py_DECREF(obj);
    return returnValue;
  }
  Py_XDECREF(obj);
}
  return PlaylistManager::current_selection();
}
void PythonQtShell_PlaylistManager::customEvent(QEvent*  arg__1)
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
  Py_XDECREF(obj);
}
  PlaylistManager::customEvent(arg__1);
}
bool  PythonQtShell_PlaylistManager::event(QEvent*  arg__1)
{
if (_wrapper) {
  PyObject* obj = PyObject_GetAttrString((PyObject*)_wrapper, "event");
  PyErr_Clear();
  if (obj && !PythonQtSlotFunction_Check(obj)) {
    static const char* argumentList[] ={"bool" , "QEvent*"};
    static const PythonQtMethodInfo* methodInfo = PythonQtMethodInfo::getCachedMethodInfoFromArgumentList(2, argumentList);
      bool returnValue = 0;
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
  Py_XDECREF(obj);
}
  return PlaylistManager::event(arg__1);
}
bool  PythonQtShell_PlaylistManager::eventFilter(QObject*  arg__1, QEvent*  arg__2)
{
if (_wrapper) {
  PyObject* obj = PyObject_GetAttrString((PyObject*)_wrapper, "eventFilter");
  PyErr_Clear();
  if (obj && !PythonQtSlotFunction_Check(obj)) {
    static const char* argumentList[] ={"bool" , "QObject*" , "QEvent*"};
    static const PythonQtMethodInfo* methodInfo = PythonQtMethodInfo::getCachedMethodInfoFromArgumentList(3, argumentList);
      bool returnValue = 0;
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
  Py_XDECREF(obj);
}
  return PlaylistManager::eventFilter(arg__1, arg__2);
}
LibraryBackend*  PythonQtShell_PlaylistManager::library_backend() const
{
if (_wrapper) {
  PyObject* obj = PyObject_GetAttrString((PyObject*)_wrapper, "library_backend");
  PyErr_Clear();
  if (obj && !PythonQtSlotFunction_Check(obj)) {
    static const char* argumentList[] ={"LibraryBackend*"};
    static const PythonQtMethodInfo* methodInfo = PythonQtMethodInfo::getCachedMethodInfoFromArgumentList(1, argumentList);
      LibraryBackend* returnValue = 0;
    void* args[1] = {NULL};
    PyObject* result = PythonQtSignalTarget::call(obj, methodInfo, args, true);
    if (result) {
      args[0] = PythonQtConv::ConvertPythonToQt(methodInfo->parameters().at(0), result, false, NULL, &returnValue);
      if (args[0]!=&returnValue) {
        if (args[0]==NULL) {
          PythonQt::priv()->handleVirtualOverloadReturnError("library_backend", methodInfo, result);
        } else {
          returnValue = *((LibraryBackend**)args[0]);
        }
      }
    }
    if (result) { Py_DECREF(result); } 
    Py_DECREF(obj);
    return returnValue;
  }
  Py_XDECREF(obj);
}
  return PlaylistManager::library_backend();
}
QString  PythonQtShell_PlaylistManager::name(int  index) const
{
if (_wrapper) {
  PyObject* obj = PyObject_GetAttrString((PyObject*)_wrapper, "name");
  PyErr_Clear();
  if (obj && !PythonQtSlotFunction_Check(obj)) {
    static const char* argumentList[] ={"QString" , "int"};
    static const PythonQtMethodInfo* methodInfo = PythonQtMethodInfo::getCachedMethodInfoFromArgumentList(2, argumentList);
      QString returnValue;
    void* args[2] = {NULL, (void*)&index};
    PyObject* result = PythonQtSignalTarget::call(obj, methodInfo, args, true);
    if (result) {
      args[0] = PythonQtConv::ConvertPythonToQt(methodInfo->parameters().at(0), result, false, NULL, &returnValue);
      if (args[0]!=&returnValue) {
        if (args[0]==NULL) {
          PythonQt::priv()->handleVirtualOverloadReturnError("name", methodInfo, result);
        } else {
          returnValue = *((QString*)args[0]);
        }
      }
    }
    if (result) { Py_DECREF(result); } 
    Py_DECREF(obj);
    return returnValue;
  }
  Py_XDECREF(obj);
}
  return PlaylistManager::name(index);
}
PlaylistParser*  PythonQtShell_PlaylistManager::parser() const
{
if (_wrapper) {
  PyObject* obj = PyObject_GetAttrString((PyObject*)_wrapper, "parser");
  PyErr_Clear();
  if (obj && !PythonQtSlotFunction_Check(obj)) {
    static const char* argumentList[] ={"PlaylistParser*"};
    static const PythonQtMethodInfo* methodInfo = PythonQtMethodInfo::getCachedMethodInfoFromArgumentList(1, argumentList);
      PlaylistParser* returnValue = 0;
    void* args[1] = {NULL};
    PyObject* result = PythonQtSignalTarget::call(obj, methodInfo, args, true);
    if (result) {
      args[0] = PythonQtConv::ConvertPythonToQt(methodInfo->parameters().at(0), result, false, NULL, &returnValue);
      if (args[0]!=&returnValue) {
        if (args[0]==NULL) {
          PythonQt::priv()->handleVirtualOverloadReturnError("parser", methodInfo, result);
        } else {
          returnValue = *((PlaylistParser**)args[0]);
        }
      }
    }
    if (result) { Py_DECREF(result); } 
    Py_DECREF(obj);
    return returnValue;
  }
  Py_XDECREF(obj);
}
  return PlaylistManager::parser();
}
Playlist*  PythonQtShell_PlaylistManager::playlist(int  id) const
{
if (_wrapper) {
  PyObject* obj = PyObject_GetAttrString((PyObject*)_wrapper, "playlist");
  PyErr_Clear();
  if (obj && !PythonQtSlotFunction_Check(obj)) {
    static const char* argumentList[] ={"Playlist*" , "int"};
    static const PythonQtMethodInfo* methodInfo = PythonQtMethodInfo::getCachedMethodInfoFromArgumentList(2, argumentList);
      Playlist* returnValue = 0;
    void* args[2] = {NULL, (void*)&id};
    PyObject* result = PythonQtSignalTarget::call(obj, methodInfo, args, true);
    if (result) {
      args[0] = PythonQtConv::ConvertPythonToQt(methodInfo->parameters().at(0), result, false, NULL, &returnValue);
      if (args[0]!=&returnValue) {
        if (args[0]==NULL) {
          PythonQt::priv()->handleVirtualOverloadReturnError("playlist", methodInfo, result);
        } else {
          returnValue = *((Playlist**)args[0]);
        }
      }
    }
    if (result) { Py_DECREF(result); } 
    Py_DECREF(obj);
    return returnValue;
  }
  Py_XDECREF(obj);
}
  return PlaylistManager::playlist(id);
}
PlaylistBackend*  PythonQtShell_PlaylistManager::playlist_backend() const
{
if (_wrapper) {
  PyObject* obj = PyObject_GetAttrString((PyObject*)_wrapper, "playlist_backend");
  PyErr_Clear();
  if (obj && !PythonQtSlotFunction_Check(obj)) {
    static const char* argumentList[] ={"PlaylistBackend*"};
    static const PythonQtMethodInfo* methodInfo = PythonQtMethodInfo::getCachedMethodInfoFromArgumentList(1, argumentList);
      PlaylistBackend* returnValue = 0;
    void* args[1] = {NULL};
    PyObject* result = PythonQtSignalTarget::call(obj, methodInfo, args, true);
    if (result) {
      args[0] = PythonQtConv::ConvertPythonToQt(methodInfo->parameters().at(0), result, false, NULL, &returnValue);
      if (args[0]!=&returnValue) {
        if (args[0]==NULL) {
          PythonQt::priv()->handleVirtualOverloadReturnError("playlist_backend", methodInfo, result);
        } else {
          returnValue = *((PlaylistBackend**)args[0]);
        }
      }
    }
    if (result) { Py_DECREF(result); } 
    Py_DECREF(obj);
    return returnValue;
  }
  Py_XDECREF(obj);
}
  return PlaylistManager::playlist_backend();
}
PlaylistContainer*  PythonQtShell_PlaylistManager::playlist_container() const
{
if (_wrapper) {
  PyObject* obj = PyObject_GetAttrString((PyObject*)_wrapper, "playlist_container");
  PyErr_Clear();
  if (obj && !PythonQtSlotFunction_Check(obj)) {
    static const char* argumentList[] ={"PlaylistContainer*"};
    static const PythonQtMethodInfo* methodInfo = PythonQtMethodInfo::getCachedMethodInfoFromArgumentList(1, argumentList);
      PlaylistContainer* returnValue = 0;
    void* args[1] = {NULL};
    PyObject* result = PythonQtSignalTarget::call(obj, methodInfo, args, true);
    if (result) {
      args[0] = PythonQtConv::ConvertPythonToQt(methodInfo->parameters().at(0), result, false, NULL, &returnValue);
      if (args[0]!=&returnValue) {
        if (args[0]==NULL) {
          PythonQt::priv()->handleVirtualOverloadReturnError("playlist_container", methodInfo, result);
        } else {
          returnValue = *((PlaylistContainer**)args[0]);
        }
      }
    }
    if (result) { Py_DECREF(result); } 
    Py_DECREF(obj);
    return returnValue;
  }
  Py_XDECREF(obj);
}
  return PlaylistManager::playlist_container();
}
QItemSelection  PythonQtShell_PlaylistManager::selection(int  id) const
{
if (_wrapper) {
  PyObject* obj = PyObject_GetAttrString((PyObject*)_wrapper, "selection");
  PyErr_Clear();
  if (obj && !PythonQtSlotFunction_Check(obj)) {
    static const char* argumentList[] ={"QItemSelection" , "int"};
    static const PythonQtMethodInfo* methodInfo = PythonQtMethodInfo::getCachedMethodInfoFromArgumentList(2, argumentList);
      QItemSelection returnValue;
    void* args[2] = {NULL, (void*)&id};
    PyObject* result = PythonQtSignalTarget::call(obj, methodInfo, args, true);
    if (result) {
      args[0] = PythonQtConv::ConvertPythonToQt(methodInfo->parameters().at(0), result, false, NULL, &returnValue);
      if (args[0]!=&returnValue) {
        if (args[0]==NULL) {
          PythonQt::priv()->handleVirtualOverloadReturnError("selection", methodInfo, result);
        } else {
          returnValue = *((QItemSelection*)args[0]);
        }
      }
    }
    if (result) { Py_DECREF(result); } 
    Py_DECREF(obj);
    return returnValue;
  }
  Py_XDECREF(obj);
}
  return PlaylistManager::selection(id);
}
PlaylistSequence*  PythonQtShell_PlaylistManager::sequence() const
{
if (_wrapper) {
  PyObject* obj = PyObject_GetAttrString((PyObject*)_wrapper, "sequence");
  PyErr_Clear();
  if (obj && !PythonQtSlotFunction_Check(obj)) {
    static const char* argumentList[] ={"PlaylistSequence*"};
    static const PythonQtMethodInfo* methodInfo = PythonQtMethodInfo::getCachedMethodInfoFromArgumentList(1, argumentList);
      PlaylistSequence* returnValue = 0;
    void* args[1] = {NULL};
    PyObject* result = PythonQtSignalTarget::call(obj, methodInfo, args, true);
    if (result) {
      args[0] = PythonQtConv::ConvertPythonToQt(methodInfo->parameters().at(0), result, false, NULL, &returnValue);
      if (args[0]!=&returnValue) {
        if (args[0]==NULL) {
          PythonQt::priv()->handleVirtualOverloadReturnError("sequence", methodInfo, result);
        } else {
          returnValue = *((PlaylistSequence**)args[0]);
        }
      }
    }
    if (result) { Py_DECREF(result); } 
    Py_DECREF(obj);
    return returnValue;
  }
  Py_XDECREF(obj);
}
  return PlaylistManager::sequence();
}
TaskManager*  PythonQtShell_PlaylistManager::task_manager() const
{
if (_wrapper) {
  PyObject* obj = PyObject_GetAttrString((PyObject*)_wrapper, "task_manager");
  PyErr_Clear();
  if (obj && !PythonQtSlotFunction_Check(obj)) {
    static const char* argumentList[] ={"TaskManager*"};
    static const PythonQtMethodInfo* methodInfo = PythonQtMethodInfo::getCachedMethodInfoFromArgumentList(1, argumentList);
      TaskManager* returnValue = 0;
    void* args[1] = {NULL};
    PyObject* result = PythonQtSignalTarget::call(obj, methodInfo, args, true);
    if (result) {
      args[0] = PythonQtConv::ConvertPythonToQt(methodInfo->parameters().at(0), result, false, NULL, &returnValue);
      if (args[0]!=&returnValue) {
        if (args[0]==NULL) {
          PythonQt::priv()->handleVirtualOverloadReturnError("task_manager", methodInfo, result);
        } else {
          returnValue = *((TaskManager**)args[0]);
        }
      }
    }
    if (result) { Py_DECREF(result); } 
    Py_DECREF(obj);
    return returnValue;
  }
  Py_XDECREF(obj);
}
  return PlaylistManager::task_manager();
}
void PythonQtShell_PlaylistManager::timerEvent(QTimerEvent*  arg__1)
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
  Py_XDECREF(obj);
}
  PlaylistManager::timerEvent(arg__1);
}
PlaylistManager* PythonQtWrapper_PlaylistManager::new_PlaylistManager(TaskManager*  task_manager, QObject*  parent)
{ 
return new PythonQtShell_PlaylistManager(task_manager, parent); }

QList<Playlist* >  PythonQtWrapper_PlaylistManager::GetAllPlaylists(PlaylistManager* theWrappedObject) const
{
  return ( ((PythonQtPublicPromoter_PlaylistManager*)theWrappedObject)->promoted_GetAllPlaylists());
}

QString  PythonQtWrapper_PlaylistManager::static_PlaylistManager_GetNameForNewPlaylist(const QList<Song >&  songs)
{
  return (PlaylistManager::GetNameForNewPlaylist(songs));
}

SpecialPlaylistType*  PythonQtWrapper_PlaylistManager::GetPlaylistType(PlaylistManager* theWrappedObject, const QString&  type) const
{
  return ( ((PythonQtPublicPromoter_PlaylistManager*)theWrappedObject)->promoted_GetPlaylistType(type));
}

void PythonQtWrapper_PlaylistManager::Init(PlaylistManager* theWrappedObject, LibraryBackend*  library_backend, PlaylistBackend*  playlist_backend, PlaylistSequence*  sequence, PlaylistContainer*  playlist_container)
{
  ( theWrappedObject->Init(library_backend, playlist_backend, sequence, playlist_container));
}

void PythonQtWrapper_PlaylistManager::InvalidateDeletedSongs(PlaylistManager* theWrappedObject)
{
  ( ((PythonQtPublicPromoter_PlaylistManager*)theWrappedObject)->promoted_InvalidateDeletedSongs());
}

void PythonQtWrapper_PlaylistManager::RegisterSpecialPlaylistType(PlaylistManager* theWrappedObject, SpecialPlaylistType*  type)
{
  ( ((PythonQtPublicPromoter_PlaylistManager*)theWrappedObject)->promoted_RegisterSpecialPlaylistType(type));
}

void PythonQtWrapper_PlaylistManager::RemoveDeletedSongs(PlaylistManager* theWrappedObject)
{
  ( ((PythonQtPublicPromoter_PlaylistManager*)theWrappedObject)->promoted_RemoveDeletedSongs());
}

void PythonQtWrapper_PlaylistManager::UnregisterSpecialPlaylistType(PlaylistManager* theWrappedObject, SpecialPlaylistType*  type)
{
  ( ((PythonQtPublicPromoter_PlaylistManager*)theWrappedObject)->promoted_UnregisterSpecialPlaylistType(type));
}

Playlist*  PythonQtWrapper_PlaylistManager::active(PlaylistManager* theWrappedObject) const
{
  return ( ((PythonQtPublicPromoter_PlaylistManager*)theWrappedObject)->promoted_active());
}

int  PythonQtWrapper_PlaylistManager::active_id(PlaylistManager* theWrappedObject) const
{
  return ( ((PythonQtPublicPromoter_PlaylistManager*)theWrappedObject)->promoted_active_id());
}

QItemSelection  PythonQtWrapper_PlaylistManager::active_selection(PlaylistManager* theWrappedObject) const
{
  return ( ((PythonQtPublicPromoter_PlaylistManager*)theWrappedObject)->promoted_active_selection());
}

Playlist*  PythonQtWrapper_PlaylistManager::current(PlaylistManager* theWrappedObject) const
{
  return ( ((PythonQtPublicPromoter_PlaylistManager*)theWrappedObject)->promoted_current());
}

int  PythonQtWrapper_PlaylistManager::current_id(PlaylistManager* theWrappedObject) const
{
  return ( ((PythonQtPublicPromoter_PlaylistManager*)theWrappedObject)->promoted_current_id());
}

QItemSelection  PythonQtWrapper_PlaylistManager::current_selection(PlaylistManager* theWrappedObject) const
{
  return ( ((PythonQtPublicPromoter_PlaylistManager*)theWrappedObject)->promoted_current_selection());
}

LibraryBackend*  PythonQtWrapper_PlaylistManager::library_backend(PlaylistManager* theWrappedObject) const
{
  return ( ((PythonQtPublicPromoter_PlaylistManager*)theWrappedObject)->promoted_library_backend());
}

QString  PythonQtWrapper_PlaylistManager::name(PlaylistManager* theWrappedObject, int  index) const
{
  return ( ((PythonQtPublicPromoter_PlaylistManager*)theWrappedObject)->promoted_name(index));
}

PlaylistParser*  PythonQtWrapper_PlaylistManager::parser(PlaylistManager* theWrappedObject) const
{
  return ( ((PythonQtPublicPromoter_PlaylistManager*)theWrappedObject)->promoted_parser());
}

Playlist*  PythonQtWrapper_PlaylistManager::playlist(PlaylistManager* theWrappedObject, int  id) const
{
  return ( ((PythonQtPublicPromoter_PlaylistManager*)theWrappedObject)->promoted_playlist(id));
}

PlaylistBackend*  PythonQtWrapper_PlaylistManager::playlist_backend(PlaylistManager* theWrappedObject) const
{
  return ( ((PythonQtPublicPromoter_PlaylistManager*)theWrappedObject)->promoted_playlist_backend());
}

PlaylistContainer*  PythonQtWrapper_PlaylistManager::playlist_container(PlaylistManager* theWrappedObject) const
{
  return ( ((PythonQtPublicPromoter_PlaylistManager*)theWrappedObject)->promoted_playlist_container());
}

QItemSelection  PythonQtWrapper_PlaylistManager::selection(PlaylistManager* theWrappedObject, int  id) const
{
  return ( ((PythonQtPublicPromoter_PlaylistManager*)theWrappedObject)->promoted_selection(id));
}

PlaylistSequence*  PythonQtWrapper_PlaylistManager::sequence(PlaylistManager* theWrappedObject) const
{
  return ( ((PythonQtPublicPromoter_PlaylistManager*)theWrappedObject)->promoted_sequence());
}

TaskManager*  PythonQtWrapper_PlaylistManager::task_manager(PlaylistManager* theWrappedObject) const
{
  return ( ((PythonQtPublicPromoter_PlaylistManager*)theWrappedObject)->promoted_task_manager());
}



void PythonQtShell_PlaylistManagerInterface::ChangePlaylistOrder(const QList<int >&  ids)
{
if (_wrapper) {
  PyObject* obj = PyObject_GetAttrString((PyObject*)_wrapper, "ChangePlaylistOrder");
  PyErr_Clear();
  if (obj && !PythonQtSlotFunction_Check(obj)) {
    static const char* argumentList[] ={"" , "const QList<int >&"};
    static const PythonQtMethodInfo* methodInfo = PythonQtMethodInfo::getCachedMethodInfoFromArgumentList(2, argumentList);
    void* args[2] = {NULL, (void*)&ids};
    PyObject* result = PythonQtSignalTarget::call(obj, methodInfo, args, true);
    if (result) { Py_DECREF(result); } 
    Py_DECREF(obj);
    return;
  }
  Py_XDECREF(obj);
}
  
}
void PythonQtShell_PlaylistManagerInterface::ClearCurrent()
{
if (_wrapper) {
  PyObject* obj = PyObject_GetAttrString((PyObject*)_wrapper, "ClearCurrent");
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
  Py_XDECREF(obj);
}
  
}
QList<Playlist* >  PythonQtShell_PlaylistManagerInterface::GetAllPlaylists() const
{
if (_wrapper) {
  PyObject* obj = PyObject_GetAttrString((PyObject*)_wrapper, "GetAllPlaylists");
  PyErr_Clear();
  if (obj && !PythonQtSlotFunction_Check(obj)) {
    static const char* argumentList[] ={"QList<Playlist* >"};
    static const PythonQtMethodInfo* methodInfo = PythonQtMethodInfo::getCachedMethodInfoFromArgumentList(1, argumentList);
      QList<Playlist* > returnValue;
    void* args[1] = {NULL};
    PyObject* result = PythonQtSignalTarget::call(obj, methodInfo, args, true);
    if (result) {
      args[0] = PythonQtConv::ConvertPythonToQt(methodInfo->parameters().at(0), result, false, NULL, &returnValue);
      if (args[0]!=&returnValue) {
        if (args[0]==NULL) {
          PythonQt::priv()->handleVirtualOverloadReturnError("GetAllPlaylists", methodInfo, result);
        } else {
          returnValue = *((QList<Playlist* >*)args[0]);
        }
      }
    }
    if (result) { Py_DECREF(result); } 
    Py_DECREF(obj);
    return returnValue;
  }
  Py_XDECREF(obj);
}
  return QList<Playlist* >();
}
SpecialPlaylistType*  PythonQtShell_PlaylistManagerInterface::GetPlaylistType(const QString&  type) const
{
if (_wrapper) {
  PyObject* obj = PyObject_GetAttrString((PyObject*)_wrapper, "GetPlaylistType");
  PyErr_Clear();
  if (obj && !PythonQtSlotFunction_Check(obj)) {
    static const char* argumentList[] ={"SpecialPlaylistType*" , "const QString&"};
    static const PythonQtMethodInfo* methodInfo = PythonQtMethodInfo::getCachedMethodInfoFromArgumentList(2, argumentList);
      SpecialPlaylistType* returnValue = 0;
    void* args[2] = {NULL, (void*)&type};
    PyObject* result = PythonQtSignalTarget::call(obj, methodInfo, args, true);
    if (result) {
      args[0] = PythonQtConv::ConvertPythonToQt(methodInfo->parameters().at(0), result, false, NULL, &returnValue);
      if (args[0]!=&returnValue) {
        if (args[0]==NULL) {
          PythonQt::priv()->handleVirtualOverloadReturnError("GetPlaylistType", methodInfo, result);
        } else {
          returnValue = *((SpecialPlaylistType**)args[0]);
        }
      }
    }
    if (result) { Py_DECREF(result); } 
    Py_DECREF(obj);
    return returnValue;
  }
  Py_XDECREF(obj);
}
  return 0;
}
void PythonQtShell_PlaylistManagerInterface::InvalidateDeletedSongs()
{
if (_wrapper) {
  PyObject* obj = PyObject_GetAttrString((PyObject*)_wrapper, "InvalidateDeletedSongs");
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
  Py_XDECREF(obj);
}
  
}
void PythonQtShell_PlaylistManagerInterface::Load(const QString&  filename)
{
if (_wrapper) {
  PyObject* obj = PyObject_GetAttrString((PyObject*)_wrapper, "Load");
  PyErr_Clear();
  if (obj && !PythonQtSlotFunction_Check(obj)) {
    static const char* argumentList[] ={"" , "const QString&"};
    static const PythonQtMethodInfo* methodInfo = PythonQtMethodInfo::getCachedMethodInfoFromArgumentList(2, argumentList);
    void* args[2] = {NULL, (void*)&filename};
    PyObject* result = PythonQtSignalTarget::call(obj, methodInfo, args, true);
    if (result) { Py_DECREF(result); } 
    Py_DECREF(obj);
    return;
  }
  Py_XDECREF(obj);
}
  
}
void PythonQtShell_PlaylistManagerInterface::New(const QString&  name, const QList<Song >&  songs, const QString&  special_type)
{
if (_wrapper) {
  PyObject* obj = PyObject_GetAttrString((PyObject*)_wrapper, "New");
  PyErr_Clear();
  if (obj && !PythonQtSlotFunction_Check(obj)) {
    static const char* argumentList[] ={"" , "const QString&" , "const QList<Song >&" , "const QString&"};
    static const PythonQtMethodInfo* methodInfo = PythonQtMethodInfo::getCachedMethodInfoFromArgumentList(4, argumentList);
    void* args[4] = {NULL, (void*)&name, (void*)&songs, (void*)&special_type};
    PyObject* result = PythonQtSignalTarget::call(obj, methodInfo, args, true);
    if (result) { Py_DECREF(result); } 
    Py_DECREF(obj);
    return;
  }
  Py_XDECREF(obj);
}
  
}
void PythonQtShell_PlaylistManagerInterface::PlaySmartPlaylist(smart_playlists::GeneratorPtr  generator, bool  as_new, bool  clear)
{
if (_wrapper) {
  PyObject* obj = PyObject_GetAttrString((PyObject*)_wrapper, "PlaySmartPlaylist");
  PyErr_Clear();
  if (obj && !PythonQtSlotFunction_Check(obj)) {
    static const char* argumentList[] ={"" , "smart_playlists::GeneratorPtr" , "bool" , "bool"};
    static const PythonQtMethodInfo* methodInfo = PythonQtMethodInfo::getCachedMethodInfoFromArgumentList(4, argumentList);
    void* args[4] = {NULL, (void*)&generator, (void*)&as_new, (void*)&clear};
    PyObject* result = PythonQtSignalTarget::call(obj, methodInfo, args, true);
    if (result) { Py_DECREF(result); } 
    Py_DECREF(obj);
    return;
  }
  Py_XDECREF(obj);
}
  
}
void PythonQtShell_PlaylistManagerInterface::RateCurrentSong(double  rating)
{
if (_wrapper) {
  PyObject* obj = PyObject_GetAttrString((PyObject*)_wrapper, "RateCurrentSong");
  PyErr_Clear();
  if (obj && !PythonQtSlotFunction_Check(obj)) {
    static const char* argumentList[] ={"" , "double"};
    static const PythonQtMethodInfo* methodInfo = PythonQtMethodInfo::getCachedMethodInfoFromArgumentList(2, argumentList);
    void* args[2] = {NULL, (void*)&rating};
    PyObject* result = PythonQtSignalTarget::call(obj, methodInfo, args, true);
    if (result) { Py_DECREF(result); } 
    Py_DECREF(obj);
    return;
  }
  Py_XDECREF(obj);
}
  
}
void PythonQtShell_PlaylistManagerInterface::RateCurrentSong(int  rating)
{
if (_wrapper) {
  PyObject* obj = PyObject_GetAttrString((PyObject*)_wrapper, "RateCurrentSong");
  PyErr_Clear();
  if (obj && !PythonQtSlotFunction_Check(obj)) {
    static const char* argumentList[] ={"" , "int"};
    static const PythonQtMethodInfo* methodInfo = PythonQtMethodInfo::getCachedMethodInfoFromArgumentList(2, argumentList);
    void* args[2] = {NULL, (void*)&rating};
    PyObject* result = PythonQtSignalTarget::call(obj, methodInfo, args, true);
    if (result) { Py_DECREF(result); } 
    Py_DECREF(obj);
    return;
  }
  Py_XDECREF(obj);
}
  
}
void PythonQtShell_PlaylistManagerInterface::RegisterSpecialPlaylistType(SpecialPlaylistType*  type)
{
if (_wrapper) {
  PyObject* obj = PyObject_GetAttrString((PyObject*)_wrapper, "RegisterSpecialPlaylistType");
  PyErr_Clear();
  if (obj && !PythonQtSlotFunction_Check(obj)) {
    static const char* argumentList[] ={"" , "SpecialPlaylistType*"};
    static const PythonQtMethodInfo* methodInfo = PythonQtMethodInfo::getCachedMethodInfoFromArgumentList(2, argumentList);
    void* args[2] = {NULL, (void*)&type};
    PyObject* result = PythonQtSignalTarget::call(obj, methodInfo, args, true);
    if (result) { Py_DECREF(result); } 
    Py_DECREF(obj);
    return;
  }
  Py_XDECREF(obj);
}
  
}
void PythonQtShell_PlaylistManagerInterface::Remove(int  id)
{
if (_wrapper) {
  PyObject* obj = PyObject_GetAttrString((PyObject*)_wrapper, "Remove");
  PyErr_Clear();
  if (obj && !PythonQtSlotFunction_Check(obj)) {
    static const char* argumentList[] ={"" , "int"};
    static const PythonQtMethodInfo* methodInfo = PythonQtMethodInfo::getCachedMethodInfoFromArgumentList(2, argumentList);
    void* args[2] = {NULL, (void*)&id};
    PyObject* result = PythonQtSignalTarget::call(obj, methodInfo, args, true);
    if (result) { Py_DECREF(result); } 
    Py_DECREF(obj);
    return;
  }
  Py_XDECREF(obj);
}
  
}
void PythonQtShell_PlaylistManagerInterface::RemoveDeletedSongs()
{
if (_wrapper) {
  PyObject* obj = PyObject_GetAttrString((PyObject*)_wrapper, "RemoveDeletedSongs");
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
  Py_XDECREF(obj);
}
  
}
void PythonQtShell_PlaylistManagerInterface::Rename(int  id, const QString&  new_name)
{
if (_wrapper) {
  PyObject* obj = PyObject_GetAttrString((PyObject*)_wrapper, "Rename");
  PyErr_Clear();
  if (obj && !PythonQtSlotFunction_Check(obj)) {
    static const char* argumentList[] ={"" , "int" , "const QString&"};
    static const PythonQtMethodInfo* methodInfo = PythonQtMethodInfo::getCachedMethodInfoFromArgumentList(3, argumentList);
    void* args[3] = {NULL, (void*)&id, (void*)&new_name};
    PyObject* result = PythonQtSignalTarget::call(obj, methodInfo, args, true);
    if (result) { Py_DECREF(result); } 
    Py_DECREF(obj);
    return;
  }
  Py_XDECREF(obj);
}
  
}
void PythonQtShell_PlaylistManagerInterface::Save(int  id, const QString&  filename)
{
if (_wrapper) {
  PyObject* obj = PyObject_GetAttrString((PyObject*)_wrapper, "Save");
  PyErr_Clear();
  if (obj && !PythonQtSlotFunction_Check(obj)) {
    static const char* argumentList[] ={"" , "int" , "const QString&"};
    static const PythonQtMethodInfo* methodInfo = PythonQtMethodInfo::getCachedMethodInfoFromArgumentList(3, argumentList);
    void* args[3] = {NULL, (void*)&id, (void*)&filename};
    PyObject* result = PythonQtSignalTarget::call(obj, methodInfo, args, true);
    if (result) { Py_DECREF(result); } 
    Py_DECREF(obj);
    return;
  }
  Py_XDECREF(obj);
}
  
}
void PythonQtShell_PlaylistManagerInterface::SelectionChanged(const QItemSelection&  selection)
{
if (_wrapper) {
  PyObject* obj = PyObject_GetAttrString((PyObject*)_wrapper, "SelectionChanged");
  PyErr_Clear();
  if (obj && !PythonQtSlotFunction_Check(obj)) {
    static const char* argumentList[] ={"" , "const QItemSelection&"};
    static const PythonQtMethodInfo* methodInfo = PythonQtMethodInfo::getCachedMethodInfoFromArgumentList(2, argumentList);
    void* args[2] = {NULL, (void*)&selection};
    PyObject* result = PythonQtSignalTarget::call(obj, methodInfo, args, true);
    if (result) { Py_DECREF(result); } 
    Py_DECREF(obj);
    return;
  }
  Py_XDECREF(obj);
}
  
}
void PythonQtShell_PlaylistManagerInterface::SetActivePaused()
{
if (_wrapper) {
  PyObject* obj = PyObject_GetAttrString((PyObject*)_wrapper, "SetActivePaused");
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
  Py_XDECREF(obj);
}
  
}
void PythonQtShell_PlaylistManagerInterface::SetActivePlaying()
{
if (_wrapper) {
  PyObject* obj = PyObject_GetAttrString((PyObject*)_wrapper, "SetActivePlaying");
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
  Py_XDECREF(obj);
}
  
}
void PythonQtShell_PlaylistManagerInterface::SetActivePlaylist(int  id)
{
if (_wrapper) {
  PyObject* obj = PyObject_GetAttrString((PyObject*)_wrapper, "SetActivePlaylist");
  PyErr_Clear();
  if (obj && !PythonQtSlotFunction_Check(obj)) {
    static const char* argumentList[] ={"" , "int"};
    static const PythonQtMethodInfo* methodInfo = PythonQtMethodInfo::getCachedMethodInfoFromArgumentList(2, argumentList);
    void* args[2] = {NULL, (void*)&id};
    PyObject* result = PythonQtSignalTarget::call(obj, methodInfo, args, true);
    if (result) { Py_DECREF(result); } 
    Py_DECREF(obj);
    return;
  }
  Py_XDECREF(obj);
}
  
}
void PythonQtShell_PlaylistManagerInterface::SetActiveStopped()
{
if (_wrapper) {
  PyObject* obj = PyObject_GetAttrString((PyObject*)_wrapper, "SetActiveStopped");
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
  Py_XDECREF(obj);
}
  
}
void PythonQtShell_PlaylistManagerInterface::SetActiveStreamMetadata(const QUrl&  url, const Song&  song)
{
if (_wrapper) {
  PyObject* obj = PyObject_GetAttrString((PyObject*)_wrapper, "SetActiveStreamMetadata");
  PyErr_Clear();
  if (obj && !PythonQtSlotFunction_Check(obj)) {
    static const char* argumentList[] ={"" , "const QUrl&" , "const Song&"};
    static const PythonQtMethodInfo* methodInfo = PythonQtMethodInfo::getCachedMethodInfoFromArgumentList(3, argumentList);
    void* args[3] = {NULL, (void*)&url, (void*)&song};
    PyObject* result = PythonQtSignalTarget::call(obj, methodInfo, args, true);
    if (result) { Py_DECREF(result); } 
    Py_DECREF(obj);
    return;
  }
  Py_XDECREF(obj);
}
  
}
void PythonQtShell_PlaylistManagerInterface::SetActiveToCurrent()
{
if (_wrapper) {
  PyObject* obj = PyObject_GetAttrString((PyObject*)_wrapper, "SetActiveToCurrent");
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
  Py_XDECREF(obj);
}
  
}
void PythonQtShell_PlaylistManagerInterface::SetCurrentPlaylist(int  id)
{
if (_wrapper) {
  PyObject* obj = PyObject_GetAttrString((PyObject*)_wrapper, "SetCurrentPlaylist");
  PyErr_Clear();
  if (obj && !PythonQtSlotFunction_Check(obj)) {
    static const char* argumentList[] ={"" , "int"};
    static const PythonQtMethodInfo* methodInfo = PythonQtMethodInfo::getCachedMethodInfoFromArgumentList(2, argumentList);
    void* args[2] = {NULL, (void*)&id};
    PyObject* result = PythonQtSignalTarget::call(obj, methodInfo, args, true);
    if (result) { Py_DECREF(result); } 
    Py_DECREF(obj);
    return;
  }
  Py_XDECREF(obj);
}
  
}
void PythonQtShell_PlaylistManagerInterface::ShuffleCurrent()
{
if (_wrapper) {
  PyObject* obj = PyObject_GetAttrString((PyObject*)_wrapper, "ShuffleCurrent");
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
  Py_XDECREF(obj);
}
  
}
void PythonQtShell_PlaylistManagerInterface::SongChangeRequestProcessed(const QUrl&  url, bool  valid)
{
if (_wrapper) {
  PyObject* obj = PyObject_GetAttrString((PyObject*)_wrapper, "SongChangeRequestProcessed");
  PyErr_Clear();
  if (obj && !PythonQtSlotFunction_Check(obj)) {
    static const char* argumentList[] ={"" , "const QUrl&" , "bool"};
    static const PythonQtMethodInfo* methodInfo = PythonQtMethodInfo::getCachedMethodInfoFromArgumentList(3, argumentList);
    void* args[3] = {NULL, (void*)&url, (void*)&valid};
    PyObject* result = PythonQtSignalTarget::call(obj, methodInfo, args, true);
    if (result) { Py_DECREF(result); } 
    Py_DECREF(obj);
    return;
  }
  Py_XDECREF(obj);
}
  
}
void PythonQtShell_PlaylistManagerInterface::UnregisterSpecialPlaylistType(SpecialPlaylistType*  type)
{
if (_wrapper) {
  PyObject* obj = PyObject_GetAttrString((PyObject*)_wrapper, "UnregisterSpecialPlaylistType");
  PyErr_Clear();
  if (obj && !PythonQtSlotFunction_Check(obj)) {
    static const char* argumentList[] ={"" , "SpecialPlaylistType*"};
    static const PythonQtMethodInfo* methodInfo = PythonQtMethodInfo::getCachedMethodInfoFromArgumentList(2, argumentList);
    void* args[2] = {NULL, (void*)&type};
    PyObject* result = PythonQtSignalTarget::call(obj, methodInfo, args, true);
    if (result) { Py_DECREF(result); } 
    Py_DECREF(obj);
    return;
  }
  Py_XDECREF(obj);
}
  
}
Playlist*  PythonQtShell_PlaylistManagerInterface::active() const
{
if (_wrapper) {
  PyObject* obj = PyObject_GetAttrString((PyObject*)_wrapper, "active");
  PyErr_Clear();
  if (obj && !PythonQtSlotFunction_Check(obj)) {
    static const char* argumentList[] ={"Playlist*"};
    static const PythonQtMethodInfo* methodInfo = PythonQtMethodInfo::getCachedMethodInfoFromArgumentList(1, argumentList);
      Playlist* returnValue = 0;
    void* args[1] = {NULL};
    PyObject* result = PythonQtSignalTarget::call(obj, methodInfo, args, true);
    if (result) {
      args[0] = PythonQtConv::ConvertPythonToQt(methodInfo->parameters().at(0), result, false, NULL, &returnValue);
      if (args[0]!=&returnValue) {
        if (args[0]==NULL) {
          PythonQt::priv()->handleVirtualOverloadReturnError("active", methodInfo, result);
        } else {
          returnValue = *((Playlist**)args[0]);
        }
      }
    }
    if (result) { Py_DECREF(result); } 
    Py_DECREF(obj);
    return returnValue;
  }
  Py_XDECREF(obj);
}
  return 0;
}
int  PythonQtShell_PlaylistManagerInterface::active_id() const
{
if (_wrapper) {
  PyObject* obj = PyObject_GetAttrString((PyObject*)_wrapper, "active_id");
  PyErr_Clear();
  if (obj && !PythonQtSlotFunction_Check(obj)) {
    static const char* argumentList[] ={"int"};
    static const PythonQtMethodInfo* methodInfo = PythonQtMethodInfo::getCachedMethodInfoFromArgumentList(1, argumentList);
      int returnValue = 0;
    void* args[1] = {NULL};
    PyObject* result = PythonQtSignalTarget::call(obj, methodInfo, args, true);
    if (result) {
      args[0] = PythonQtConv::ConvertPythonToQt(methodInfo->parameters().at(0), result, false, NULL, &returnValue);
      if (args[0]!=&returnValue) {
        if (args[0]==NULL) {
          PythonQt::priv()->handleVirtualOverloadReturnError("active_id", methodInfo, result);
        } else {
          returnValue = *((int*)args[0]);
        }
      }
    }
    if (result) { Py_DECREF(result); } 
    Py_DECREF(obj);
    return returnValue;
  }
  Py_XDECREF(obj);
}
  return int();
}
QItemSelection  PythonQtShell_PlaylistManagerInterface::active_selection() const
{
if (_wrapper) {
  PyObject* obj = PyObject_GetAttrString((PyObject*)_wrapper, "active_selection");
  PyErr_Clear();
  if (obj && !PythonQtSlotFunction_Check(obj)) {
    static const char* argumentList[] ={"QItemSelection"};
    static const PythonQtMethodInfo* methodInfo = PythonQtMethodInfo::getCachedMethodInfoFromArgumentList(1, argumentList);
      QItemSelection returnValue;
    void* args[1] = {NULL};
    PyObject* result = PythonQtSignalTarget::call(obj, methodInfo, args, true);
    if (result) {
      args[0] = PythonQtConv::ConvertPythonToQt(methodInfo->parameters().at(0), result, false, NULL, &returnValue);
      if (args[0]!=&returnValue) {
        if (args[0]==NULL) {
          PythonQt::priv()->handleVirtualOverloadReturnError("active_selection", methodInfo, result);
        } else {
          returnValue = *((QItemSelection*)args[0]);
        }
      }
    }
    if (result) { Py_DECREF(result); } 
    Py_DECREF(obj);
    return returnValue;
  }
  Py_XDECREF(obj);
}
  return QItemSelection();
}
void PythonQtShell_PlaylistManagerInterface::childEvent(QChildEvent*  arg__1)
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
  Py_XDECREF(obj);
}
  PlaylistManagerInterface::childEvent(arg__1);
}
Playlist*  PythonQtShell_PlaylistManagerInterface::current() const
{
if (_wrapper) {
  PyObject* obj = PyObject_GetAttrString((PyObject*)_wrapper, "current");
  PyErr_Clear();
  if (obj && !PythonQtSlotFunction_Check(obj)) {
    static const char* argumentList[] ={"Playlist*"};
    static const PythonQtMethodInfo* methodInfo = PythonQtMethodInfo::getCachedMethodInfoFromArgumentList(1, argumentList);
      Playlist* returnValue = 0;
    void* args[1] = {NULL};
    PyObject* result = PythonQtSignalTarget::call(obj, methodInfo, args, true);
    if (result) {
      args[0] = PythonQtConv::ConvertPythonToQt(methodInfo->parameters().at(0), result, false, NULL, &returnValue);
      if (args[0]!=&returnValue) {
        if (args[0]==NULL) {
          PythonQt::priv()->handleVirtualOverloadReturnError("current", methodInfo, result);
        } else {
          returnValue = *((Playlist**)args[0]);
        }
      }
    }
    if (result) { Py_DECREF(result); } 
    Py_DECREF(obj);
    return returnValue;
  }
  Py_XDECREF(obj);
}
  return 0;
}
int  PythonQtShell_PlaylistManagerInterface::current_id() const
{
if (_wrapper) {
  PyObject* obj = PyObject_GetAttrString((PyObject*)_wrapper, "current_id");
  PyErr_Clear();
  if (obj && !PythonQtSlotFunction_Check(obj)) {
    static const char* argumentList[] ={"int"};
    static const PythonQtMethodInfo* methodInfo = PythonQtMethodInfo::getCachedMethodInfoFromArgumentList(1, argumentList);
      int returnValue = 0;
    void* args[1] = {NULL};
    PyObject* result = PythonQtSignalTarget::call(obj, methodInfo, args, true);
    if (result) {
      args[0] = PythonQtConv::ConvertPythonToQt(methodInfo->parameters().at(0), result, false, NULL, &returnValue);
      if (args[0]!=&returnValue) {
        if (args[0]==NULL) {
          PythonQt::priv()->handleVirtualOverloadReturnError("current_id", methodInfo, result);
        } else {
          returnValue = *((int*)args[0]);
        }
      }
    }
    if (result) { Py_DECREF(result); } 
    Py_DECREF(obj);
    return returnValue;
  }
  Py_XDECREF(obj);
}
  return int();
}
QItemSelection  PythonQtShell_PlaylistManagerInterface::current_selection() const
{
if (_wrapper) {
  PyObject* obj = PyObject_GetAttrString((PyObject*)_wrapper, "current_selection");
  PyErr_Clear();
  if (obj && !PythonQtSlotFunction_Check(obj)) {
    static const char* argumentList[] ={"QItemSelection"};
    static const PythonQtMethodInfo* methodInfo = PythonQtMethodInfo::getCachedMethodInfoFromArgumentList(1, argumentList);
      QItemSelection returnValue;
    void* args[1] = {NULL};
    PyObject* result = PythonQtSignalTarget::call(obj, methodInfo, args, true);
    if (result) {
      args[0] = PythonQtConv::ConvertPythonToQt(methodInfo->parameters().at(0), result, false, NULL, &returnValue);
      if (args[0]!=&returnValue) {
        if (args[0]==NULL) {
          PythonQt::priv()->handleVirtualOverloadReturnError("current_selection", methodInfo, result);
        } else {
          returnValue = *((QItemSelection*)args[0]);
        }
      }
    }
    if (result) { Py_DECREF(result); } 
    Py_DECREF(obj);
    return returnValue;
  }
  Py_XDECREF(obj);
}
  return QItemSelection();
}
void PythonQtShell_PlaylistManagerInterface::customEvent(QEvent*  arg__1)
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
  Py_XDECREF(obj);
}
  PlaylistManagerInterface::customEvent(arg__1);
}
bool  PythonQtShell_PlaylistManagerInterface::event(QEvent*  arg__1)
{
if (_wrapper) {
  PyObject* obj = PyObject_GetAttrString((PyObject*)_wrapper, "event");
  PyErr_Clear();
  if (obj && !PythonQtSlotFunction_Check(obj)) {
    static const char* argumentList[] ={"bool" , "QEvent*"};
    static const PythonQtMethodInfo* methodInfo = PythonQtMethodInfo::getCachedMethodInfoFromArgumentList(2, argumentList);
      bool returnValue = 0;
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
  Py_XDECREF(obj);
}
  return PlaylistManagerInterface::event(arg__1);
}
bool  PythonQtShell_PlaylistManagerInterface::eventFilter(QObject*  arg__1, QEvent*  arg__2)
{
if (_wrapper) {
  PyObject* obj = PyObject_GetAttrString((PyObject*)_wrapper, "eventFilter");
  PyErr_Clear();
  if (obj && !PythonQtSlotFunction_Check(obj)) {
    static const char* argumentList[] ={"bool" , "QObject*" , "QEvent*"};
    static const PythonQtMethodInfo* methodInfo = PythonQtMethodInfo::getCachedMethodInfoFromArgumentList(3, argumentList);
      bool returnValue = 0;
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
  Py_XDECREF(obj);
}
  return PlaylistManagerInterface::eventFilter(arg__1, arg__2);
}
LibraryBackend*  PythonQtShell_PlaylistManagerInterface::library_backend() const
{
if (_wrapper) {
  PyObject* obj = PyObject_GetAttrString((PyObject*)_wrapper, "library_backend");
  PyErr_Clear();
  if (obj && !PythonQtSlotFunction_Check(obj)) {
    static const char* argumentList[] ={"LibraryBackend*"};
    static const PythonQtMethodInfo* methodInfo = PythonQtMethodInfo::getCachedMethodInfoFromArgumentList(1, argumentList);
      LibraryBackend* returnValue = 0;
    void* args[1] = {NULL};
    PyObject* result = PythonQtSignalTarget::call(obj, methodInfo, args, true);
    if (result) {
      args[0] = PythonQtConv::ConvertPythonToQt(methodInfo->parameters().at(0), result, false, NULL, &returnValue);
      if (args[0]!=&returnValue) {
        if (args[0]==NULL) {
          PythonQt::priv()->handleVirtualOverloadReturnError("library_backend", methodInfo, result);
        } else {
          returnValue = *((LibraryBackend**)args[0]);
        }
      }
    }
    if (result) { Py_DECREF(result); } 
    Py_DECREF(obj);
    return returnValue;
  }
  Py_XDECREF(obj);
}
  return 0;
}
QString  PythonQtShell_PlaylistManagerInterface::name(int  index) const
{
if (_wrapper) {
  PyObject* obj = PyObject_GetAttrString((PyObject*)_wrapper, "name");
  PyErr_Clear();
  if (obj && !PythonQtSlotFunction_Check(obj)) {
    static const char* argumentList[] ={"QString" , "int"};
    static const PythonQtMethodInfo* methodInfo = PythonQtMethodInfo::getCachedMethodInfoFromArgumentList(2, argumentList);
      QString returnValue;
    void* args[2] = {NULL, (void*)&index};
    PyObject* result = PythonQtSignalTarget::call(obj, methodInfo, args, true);
    if (result) {
      args[0] = PythonQtConv::ConvertPythonToQt(methodInfo->parameters().at(0), result, false, NULL, &returnValue);
      if (args[0]!=&returnValue) {
        if (args[0]==NULL) {
          PythonQt::priv()->handleVirtualOverloadReturnError("name", methodInfo, result);
        } else {
          returnValue = *((QString*)args[0]);
        }
      }
    }
    if (result) { Py_DECREF(result); } 
    Py_DECREF(obj);
    return returnValue;
  }
  Py_XDECREF(obj);
}
  return QString();
}
PlaylistParser*  PythonQtShell_PlaylistManagerInterface::parser() const
{
if (_wrapper) {
  PyObject* obj = PyObject_GetAttrString((PyObject*)_wrapper, "parser");
  PyErr_Clear();
  if (obj && !PythonQtSlotFunction_Check(obj)) {
    static const char* argumentList[] ={"PlaylistParser*"};
    static const PythonQtMethodInfo* methodInfo = PythonQtMethodInfo::getCachedMethodInfoFromArgumentList(1, argumentList);
      PlaylistParser* returnValue = 0;
    void* args[1] = {NULL};
    PyObject* result = PythonQtSignalTarget::call(obj, methodInfo, args, true);
    if (result) {
      args[0] = PythonQtConv::ConvertPythonToQt(methodInfo->parameters().at(0), result, false, NULL, &returnValue);
      if (args[0]!=&returnValue) {
        if (args[0]==NULL) {
          PythonQt::priv()->handleVirtualOverloadReturnError("parser", methodInfo, result);
        } else {
          returnValue = *((PlaylistParser**)args[0]);
        }
      }
    }
    if (result) { Py_DECREF(result); } 
    Py_DECREF(obj);
    return returnValue;
  }
  Py_XDECREF(obj);
}
  return 0;
}
Playlist*  PythonQtShell_PlaylistManagerInterface::playlist(int  id) const
{
if (_wrapper) {
  PyObject* obj = PyObject_GetAttrString((PyObject*)_wrapper, "playlist");
  PyErr_Clear();
  if (obj && !PythonQtSlotFunction_Check(obj)) {
    static const char* argumentList[] ={"Playlist*" , "int"};
    static const PythonQtMethodInfo* methodInfo = PythonQtMethodInfo::getCachedMethodInfoFromArgumentList(2, argumentList);
      Playlist* returnValue = 0;
    void* args[2] = {NULL, (void*)&id};
    PyObject* result = PythonQtSignalTarget::call(obj, methodInfo, args, true);
    if (result) {
      args[0] = PythonQtConv::ConvertPythonToQt(methodInfo->parameters().at(0), result, false, NULL, &returnValue);
      if (args[0]!=&returnValue) {
        if (args[0]==NULL) {
          PythonQt::priv()->handleVirtualOverloadReturnError("playlist", methodInfo, result);
        } else {
          returnValue = *((Playlist**)args[0]);
        }
      }
    }
    if (result) { Py_DECREF(result); } 
    Py_DECREF(obj);
    return returnValue;
  }
  Py_XDECREF(obj);
}
  return 0;
}
PlaylistBackend*  PythonQtShell_PlaylistManagerInterface::playlist_backend() const
{
if (_wrapper) {
  PyObject* obj = PyObject_GetAttrString((PyObject*)_wrapper, "playlist_backend");
  PyErr_Clear();
  if (obj && !PythonQtSlotFunction_Check(obj)) {
    static const char* argumentList[] ={"PlaylistBackend*"};
    static const PythonQtMethodInfo* methodInfo = PythonQtMethodInfo::getCachedMethodInfoFromArgumentList(1, argumentList);
      PlaylistBackend* returnValue = 0;
    void* args[1] = {NULL};
    PyObject* result = PythonQtSignalTarget::call(obj, methodInfo, args, true);
    if (result) {
      args[0] = PythonQtConv::ConvertPythonToQt(methodInfo->parameters().at(0), result, false, NULL, &returnValue);
      if (args[0]!=&returnValue) {
        if (args[0]==NULL) {
          PythonQt::priv()->handleVirtualOverloadReturnError("playlist_backend", methodInfo, result);
        } else {
          returnValue = *((PlaylistBackend**)args[0]);
        }
      }
    }
    if (result) { Py_DECREF(result); } 
    Py_DECREF(obj);
    return returnValue;
  }
  Py_XDECREF(obj);
}
  return 0;
}
PlaylistContainer*  PythonQtShell_PlaylistManagerInterface::playlist_container() const
{
if (_wrapper) {
  PyObject* obj = PyObject_GetAttrString((PyObject*)_wrapper, "playlist_container");
  PyErr_Clear();
  if (obj && !PythonQtSlotFunction_Check(obj)) {
    static const char* argumentList[] ={"PlaylistContainer*"};
    static const PythonQtMethodInfo* methodInfo = PythonQtMethodInfo::getCachedMethodInfoFromArgumentList(1, argumentList);
      PlaylistContainer* returnValue = 0;
    void* args[1] = {NULL};
    PyObject* result = PythonQtSignalTarget::call(obj, methodInfo, args, true);
    if (result) {
      args[0] = PythonQtConv::ConvertPythonToQt(methodInfo->parameters().at(0), result, false, NULL, &returnValue);
      if (args[0]!=&returnValue) {
        if (args[0]==NULL) {
          PythonQt::priv()->handleVirtualOverloadReturnError("playlist_container", methodInfo, result);
        } else {
          returnValue = *((PlaylistContainer**)args[0]);
        }
      }
    }
    if (result) { Py_DECREF(result); } 
    Py_DECREF(obj);
    return returnValue;
  }
  Py_XDECREF(obj);
}
  return 0;
}
QItemSelection  PythonQtShell_PlaylistManagerInterface::selection(int  id) const
{
if (_wrapper) {
  PyObject* obj = PyObject_GetAttrString((PyObject*)_wrapper, "selection");
  PyErr_Clear();
  if (obj && !PythonQtSlotFunction_Check(obj)) {
    static const char* argumentList[] ={"QItemSelection" , "int"};
    static const PythonQtMethodInfo* methodInfo = PythonQtMethodInfo::getCachedMethodInfoFromArgumentList(2, argumentList);
      QItemSelection returnValue;
    void* args[2] = {NULL, (void*)&id};
    PyObject* result = PythonQtSignalTarget::call(obj, methodInfo, args, true);
    if (result) {
      args[0] = PythonQtConv::ConvertPythonToQt(methodInfo->parameters().at(0), result, false, NULL, &returnValue);
      if (args[0]!=&returnValue) {
        if (args[0]==NULL) {
          PythonQt::priv()->handleVirtualOverloadReturnError("selection", methodInfo, result);
        } else {
          returnValue = *((QItemSelection*)args[0]);
        }
      }
    }
    if (result) { Py_DECREF(result); } 
    Py_DECREF(obj);
    return returnValue;
  }
  Py_XDECREF(obj);
}
  return QItemSelection();
}
PlaylistSequence*  PythonQtShell_PlaylistManagerInterface::sequence() const
{
if (_wrapper) {
  PyObject* obj = PyObject_GetAttrString((PyObject*)_wrapper, "sequence");
  PyErr_Clear();
  if (obj && !PythonQtSlotFunction_Check(obj)) {
    static const char* argumentList[] ={"PlaylistSequence*"};
    static const PythonQtMethodInfo* methodInfo = PythonQtMethodInfo::getCachedMethodInfoFromArgumentList(1, argumentList);
      PlaylistSequence* returnValue = 0;
    void* args[1] = {NULL};
    PyObject* result = PythonQtSignalTarget::call(obj, methodInfo, args, true);
    if (result) {
      args[0] = PythonQtConv::ConvertPythonToQt(methodInfo->parameters().at(0), result, false, NULL, &returnValue);
      if (args[0]!=&returnValue) {
        if (args[0]==NULL) {
          PythonQt::priv()->handleVirtualOverloadReturnError("sequence", methodInfo, result);
        } else {
          returnValue = *((PlaylistSequence**)args[0]);
        }
      }
    }
    if (result) { Py_DECREF(result); } 
    Py_DECREF(obj);
    return returnValue;
  }
  Py_XDECREF(obj);
}
  return 0;
}
TaskManager*  PythonQtShell_PlaylistManagerInterface::task_manager() const
{
if (_wrapper) {
  PyObject* obj = PyObject_GetAttrString((PyObject*)_wrapper, "task_manager");
  PyErr_Clear();
  if (obj && !PythonQtSlotFunction_Check(obj)) {
    static const char* argumentList[] ={"TaskManager*"};
    static const PythonQtMethodInfo* methodInfo = PythonQtMethodInfo::getCachedMethodInfoFromArgumentList(1, argumentList);
      TaskManager* returnValue = 0;
    void* args[1] = {NULL};
    PyObject* result = PythonQtSignalTarget::call(obj, methodInfo, args, true);
    if (result) {
      args[0] = PythonQtConv::ConvertPythonToQt(methodInfo->parameters().at(0), result, false, NULL, &returnValue);
      if (args[0]!=&returnValue) {
        if (args[0]==NULL) {
          PythonQt::priv()->handleVirtualOverloadReturnError("task_manager", methodInfo, result);
        } else {
          returnValue = *((TaskManager**)args[0]);
        }
      }
    }
    if (result) { Py_DECREF(result); } 
    Py_DECREF(obj);
    return returnValue;
  }
  Py_XDECREF(obj);
}
  return 0;
}
void PythonQtShell_PlaylistManagerInterface::timerEvent(QTimerEvent*  arg__1)
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
  Py_XDECREF(obj);
}
  PlaylistManagerInterface::timerEvent(arg__1);
}
PlaylistManagerInterface* PythonQtWrapper_PlaylistManagerInterface::new_PlaylistManagerInterface(QObject*  parent)
{ 
return new PythonQtShell_PlaylistManagerInterface(parent); }



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
  Py_XDECREF(obj);
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
  Py_XDECREF(obj);
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
      bool returnValue = 0;
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
  Py_XDECREF(obj);
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
      bool returnValue = 0;
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
  Py_XDECREF(obj);
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
  Py_XDECREF(obj);
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



QMenu*  PythonQtWrapper_PlaylistSequence::repeat_menu(PlaylistSequence* theWrappedObject) const
{
  return ( theWrappedObject->repeat_menu());
}

PlaylistSequence::RepeatMode  PythonQtWrapper_PlaylistSequence::repeat_mode(PlaylistSequence* theWrappedObject) const
{
  return ( theWrappedObject->repeat_mode());
}

QMenu*  PythonQtWrapper_PlaylistSequence::shuffle_menu(PlaylistSequence* theWrappedObject) const
{
  return ( theWrappedObject->shuffle_menu());
}

PlaylistSequence::ShuffleMode  PythonQtWrapper_PlaylistSequence::shuffle_mode(PlaylistSequence* theWrappedObject) const
{
  return ( theWrappedObject->shuffle_mode());
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

PlayerInterface*  PythonQtWrapper_RadioModel::player(RadioModel* theWrappedObject) const
{
  return ( theWrappedObject->player());
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
      QStandardItem* returnValue = 0;
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
  Py_XDECREF(obj);
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
  Py_XDECREF(obj);
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
      QWidget* returnValue = 0;
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
  Py_XDECREF(obj);
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
  Py_XDECREF(obj);
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
  Py_XDECREF(obj);
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
  Py_XDECREF(obj);
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
  Py_XDECREF(obj);
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
  Py_XDECREF(obj);
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
  Py_XDECREF(obj);
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
  Py_XDECREF(obj);
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
      bool returnValue = 0;
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
  Py_XDECREF(obj);
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
      bool returnValue = 0;
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
  Py_XDECREF(obj);
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
  Py_XDECREF(obj);
}
  RadioService::timerEvent(arg__1);
}
RadioService* PythonQtWrapper_RadioService::new_RadioService(const QString&  name, RadioModel*  model, QObject*  parent)
{ 
return new PythonQtShell_RadioService(name, model, parent); }

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

bool  PythonQtWrapper_Song::is_cdda(Song* theWrappedObject) const
{
  return ( theWrappedObject->is_cdda());
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



QList<Song >  PythonQtShell_SongInsertVetoListener::AboutToInsertSongs(const QList<Song >&  old_songs, const QList<Song >&  new_songs)
{
if (_wrapper) {
  PyObject* obj = PyObject_GetAttrString((PyObject*)_wrapper, "AboutToInsertSongs");
  PyErr_Clear();
  if (obj && !PythonQtSlotFunction_Check(obj)) {
    static const char* argumentList[] ={"QList<Song >" , "const QList<Song >&" , "const QList<Song >&"};
    static const PythonQtMethodInfo* methodInfo = PythonQtMethodInfo::getCachedMethodInfoFromArgumentList(3, argumentList);
      QList<Song > returnValue;
    void* args[3] = {NULL, (void*)&old_songs, (void*)&new_songs};
    PyObject* result = PythonQtSignalTarget::call(obj, methodInfo, args, true);
    if (result) {
      args[0] = PythonQtConv::ConvertPythonToQt(methodInfo->parameters().at(0), result, false, NULL, &returnValue);
      if (args[0]!=&returnValue) {
        if (args[0]==NULL) {
          PythonQt::priv()->handleVirtualOverloadReturnError("AboutToInsertSongs", methodInfo, result);
        } else {
          returnValue = *((QList<Song >*)args[0]);
        }
      }
    }
    if (result) { Py_DECREF(result); } 
    Py_DECREF(obj);
    return returnValue;
  }
  Py_XDECREF(obj);
}
  return QList<Song >();
}
void PythonQtShell_SongInsertVetoListener::childEvent(QChildEvent*  arg__1)
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
  Py_XDECREF(obj);
}
  SongInsertVetoListener::childEvent(arg__1);
}
void PythonQtShell_SongInsertVetoListener::customEvent(QEvent*  arg__1)
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
  Py_XDECREF(obj);
}
  SongInsertVetoListener::customEvent(arg__1);
}
bool  PythonQtShell_SongInsertVetoListener::event(QEvent*  arg__1)
{
if (_wrapper) {
  PyObject* obj = PyObject_GetAttrString((PyObject*)_wrapper, "event");
  PyErr_Clear();
  if (obj && !PythonQtSlotFunction_Check(obj)) {
    static const char* argumentList[] ={"bool" , "QEvent*"};
    static const PythonQtMethodInfo* methodInfo = PythonQtMethodInfo::getCachedMethodInfoFromArgumentList(2, argumentList);
      bool returnValue = 0;
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
  Py_XDECREF(obj);
}
  return SongInsertVetoListener::event(arg__1);
}
bool  PythonQtShell_SongInsertVetoListener::eventFilter(QObject*  arg__1, QEvent*  arg__2)
{
if (_wrapper) {
  PyObject* obj = PyObject_GetAttrString((PyObject*)_wrapper, "eventFilter");
  PyErr_Clear();
  if (obj && !PythonQtSlotFunction_Check(obj)) {
    static const char* argumentList[] ={"bool" , "QObject*" , "QEvent*"};
    static const PythonQtMethodInfo* methodInfo = PythonQtMethodInfo::getCachedMethodInfoFromArgumentList(3, argumentList);
      bool returnValue = 0;
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
  Py_XDECREF(obj);
}
  return SongInsertVetoListener::eventFilter(arg__1, arg__2);
}
void PythonQtShell_SongInsertVetoListener::timerEvent(QTimerEvent*  arg__1)
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
  Py_XDECREF(obj);
}
  SongInsertVetoListener::timerEvent(arg__1);
}
SongInsertVetoListener* PythonQtWrapper_SongInsertVetoListener::new_SongInsertVetoListener()
{ 
return new PythonQtShell_SongInsertVetoListener(); }



void PythonQtShell_SpecialPlaylistType::DidYouMeanClicked(const QString&  text, Playlist*  playlist)
{
if (_wrapper) {
  PyObject* obj = PyObject_GetAttrString((PyObject*)_wrapper, "DidYouMeanClicked");
  PyErr_Clear();
  if (obj && !PythonQtSlotFunction_Check(obj)) {
    static const char* argumentList[] ={"" , "const QString&" , "Playlist*"};
    static const PythonQtMethodInfo* methodInfo = PythonQtMethodInfo::getCachedMethodInfoFromArgumentList(3, argumentList);
    void* args[3] = {NULL, (void*)&text, (void*)&playlist};
    PyObject* result = PythonQtSignalTarget::call(obj, methodInfo, args, true);
    if (result) { Py_DECREF(result); } 
    Py_DECREF(obj);
    return;
  }
  Py_XDECREF(obj);
}
  SpecialPlaylistType::DidYouMeanClicked(text, playlist);
}
void PythonQtShell_SpecialPlaylistType::Search(const QString&  text, Playlist*  playlist)
{
if (_wrapper) {
  PyObject* obj = PyObject_GetAttrString((PyObject*)_wrapper, "Search");
  PyErr_Clear();
  if (obj && !PythonQtSlotFunction_Check(obj)) {
    static const char* argumentList[] ={"" , "const QString&" , "Playlist*"};
    static const PythonQtMethodInfo* methodInfo = PythonQtMethodInfo::getCachedMethodInfoFromArgumentList(3, argumentList);
    void* args[3] = {NULL, (void*)&text, (void*)&playlist};
    PyObject* result = PythonQtSignalTarget::call(obj, methodInfo, args, true);
    if (result) { Py_DECREF(result); } 
    Py_DECREF(obj);
    return;
  }
  Py_XDECREF(obj);
}
  SpecialPlaylistType::Search(text, playlist);
}
QString  PythonQtShell_SpecialPlaylistType::empty_playlist_text(Playlist*  playlist) const
{
if (_wrapper) {
  PyObject* obj = PyObject_GetAttrString((PyObject*)_wrapper, "empty_playlist_text");
  PyErr_Clear();
  if (obj && !PythonQtSlotFunction_Check(obj)) {
    static const char* argumentList[] ={"QString" , "Playlist*"};
    static const PythonQtMethodInfo* methodInfo = PythonQtMethodInfo::getCachedMethodInfoFromArgumentList(2, argumentList);
      QString returnValue;
    void* args[2] = {NULL, (void*)&playlist};
    PyObject* result = PythonQtSignalTarget::call(obj, methodInfo, args, true);
    if (result) {
      args[0] = PythonQtConv::ConvertPythonToQt(methodInfo->parameters().at(0), result, false, NULL, &returnValue);
      if (args[0]!=&returnValue) {
        if (args[0]==NULL) {
          PythonQt::priv()->handleVirtualOverloadReturnError("empty_playlist_text", methodInfo, result);
        } else {
          returnValue = *((QString*)args[0]);
        }
      }
    }
    if (result) { Py_DECREF(result); } 
    Py_DECREF(obj);
    return returnValue;
  }
  Py_XDECREF(obj);
}
  return SpecialPlaylistType::empty_playlist_text(playlist);
}
bool  PythonQtShell_SpecialPlaylistType::has_special_search_behaviour(Playlist*  playlist) const
{
if (_wrapper) {
  PyObject* obj = PyObject_GetAttrString((PyObject*)_wrapper, "has_special_search_behaviour");
  PyErr_Clear();
  if (obj && !PythonQtSlotFunction_Check(obj)) {
    static const char* argumentList[] ={"bool" , "Playlist*"};
    static const PythonQtMethodInfo* methodInfo = PythonQtMethodInfo::getCachedMethodInfoFromArgumentList(2, argumentList);
      bool returnValue = 0;
    void* args[2] = {NULL, (void*)&playlist};
    PyObject* result = PythonQtSignalTarget::call(obj, methodInfo, args, true);
    if (result) {
      args[0] = PythonQtConv::ConvertPythonToQt(methodInfo->parameters().at(0), result, false, NULL, &returnValue);
      if (args[0]!=&returnValue) {
        if (args[0]==NULL) {
          PythonQt::priv()->handleVirtualOverloadReturnError("has_special_search_behaviour", methodInfo, result);
        } else {
          returnValue = *((bool*)args[0]);
        }
      }
    }
    if (result) { Py_DECREF(result); } 
    Py_DECREF(obj);
    return returnValue;
  }
  Py_XDECREF(obj);
}
  return SpecialPlaylistType::has_special_search_behaviour(playlist);
}
QIcon  PythonQtShell_SpecialPlaylistType::icon(Playlist*  playlist) const
{
if (_wrapper) {
  PyObject* obj = PyObject_GetAttrString((PyObject*)_wrapper, "icon");
  PyErr_Clear();
  if (obj && !PythonQtSlotFunction_Check(obj)) {
    static const char* argumentList[] ={"QIcon" , "Playlist*"};
    static const PythonQtMethodInfo* methodInfo = PythonQtMethodInfo::getCachedMethodInfoFromArgumentList(2, argumentList);
      QIcon returnValue;
    void* args[2] = {NULL, (void*)&playlist};
    PyObject* result = PythonQtSignalTarget::call(obj, methodInfo, args, true);
    if (result) {
      args[0] = PythonQtConv::ConvertPythonToQt(methodInfo->parameters().at(0), result, false, NULL, &returnValue);
      if (args[0]!=&returnValue) {
        if (args[0]==NULL) {
          PythonQt::priv()->handleVirtualOverloadReturnError("icon", methodInfo, result);
        } else {
          returnValue = *((QIcon*)args[0]);
        }
      }
    }
    if (result) { Py_DECREF(result); } 
    Py_DECREF(obj);
    return returnValue;
  }
  Py_XDECREF(obj);
}
  return SpecialPlaylistType::icon(playlist);
}
QString  PythonQtShell_SpecialPlaylistType::name() const
{
if (_wrapper) {
  PyObject* obj = PyObject_GetAttrString((PyObject*)_wrapper, "name");
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
          PythonQt::priv()->handleVirtualOverloadReturnError("name", methodInfo, result);
        } else {
          returnValue = *((QString*)args[0]);
        }
      }
    }
    if (result) { Py_DECREF(result); } 
    Py_DECREF(obj);
    return returnValue;
  }
  Py_XDECREF(obj);
}
  return QString();
}
QString  PythonQtShell_SpecialPlaylistType::search_hint_text(Playlist*  playlist) const
{
if (_wrapper) {
  PyObject* obj = PyObject_GetAttrString((PyObject*)_wrapper, "search_hint_text");
  PyErr_Clear();
  if (obj && !PythonQtSlotFunction_Check(obj)) {
    static const char* argumentList[] ={"QString" , "Playlist*"};
    static const PythonQtMethodInfo* methodInfo = PythonQtMethodInfo::getCachedMethodInfoFromArgumentList(2, argumentList);
      QString returnValue;
    void* args[2] = {NULL, (void*)&playlist};
    PyObject* result = PythonQtSignalTarget::call(obj, methodInfo, args, true);
    if (result) {
      args[0] = PythonQtConv::ConvertPythonToQt(methodInfo->parameters().at(0), result, false, NULL, &returnValue);
      if (args[0]!=&returnValue) {
        if (args[0]==NULL) {
          PythonQt::priv()->handleVirtualOverloadReturnError("search_hint_text", methodInfo, result);
        } else {
          returnValue = *((QString*)args[0]);
        }
      }
    }
    if (result) { Py_DECREF(result); } 
    Py_DECREF(obj);
    return returnValue;
  }
  Py_XDECREF(obj);
}
  return SpecialPlaylistType::search_hint_text(playlist);
}
SpecialPlaylistType* PythonQtWrapper_SpecialPlaylistType::new_SpecialPlaylistType()
{ 
return new PythonQtShell_SpecialPlaylistType(); }

void PythonQtWrapper_SpecialPlaylistType::DidYouMeanClicked(SpecialPlaylistType* theWrappedObject, const QString&  text, Playlist*  playlist)
{
  ( ((PythonQtPublicPromoter_SpecialPlaylistType*)theWrappedObject)->promoted_DidYouMeanClicked(text, playlist));
}

void PythonQtWrapper_SpecialPlaylistType::Search(SpecialPlaylistType* theWrappedObject, const QString&  text, Playlist*  playlist)
{
  ( ((PythonQtPublicPromoter_SpecialPlaylistType*)theWrappedObject)->promoted_Search(text, playlist));
}

QString  PythonQtWrapper_SpecialPlaylistType::empty_playlist_text(SpecialPlaylistType* theWrappedObject, Playlist*  playlist) const
{
  return ( ((PythonQtPublicPromoter_SpecialPlaylistType*)theWrappedObject)->promoted_empty_playlist_text(playlist));
}

bool  PythonQtWrapper_SpecialPlaylistType::has_special_search_behaviour(SpecialPlaylistType* theWrappedObject, Playlist*  playlist) const
{
  return ( ((PythonQtPublicPromoter_SpecialPlaylistType*)theWrappedObject)->promoted_has_special_search_behaviour(playlist));
}

QIcon  PythonQtWrapper_SpecialPlaylistType::icon(SpecialPlaylistType* theWrappedObject, Playlist*  playlist) const
{
  return ( ((PythonQtPublicPromoter_SpecialPlaylistType*)theWrappedObject)->promoted_icon(playlist));
}

QString  PythonQtWrapper_SpecialPlaylistType::search_hint_text(SpecialPlaylistType* theWrappedObject, Playlist*  playlist) const
{
  return ( ((PythonQtPublicPromoter_SpecialPlaylistType*)theWrappedObject)->promoted_search_hint_text(playlist));
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
  Py_XDECREF(obj);
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
  Py_XDECREF(obj);
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
      bool returnValue = 0;
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
  Py_XDECREF(obj);
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
      bool returnValue = 0;
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
  Py_XDECREF(obj);
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
  Py_XDECREF(obj);
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


