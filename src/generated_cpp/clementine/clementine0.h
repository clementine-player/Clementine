#include <PythonQt.h>
#include <QObject>
#include <QVariant>
#include <qabstractitemmodel.h>
#include <qbytearray.h>
#include <qcoreevent.h>
#include <qdatastream.h>
#include <qlist.h>
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


