#include <PythonQt.h>
#include <QObject>
#include <QVariant>
#include <albumcoverfetcher.h>
#include <albumcoverfetchersearch.h>
#include <coverprovider.h>
#include <coverproviderfactory.h>
#include <coverproviders.h>
#include <network.h>
#include <qabstractitemmodel.h>
#include <qabstractnetworkcache.h>
#include <qbytearray.h>
#include <qcoreevent.h>
#include <qdatastream.h>
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
#include <urlhandler.h>



class PythonQtShell_AlbumCoverFetcherSearch : public AlbumCoverFetcherSearch
{
public:

virtual void childEvent(QChildEvent*  arg__1);
virtual void customEvent(QEvent*  arg__1);
virtual bool  event(QEvent*  arg__1);
virtual bool  eventFilter(QObject*  arg__1, QEvent*  arg__2);
virtual void timerEvent(QTimerEvent*  arg__1);

  PythonQtInstanceWrapper* _wrapper; 
};

class PythonQtWrapper_AlbumCoverFetcherSearch : public QObject
{ Q_OBJECT
public:
public slots:
void delete_AlbumCoverFetcherSearch(AlbumCoverFetcherSearch* obj) { delete obj; } 
   void Start(AlbumCoverFetcherSearch* theWrappedObject);
};





class PythonQtShell_CoverProvider : public CoverProvider
{
public:
    PythonQtShell_CoverProvider(const QString&  name, QObject*  parent = &CoverProviders::instance()):CoverProvider(name, parent),_wrapper(NULL) {};

virtual CoverSearchResults  ParseReply(QNetworkReply*  reply);
virtual QNetworkReply*  SendRequest(const QString&  query);
virtual void childEvent(QChildEvent*  arg__1);
virtual void customEvent(QEvent*  arg__1);
virtual bool  event(QEvent*  arg__1);
virtual bool  eventFilter(QObject*  arg__1, QEvent*  arg__2);
virtual void timerEvent(QTimerEvent*  arg__1);

  PythonQtInstanceWrapper* _wrapper; 
};

class PythonQtWrapper_CoverProvider : public QObject
{ Q_OBJECT
public:
public slots:
CoverProvider* new_CoverProvider(const QString&  name, QObject*  parent = &CoverProviders::instance());
void delete_CoverProvider(CoverProvider* obj) { delete obj; } 
   QString  name(CoverProvider* theWrappedObject) const;
};





class PythonQtShell_CoverProviderFactory : public CoverProviderFactory
{
public:
    PythonQtShell_CoverProviderFactory(QObject*  parent = &CoverProviders::instance()):CoverProviderFactory(parent),_wrapper(NULL) {};

virtual CoverProvider*  CreateCoverProvider(AlbumCoverFetcherSearch*  parent);
virtual void childEvent(QChildEvent*  arg__1);
virtual void customEvent(QEvent*  arg__1);
virtual bool  event(QEvent*  arg__1);
virtual bool  eventFilter(QObject*  arg__1, QEvent*  arg__2);
virtual void timerEvent(QTimerEvent*  arg__1);

  PythonQtInstanceWrapper* _wrapper; 
};

class PythonQtWrapper_CoverProviderFactory : public QObject
{ Q_OBJECT
public:
public slots:
CoverProviderFactory* new_CoverProviderFactory(QObject*  parent = &CoverProviders::instance());
void delete_CoverProviderFactory(CoverProviderFactory* obj) { delete obj; } 
};





class PythonQtWrapper_CoverProviders : public QObject
{ Q_OBJECT
public:
public slots:
void delete_CoverProviders(CoverProviders* obj) { delete obj; } 
   void AddProviderFactory(CoverProviders* theWrappedObject, CoverProviderFactory*  factory);
   bool  HasAnyProviderFactories(CoverProviders* theWrappedObject);
   QList<CoverProvider* >  List(CoverProviders* theWrappedObject, AlbumCoverFetcherSearch*  parent);
   CoverProviders*  static_CoverProviders_instance();
};





class PythonQtWrapper_CoverSearchResults : public QObject
{ Q_OBJECT
public:
public slots:
CoverSearchResults* new_CoverSearchResults();
CoverSearchResults* new_CoverSearchResults(const CoverSearchResults& other) {
CoverSearchResults* a = new CoverSearchResults();
*((CoverSearchResults*)a) = other;
return a; }
void delete_CoverSearchResults(CoverSearchResults* obj) { delete obj; } 
   void clear(CoverSearchResults* theWrappedObject);
   int  count(CoverSearchResults* theWrappedObject) const;
   void detachShared(CoverSearchResults* theWrappedObject);
   bool  empty(CoverSearchResults* theWrappedObject) const;
   bool  isEmpty(CoverSearchResults* theWrappedObject) const;
   int  length(CoverSearchResults* theWrappedObject) const;
   void move(CoverSearchResults* theWrappedObject, int  from, int  to);
   void pop_back(CoverSearchResults* theWrappedObject);
   void pop_front(CoverSearchResults* theWrappedObject);
   void removeAt(CoverSearchResults* theWrappedObject, int  i);
   void removeFirst(CoverSearchResults* theWrappedObject);
   void removeLast(CoverSearchResults* theWrappedObject);
   void reserve(CoverSearchResults* theWrappedObject, int  size);
   void setSharable(CoverSearchResults* theWrappedObject, bool  sharable);
   int  size(CoverSearchResults* theWrappedObject) const;
   void swap(CoverSearchResults* theWrappedObject, int  i, int  j);
};





class PythonQtShell_NetworkAccessManager : public NetworkAccessManager
{
public:
    PythonQtShell_NetworkAccessManager(QObject*  parent = 0):NetworkAccessManager(parent),_wrapper(NULL) {};

virtual void childEvent(QChildEvent*  arg__1);
virtual QNetworkReply*  createRequest(QNetworkAccessManager::Operation  op, const QNetworkRequest&  request, QIODevice*  outgoingData);
virtual void customEvent(QEvent*  arg__1);
virtual bool  event(QEvent*  arg__1);
virtual bool  eventFilter(QObject*  arg__1, QEvent*  arg__2);
virtual void timerEvent(QTimerEvent*  arg__1);

  PythonQtInstanceWrapper* _wrapper; 
};

class PythonQtPublicPromoter_NetworkAccessManager : public NetworkAccessManager
{ public:
inline QNetworkReply*  promoted_createRequest(QNetworkAccessManager::Operation  op, const QNetworkRequest&  request, QIODevice*  outgoingData) { return NetworkAccessManager::createRequest(op, request, outgoingData); }
};

class PythonQtWrapper_NetworkAccessManager : public QObject
{ Q_OBJECT
public:
public slots:
NetworkAccessManager* new_NetworkAccessManager(QObject*  parent = 0);
void delete_NetworkAccessManager(NetworkAccessManager* obj) { delete obj; } 
   QNetworkReply*  createRequest(NetworkAccessManager* theWrappedObject, QNetworkAccessManager::Operation  op, const QNetworkRequest&  request, QIODevice*  outgoingData);
};





class PythonQtShell_NetworkTimeouts : public NetworkTimeouts
{
public:
    PythonQtShell_NetworkTimeouts(int  timeout_msec, QObject*  parent = 0):NetworkTimeouts(timeout_msec, parent),_wrapper(NULL) {};

virtual void childEvent(QChildEvent*  arg__1);
virtual void customEvent(QEvent*  arg__1);
virtual bool  event(QEvent*  arg__1);
virtual bool  eventFilter(QObject*  arg__1, QEvent*  arg__2);
virtual void timerEvent(QTimerEvent*  arg__1);

  PythonQtInstanceWrapper* _wrapper; 
};

class PythonQtWrapper_NetworkTimeouts : public QObject
{ Q_OBJECT
public:
public slots:
NetworkTimeouts* new_NetworkTimeouts(int  timeout_msec, QObject*  parent = 0);
void delete_NetworkTimeouts(NetworkTimeouts* obj) { delete obj; } 
   void AddReply(NetworkTimeouts* theWrappedObject, QNetworkReply*  reply);
   void SetTimeout(NetworkTimeouts* theWrappedObject, int  msec);
};





class PythonQtPublicPromoter_RadioModel : public RadioModel
{ public:
inline Qt::ItemFlags  promoted_flags(const QModelIndex&  index) const { return RadioModel::flags(index); }
inline bool  promoted_hasChildren(const QModelIndex&  parent) const { return RadioModel::hasChildren(parent); }
inline QMimeData*  promoted_mimeData(const QList<QModelIndex >&  indexes) const { return RadioModel::mimeData(indexes); }
inline QStringList  promoted_mimeTypes() const { return RadioModel::mimeTypes(); }
inline int  promoted_rowCount(const QModelIndex&  parent) const { return RadioModel::rowCount(parent); }
};

class PythonQtWrapper_RadioModel : public QObject
{ Q_OBJECT
public:
Q_ENUMS(PlayBehaviour Role Type )
enum PlayBehaviour{
  PlayBehaviour_None = RadioModel::PlayBehaviour_None,   PlayBehaviour_UseSongLoader = RadioModel::PlayBehaviour_UseSongLoader,   PlayBehaviour_SingleItem = RadioModel::PlayBehaviour_SingleItem,   PlayBehaviour_DoubleClickAction = RadioModel::PlayBehaviour_DoubleClickAction};
enum Role{
  Role_Type = RadioModel::Role_Type,   Role_PlayBehaviour = RadioModel::Role_PlayBehaviour,   Role_Url = RadioModel::Role_Url,   Role_SongMetadata = RadioModel::Role_SongMetadata,   Role_CanLazyLoad = RadioModel::Role_CanLazyLoad,   Role_Service = RadioModel::Role_Service,   RoleCount = RadioModel::RoleCount};
enum Type{
  Type_Service = RadioModel::Type_Service,   TypeCount = RadioModel::TypeCount};
public slots:
void delete_RadioModel(RadioModel* obj) { delete obj; } 
   void AddService(RadioModel* theWrappedObject, RadioService*  service);
   bool  IsPlayable(RadioModel* theWrappedObject, const QModelIndex&  index) const;
   void ReloadSettings(RadioModel* theWrappedObject);
   void RemoveService(RadioModel* theWrappedObject, RadioService*  service);
   RadioService*  static_RadioModel_ServiceByName(const QString&  name);
   RadioService*  ServiceForIndex(RadioModel* theWrappedObject, const QModelIndex&  index) const;
   RadioService*  ServiceForItem(RadioModel* theWrappedObject, const QStandardItem*  item) const;
   void ShowContextMenu(RadioModel* theWrappedObject, const QModelIndex&  merged_model_index, const QPoint&  global_pos);
   Qt::ItemFlags  flags(RadioModel* theWrappedObject, const QModelIndex&  index) const;
   bool  hasChildren(RadioModel* theWrappedObject, const QModelIndex&  parent) const;
   QMimeData*  mimeData(RadioModel* theWrappedObject, const QList<QModelIndex >&  indexes) const;
   QStringList  mimeTypes(RadioModel* theWrappedObject) const;
   int  rowCount(RadioModel* theWrappedObject, const QModelIndex&  parent) const;
};





class PythonQtShell_RadioService : public RadioService
{
public:
    PythonQtShell_RadioService(const QString&  name, RadioModel*  model):RadioService(name, model),_wrapper(NULL) {};

virtual QStandardItem*  CreateRootItem();
virtual QModelIndex  GetCurrentIndex();
virtual QWidget*  HeaderWidget() const;
virtual QString  Icon();
virtual void ItemDoubleClicked(QStandardItem*  item);
virtual void LazyPopulate(QStandardItem*  parent);
virtual void ReloadSettings();
virtual void ShowContextMenu(const QModelIndex&  index, const QPoint&  global_pos);
virtual void childEvent(QChildEvent*  arg__1);
virtual void customEvent(QEvent*  arg__1);
virtual bool  event(QEvent*  arg__1);
virtual bool  eventFilter(QObject*  arg__1, QEvent*  arg__2);
virtual void timerEvent(QTimerEvent*  arg__1);

  PythonQtInstanceWrapper* _wrapper; 
};

class PythonQtPublicPromoter_RadioService : public RadioService
{ public:
inline QWidget*  promoted_HeaderWidget() const { return RadioService::HeaderWidget(); }
inline QString  promoted_Icon() { return RadioService::Icon(); }
inline void promoted_ItemDoubleClicked(QStandardItem*  item) { RadioService::ItemDoubleClicked(item); }
inline void promoted_ReloadSettings() { RadioService::ReloadSettings(); }
inline void promoted_ShowContextMenu(const QModelIndex&  index, const QPoint&  global_pos) { RadioService::ShowContextMenu(index, global_pos); }
};

class PythonQtWrapper_RadioService : public QObject
{ Q_OBJECT
public:
public slots:
RadioService* new_RadioService(const QString&  name, RadioModel*  model);
void delete_RadioService(RadioService* obj) { delete obj; } 
   QWidget*  HeaderWidget(RadioService* theWrappedObject) const;
   QString  Icon(RadioService* theWrappedObject);
   void ItemDoubleClicked(RadioService* theWrappedObject, QStandardItem*  item);
   void ReloadSettings(RadioService* theWrappedObject);
   void ShowContextMenu(RadioService* theWrappedObject, const QModelIndex&  index, const QPoint&  global_pos);
   RadioModel*  model(RadioService* theWrappedObject) const;
   QString  name(RadioService* theWrappedObject) const;
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

virtual UrlHandler::LoadResult  LoadNext(const QUrl&  url);
virtual UrlHandler::LoadResult  StartLoading(const QUrl&  url);
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
inline UrlHandler::LoadResult  promoted_LoadNext(const QUrl&  url) { return UrlHandler::LoadNext(url); }
inline UrlHandler::LoadResult  promoted_StartLoading(const QUrl&  url) { return UrlHandler::StartLoading(url); }
};

class PythonQtWrapper_UrlHandler : public QObject
{ Q_OBJECT
public:
public slots:
UrlHandler* new_UrlHandler(QObject*  parent = 0);
void delete_UrlHandler(UrlHandler* obj) { delete obj; } 
   UrlHandler::LoadResult  LoadNext(UrlHandler* theWrappedObject, const QUrl&  url);
   UrlHandler::LoadResult  StartLoading(UrlHandler* theWrappedObject, const QUrl&  url);
};





class PythonQtShell_UrlHandler_LoadResult : public UrlHandler::LoadResult
{
public:
    PythonQtShell_UrlHandler_LoadResult(const QUrl&  original_url = QUrl(), UrlHandler::LoadResult::Type  type = UrlHandler::LoadResult::NoMoreTracks, const QUrl&  media_url = QUrl()):UrlHandler::LoadResult(original_url, type, media_url),_wrapper(NULL) {};


  PythonQtInstanceWrapper* _wrapper; 
};

class PythonQtWrapper_UrlHandler_LoadResult : public QObject
{ Q_OBJECT
public:
Q_ENUMS(Type )
enum Type{
  NoMoreTracks = UrlHandler::LoadResult::NoMoreTracks,   WillLoadAsynchronously = UrlHandler::LoadResult::WillLoadAsynchronously,   TrackAvailable = UrlHandler::LoadResult::TrackAvailable};
public slots:
UrlHandler::LoadResult* new_UrlHandler_LoadResult(const QUrl&  original_url = QUrl(), UrlHandler::LoadResult::Type  type = UrlHandler::LoadResult::NoMoreTracks, const QUrl&  media_url = QUrl());
void delete_UrlHandler_LoadResult(UrlHandler::LoadResult* obj) { delete obj; } 
void py_set_original_url_(UrlHandler::LoadResult* theWrappedObject, QUrl  original_url_){ theWrappedObject->original_url_ = original_url_; }
QUrl  py_get_original_url_(UrlHandler::LoadResult* theWrappedObject){ return theWrappedObject->original_url_; }
void py_set_media_url_(UrlHandler::LoadResult* theWrappedObject, QUrl  media_url_){ theWrappedObject->media_url_ = media_url_; }
QUrl  py_get_media_url_(UrlHandler::LoadResult* theWrappedObject){ return theWrappedObject->media_url_; }
void py_set_type_(UrlHandler::LoadResult* theWrappedObject, UrlHandler::LoadResult::Type  type_){ theWrappedObject->type_ = type_; }
UrlHandler::LoadResult::Type  py_get_type_(UrlHandler::LoadResult* theWrappedObject){ return theWrappedObject->type_; }
};


