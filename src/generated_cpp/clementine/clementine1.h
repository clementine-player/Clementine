#include <PythonQt.h>
#include <QObject>
#include <QVariant>
#include <network.h>
#include <qabstractnetworkcache.h>
#include <qbytearray.h>
#include <qcoreevent.h>
#include <qiodevice.h>
#include <qlist.h>
#include <qobject.h>
#include <qurl.h>
#include <taskmanager.h>
#include <urlhandler.h>



class PythonQtShell_TaskManager_Task : public TaskManager_Task
{
public:
    PythonQtShell_TaskManager_Task():TaskManager_Task(),_wrapper(NULL) {};


  PythonQtInstanceWrapper* _wrapper; 
};

class PythonQtWrapper_TaskManager_Task : public QObject
{ Q_OBJECT
public:
public slots:
TaskManager_Task* new_TaskManager_Task();
TaskManager_Task* new_TaskManager_Task(const TaskManager_Task& other) {
PythonQtShell_TaskManager_Task* a = new PythonQtShell_TaskManager_Task();
*((TaskManager_Task*)a) = other;
return a; }
void delete_TaskManager_Task(TaskManager_Task* obj) { delete obj; } 
void py_set_progress_max(TaskManager_Task* theWrappedObject, int  progress_max){ theWrappedObject->progress_max = progress_max; }
int  py_get_progress_max(TaskManager_Task* theWrappedObject){ return theWrappedObject->progress_max; }
void py_set_progress(TaskManager_Task* theWrappedObject, int  progress){ theWrappedObject->progress = progress; }
int  py_get_progress(TaskManager_Task* theWrappedObject){ return theWrappedObject->progress; }
void py_set_id(TaskManager_Task* theWrappedObject, int  id){ theWrappedObject->id = id; }
int  py_get_id(TaskManager_Task* theWrappedObject){ return theWrappedObject->id; }
void py_set_name(TaskManager_Task* theWrappedObject, QString  name){ theWrappedObject->name = name; }
QString  py_get_name(TaskManager_Task* theWrappedObject){ return theWrappedObject->name; }
void py_set_blocks_library_scans(TaskManager_Task* theWrappedObject, bool  blocks_library_scans){ theWrappedObject->blocks_library_scans = blocks_library_scans; }
bool  py_get_blocks_library_scans(TaskManager_Task* theWrappedObject){ return theWrappedObject->blocks_library_scans; }
};





class PythonQtShell_ThreadSafeNetworkDiskCache : public ThreadSafeNetworkDiskCache
{
public:
    PythonQtShell_ThreadSafeNetworkDiskCache(QObject*  parent):ThreadSafeNetworkDiskCache(parent),_wrapper(NULL) {};

virtual qint64  cacheSize() const;
virtual void childEvent(QChildEvent*  arg__1);
virtual void clear();
virtual void customEvent(QEvent*  arg__1);
virtual QIODevice*  data(const QUrl&  url);
virtual bool  event(QEvent*  arg__1);
virtual bool  eventFilter(QObject*  arg__1, QEvent*  arg__2);
virtual void insert(QIODevice*  device);
virtual QNetworkCacheMetaData  metaData(const QUrl&  url);
virtual QIODevice*  prepare(const QNetworkCacheMetaData&  metaData);
virtual bool  remove(const QUrl&  url);
virtual void timerEvent(QTimerEvent*  arg__1);
virtual void updateMetaData(const QNetworkCacheMetaData&  metaData);

  PythonQtInstanceWrapper* _wrapper; 
};

class PythonQtPublicPromoter_ThreadSafeNetworkDiskCache : public ThreadSafeNetworkDiskCache
{ public:
inline qint64  promoted_cacheSize() const { return ThreadSafeNetworkDiskCache::cacheSize(); }
inline void promoted_clear() { ThreadSafeNetworkDiskCache::clear(); }
inline QIODevice*  promoted_data(const QUrl&  url) { return ThreadSafeNetworkDiskCache::data(url); }
inline void promoted_insert(QIODevice*  device) { ThreadSafeNetworkDiskCache::insert(device); }
inline QNetworkCacheMetaData  promoted_metaData(const QUrl&  url) { return ThreadSafeNetworkDiskCache::metaData(url); }
inline QIODevice*  promoted_prepare(const QNetworkCacheMetaData&  metaData) { return ThreadSafeNetworkDiskCache::prepare(metaData); }
inline bool  promoted_remove(const QUrl&  url) { return ThreadSafeNetworkDiskCache::remove(url); }
inline void promoted_updateMetaData(const QNetworkCacheMetaData&  metaData) { ThreadSafeNetworkDiskCache::updateMetaData(metaData); }
};

class PythonQtWrapper_ThreadSafeNetworkDiskCache : public QObject
{ Q_OBJECT
public:
public slots:
ThreadSafeNetworkDiskCache* new_ThreadSafeNetworkDiskCache(QObject*  parent);
void delete_ThreadSafeNetworkDiskCache(ThreadSafeNetworkDiskCache* obj) { delete obj; } 
   qint64  cacheSize(ThreadSafeNetworkDiskCache* theWrappedObject) const;
   void clear(ThreadSafeNetworkDiskCache* theWrappedObject);
   QIODevice*  data(ThreadSafeNetworkDiskCache* theWrappedObject, const QUrl&  url);
   void insert(ThreadSafeNetworkDiskCache* theWrappedObject, QIODevice*  device);
   QNetworkCacheMetaData  metaData(ThreadSafeNetworkDiskCache* theWrappedObject, const QUrl&  url);
   QIODevice*  prepare(ThreadSafeNetworkDiskCache* theWrappedObject, const QNetworkCacheMetaData&  metaData);
   bool  remove(ThreadSafeNetworkDiskCache* theWrappedObject, const QUrl&  url);
   void updateMetaData(ThreadSafeNetworkDiskCache* theWrappedObject, const QNetworkCacheMetaData&  metaData);
};





class PythonQtShell_UrlHandler : public UrlHandler
{
public:
    PythonQtShell_UrlHandler(QObject*  parent = 0):UrlHandler(parent),_wrapper(NULL) {};

virtual UrlHandler_LoadResult  LoadNext(const QUrl&  url);
virtual UrlHandler_LoadResult  StartLoading(const QUrl&  url);
virtual void childEvent(QChildEvent*  arg__1);
virtual void customEvent(QEvent*  arg__1);
virtual bool  event(QEvent*  arg__1);
virtual bool  eventFilter(QObject*  arg__1, QEvent*  arg__2);
virtual QString  scheme() const;
virtual void timerEvent(QTimerEvent*  arg__1);

  PythonQtInstanceWrapper* _wrapper; 
};

class PythonQtPublicPromoter_UrlHandler : public UrlHandler
{ public:
inline UrlHandler_LoadResult  promoted_LoadNext(const QUrl&  url) { return UrlHandler::LoadNext(url); }
inline UrlHandler_LoadResult  promoted_StartLoading(const QUrl&  url) { return UrlHandler::StartLoading(url); }
};

class PythonQtWrapper_UrlHandler : public QObject
{ Q_OBJECT
public:
public slots:
UrlHandler* new_UrlHandler(QObject*  parent = 0);
void delete_UrlHandler(UrlHandler* obj) { delete obj; } 
   UrlHandler_LoadResult  LoadNext(UrlHandler* theWrappedObject, const QUrl&  url);
   UrlHandler_LoadResult  StartLoading(UrlHandler* theWrappedObject, const QUrl&  url);
};





class PythonQtShell_UrlHandler_LoadResult : public UrlHandler_LoadResult
{
public:
    PythonQtShell_UrlHandler_LoadResult(const QUrl&  original_url = QUrl(), UrlHandler_LoadResult::Type  type = UrlHandler_LoadResult::NoMoreTracks, const QUrl&  media_url = QUrl()):UrlHandler_LoadResult(original_url, type, media_url),_wrapper(NULL) {};


  PythonQtInstanceWrapper* _wrapper; 
};

class PythonQtWrapper_UrlHandler_LoadResult : public QObject
{ Q_OBJECT
public:
Q_ENUMS(Type )
enum Type{
  NoMoreTracks = UrlHandler_LoadResult::NoMoreTracks,   WillLoadAsynchronously = UrlHandler_LoadResult::WillLoadAsynchronously,   TrackAvailable = UrlHandler_LoadResult::TrackAvailable};
public slots:
UrlHandler_LoadResult* new_UrlHandler_LoadResult(const QUrl&  original_url = QUrl(), UrlHandler_LoadResult::Type  type = UrlHandler_LoadResult::NoMoreTracks, const QUrl&  media_url = QUrl());
void delete_UrlHandler_LoadResult(UrlHandler_LoadResult* obj) { delete obj; } 
void py_set_original_url_(UrlHandler_LoadResult* theWrappedObject, QUrl  original_url_){ theWrappedObject->original_url_ = original_url_; }
QUrl  py_get_original_url_(UrlHandler_LoadResult* theWrappedObject){ return theWrappedObject->original_url_; }
void py_set_media_url_(UrlHandler_LoadResult* theWrappedObject, QUrl  media_url_){ theWrappedObject->media_url_ = media_url_; }
QUrl  py_get_media_url_(UrlHandler_LoadResult* theWrappedObject){ return theWrappedObject->media_url_; }
void py_set_type_(UrlHandler_LoadResult* theWrappedObject, UrlHandler_LoadResult::Type  type_){ theWrappedObject->type_ = type_; }
UrlHandler_LoadResult::Type  py_get_type_(UrlHandler_LoadResult* theWrappedObject){ return theWrappedObject->type_; }
};


