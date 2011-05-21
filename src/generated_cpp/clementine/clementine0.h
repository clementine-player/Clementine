#include <PythonQt.h>
#include <QObject>
#include <QVariant>
#include <albumcoverfetcher.h>
#include <albumcoverfetchersearch.h>
#include <coverprovider.h>
#include <coverproviderfactory.h>
#include <coverproviders.h>
#include <directory.h>
#include <librarybackend.h>
#include <libraryquery.h>
#include <network.h>
#include <player.h>
#include <playlistparser.h>
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
   void setSharable(CoverSearchResults* theWrappedObject, bool  sharable);
   int  size(CoverSearchResults* theWrappedObject) const;
   void swap(CoverSearchResults* theWrappedObject, int  i, int  j);
};





class PythonQtShell_Directory : public Directory
{
public:
    PythonQtShell_Directory():Directory(),_wrapper(NULL) {};


  PythonQtInstanceWrapper* _wrapper; 
};

class PythonQtWrapper_Directory : public QObject
{ Q_OBJECT
public:
public slots:
Directory* new_Directory();
Directory* new_Directory(const Directory& other) {
PythonQtShell_Directory* a = new PythonQtShell_Directory();
*((Directory*)a) = other;
return a; }
void delete_Directory(Directory* obj) { delete obj; } 
void py_set_path(Directory* theWrappedObject, QString  path){ theWrappedObject->path = path; }
QString  py_get_path(Directory* theWrappedObject){ return theWrappedObject->path; }
void py_set_id(Directory* theWrappedObject, int  id){ theWrappedObject->id = id; }
int  py_get_id(Directory* theWrappedObject){ return theWrappedObject->id; }
};





class PythonQtShell_LibraryBackend : public LibraryBackend
{
public:
    PythonQtShell_LibraryBackend(QObject*  parent = 0):LibraryBackend(parent),_wrapper(NULL) {};

virtual void AddDirectory(const QString&  path);
virtual void ChangeDirPath(int  id, const QString&  old_path, const QString&  new_path);
virtual bool  ExecQuery(LibraryQuery*  q);
virtual QList<Song >  FindSongsInDirectory(int  id);
virtual LibraryBackendInterface::Album  GetAlbumArt(const QString&  artist, const QString&  album);
virtual QList<LibraryBackendInterface::Album >  GetAlbumsByArtist(const QString&  artist, const QueryOptions&  opt = QueryOptions());
virtual QList<LibraryBackendInterface::Album >  GetAllAlbums(const QueryOptions&  opt = QueryOptions());
virtual QStringList  GetAllArtists(const QueryOptions&  opt = QueryOptions());
virtual QStringList  GetAllArtistsWithAlbums(const QueryOptions&  opt = QueryOptions());
virtual QList<Directory >  GetAllDirectories();
virtual QList<LibraryBackendInterface::Album >  GetCompilationAlbums(const QueryOptions&  opt = QueryOptions());
virtual QList<Song >  GetCompilationSongs(const QString&  album, const QueryOptions&  opt = QueryOptions());
virtual Song  GetSongById(int  id);
virtual Song  GetSongByUrl(const QUrl&  url, qint64  beginning = 0);
virtual QList<Song >  GetSongs(const QString&  artist, const QString&  album, const QueryOptions&  opt = QueryOptions());
virtual QList<Song >  GetSongsByUrl(const QUrl&  url);
virtual bool  HasCompilations(const QueryOptions&  opt = QueryOptions());
virtual void LoadDirectoriesAsync();
virtual void RemoveDirectory(const Directory&  dir);
virtual QList<Subdirectory >  SubdirsInDirectory(int  id);
virtual void UpdateManualAlbumArtAsync(const QString&  artist, const QString&  album, const QString&  art);
virtual void UpdateTotalSongCountAsync();
virtual void childEvent(QChildEvent*  arg__1);
virtual void customEvent(QEvent*  arg__1);
virtual bool  event(QEvent*  arg__1);
virtual bool  eventFilter(QObject*  arg__1, QEvent*  arg__2);
virtual void timerEvent(QTimerEvent*  arg__1);

  PythonQtInstanceWrapper* _wrapper; 
};

class PythonQtPublicPromoter_LibraryBackend : public LibraryBackend
{ public:
inline void promoted_AddDirectory(const QString&  path) { LibraryBackend::AddDirectory(path); }
inline void promoted_ChangeDirPath(int  id, const QString&  old_path, const QString&  new_path) { LibraryBackend::ChangeDirPath(id, old_path, new_path); }
inline bool  promoted_ExecQuery(LibraryQuery*  q) { return LibraryBackend::ExecQuery(q); }
inline QList<Song >  promoted_FindSongsInDirectory(int  id) { return LibraryBackend::FindSongsInDirectory(id); }
inline LibraryBackendInterface::Album  promoted_GetAlbumArt(const QString&  artist, const QString&  album) { return LibraryBackend::GetAlbumArt(artist, album); }
inline QList<LibraryBackendInterface::Album >  promoted_GetAlbumsByArtist(const QString&  artist, const QueryOptions&  opt = QueryOptions()) { return LibraryBackend::GetAlbumsByArtist(artist, opt); }
inline QList<LibraryBackendInterface::Album >  promoted_GetAllAlbums(const QueryOptions&  opt = QueryOptions()) { return LibraryBackend::GetAllAlbums(opt); }
inline QStringList  promoted_GetAllArtists(const QueryOptions&  opt = QueryOptions()) { return LibraryBackend::GetAllArtists(opt); }
inline QStringList  promoted_GetAllArtistsWithAlbums(const QueryOptions&  opt = QueryOptions()) { return LibraryBackend::GetAllArtistsWithAlbums(opt); }
inline QList<Directory >  promoted_GetAllDirectories() { return LibraryBackend::GetAllDirectories(); }
inline QList<LibraryBackendInterface::Album >  promoted_GetCompilationAlbums(const QueryOptions&  opt = QueryOptions()) { return LibraryBackend::GetCompilationAlbums(opt); }
inline QList<Song >  promoted_GetCompilationSongs(const QString&  album, const QueryOptions&  opt = QueryOptions()) { return LibraryBackend::GetCompilationSongs(album, opt); }
inline Song  promoted_GetSongById(int  id) { return LibraryBackend::GetSongById(id); }
inline Song  promoted_GetSongByUrl(const QUrl&  url, qint64  beginning = 0) { return LibraryBackend::GetSongByUrl(url, beginning); }
inline QList<Song >  promoted_GetSongs(const QString&  artist, const QString&  album, const QueryOptions&  opt = QueryOptions()) { return LibraryBackend::GetSongs(artist, album, opt); }
inline QList<Song >  promoted_GetSongsByUrl(const QUrl&  url) { return LibraryBackend::GetSongsByUrl(url); }
inline bool  promoted_HasCompilations(const QueryOptions&  opt = QueryOptions()) { return LibraryBackend::HasCompilations(opt); }
inline void promoted_LoadDirectoriesAsync() { LibraryBackend::LoadDirectoriesAsync(); }
inline void promoted_RemoveDirectory(const Directory&  dir) { LibraryBackend::RemoveDirectory(dir); }
inline QList<Subdirectory >  promoted_SubdirsInDirectory(int  id) { return LibraryBackend::SubdirsInDirectory(id); }
inline void promoted_UpdateManualAlbumArtAsync(const QString&  artist, const QString&  album, const QString&  art) { LibraryBackend::UpdateManualAlbumArtAsync(artist, album, art); }
inline void promoted_UpdateTotalSongCountAsync() { LibraryBackend::UpdateTotalSongCountAsync(); }
};

class PythonQtWrapper_LibraryBackend : public QObject
{ Q_OBJECT
public:
public slots:
LibraryBackend* new_LibraryBackend(QObject*  parent = 0);
void delete_LibraryBackend(LibraryBackend* obj) { delete obj; } 
   void AddDirectory(LibraryBackend* theWrappedObject, const QString&  path);
   void ChangeDirPath(LibraryBackend* theWrappedObject, int  id, const QString&  old_path, const QString&  new_path);
   void DeleteAll(LibraryBackend* theWrappedObject);
   bool  ExecQuery(LibraryBackend* theWrappedObject, LibraryQuery*  q);
   QList<Song >  FindSongsInDirectory(LibraryBackend* theWrappedObject, int  id);
   LibraryBackendInterface::Album  GetAlbumArt(LibraryBackend* theWrappedObject, const QString&  artist, const QString&  album);
   QList<LibraryBackendInterface::Album >  GetAlbumsByArtist(LibraryBackend* theWrappedObject, const QString&  artist, const QueryOptions&  opt = QueryOptions());
   QStringList  GetAll(LibraryBackend* theWrappedObject, const QString&  column, const QueryOptions&  opt = QueryOptions());
   QList<LibraryBackendInterface::Album >  GetAllAlbums(LibraryBackend* theWrappedObject, const QueryOptions&  opt = QueryOptions());
   QStringList  GetAllArtists(LibraryBackend* theWrappedObject, const QueryOptions&  opt = QueryOptions());
   QStringList  GetAllArtistsWithAlbums(LibraryBackend* theWrappedObject, const QueryOptions&  opt = QueryOptions());
   QList<Directory >  GetAllDirectories(LibraryBackend* theWrappedObject);
   QList<LibraryBackendInterface::Album >  GetCompilationAlbums(LibraryBackend* theWrappedObject, const QueryOptions&  opt = QueryOptions());
   QList<Song >  GetCompilationSongs(LibraryBackend* theWrappedObject, const QString&  album, const QueryOptions&  opt = QueryOptions());
   Song  GetSongById(LibraryBackend* theWrappedObject, int  id);
   Song  GetSongByUrl(LibraryBackend* theWrappedObject, const QUrl&  url, qint64  beginning = 0);
   QList<Song >  GetSongs(LibraryBackend* theWrappedObject, const QString&  artist, const QString&  album, const QueryOptions&  opt = QueryOptions());
   QList<Song >  GetSongsByForeignId(LibraryBackend* theWrappedObject, const QStringList&  ids, const QString&  table, const QString&  column);
   QList<Song >  GetSongsById(LibraryBackend* theWrappedObject, const QList<int >&  ids);
   QList<Song >  GetSongsById(LibraryBackend* theWrappedObject, const QStringList&  ids);
   QList<Song >  GetSongsByUrl(LibraryBackend* theWrappedObject, const QUrl&  url);
   bool  HasCompilations(LibraryBackend* theWrappedObject, const QueryOptions&  opt = QueryOptions());
   void IncrementPlayCountAsync(LibraryBackend* theWrappedObject, int  id);
   void IncrementSkipCountAsync(LibraryBackend* theWrappedObject, int  id, float  progress);
   void LoadDirectoriesAsync(LibraryBackend* theWrappedObject);
   void RemoveDirectory(LibraryBackend* theWrappedObject, const Directory&  dir);
   void ResetStatisticsAsync(LibraryBackend* theWrappedObject, int  id);
   QList<Subdirectory >  SubdirsInDirectory(LibraryBackend* theWrappedObject, int  id);
   void UpdateManualAlbumArtAsync(LibraryBackend* theWrappedObject, const QString&  artist, const QString&  album, const QString&  art);
   void UpdateSongRatingAsync(LibraryBackend* theWrappedObject, int  id, float  rating);
   void UpdateTotalSongCountAsync(LibraryBackend* theWrappedObject);
   QString  dirs_table(LibraryBackend* theWrappedObject) const;
   QString  songs_table(LibraryBackend* theWrappedObject) const;
   QString  subdirs_table(LibraryBackend* theWrappedObject) const;
};





class PythonQtShell_LibraryBackendInterface : public LibraryBackendInterface
{
public:
    PythonQtShell_LibraryBackendInterface(QObject*  parent = 0):LibraryBackendInterface(parent),_wrapper(NULL) {};

virtual void AddDirectory(const QString&  path);
virtual void ChangeDirPath(int  id, const QString&  old_path, const QString&  new_path);
virtual bool  ExecQuery(LibraryQuery*  q);
virtual QList<Song >  FindSongsInDirectory(int  id);
virtual LibraryBackendInterface::Album  GetAlbumArt(const QString&  artist, const QString&  album);
virtual QList<LibraryBackendInterface::Album >  GetAlbumsByArtist(const QString&  artist, const QueryOptions&  opt = QueryOptions());
virtual QList<LibraryBackendInterface::Album >  GetAllAlbums(const QueryOptions&  opt = QueryOptions());
virtual QStringList  GetAllArtists(const QueryOptions&  opt = QueryOptions());
virtual QStringList  GetAllArtistsWithAlbums(const QueryOptions&  opt = QueryOptions());
virtual QList<Directory >  GetAllDirectories();
virtual QList<LibraryBackendInterface::Album >  GetCompilationAlbums(const QueryOptions&  opt = QueryOptions());
virtual QList<Song >  GetCompilationSongs(const QString&  album, const QueryOptions&  opt = QueryOptions());
virtual Song  GetSongById(int  id);
virtual Song  GetSongByUrl(const QUrl&  url, qint64  beginning = 0);
virtual QList<Song >  GetSongs(const QString&  artist, const QString&  album, const QueryOptions&  opt = QueryOptions());
virtual QList<Song >  GetSongsByUrl(const QUrl&  url);
virtual bool  HasCompilations(const QueryOptions&  opt = QueryOptions());
virtual void LoadDirectoriesAsync();
virtual void RemoveDirectory(const Directory&  dir);
virtual QList<Subdirectory >  SubdirsInDirectory(int  id);
virtual void UpdateManualAlbumArtAsync(const QString&  artist, const QString&  album, const QString&  art);
virtual void UpdateTotalSongCountAsync();
virtual void childEvent(QChildEvent*  arg__1);
virtual void customEvent(QEvent*  arg__1);
virtual bool  event(QEvent*  arg__1);
virtual bool  eventFilter(QObject*  arg__1, QEvent*  arg__2);
virtual void timerEvent(QTimerEvent*  arg__1);

  PythonQtInstanceWrapper* _wrapper; 
};

class PythonQtWrapper_LibraryBackendInterface : public QObject
{ Q_OBJECT
public:
public slots:
LibraryBackendInterface* new_LibraryBackendInterface(QObject*  parent = 0);
void delete_LibraryBackendInterface(LibraryBackendInterface* obj) { delete obj; } 
};





class PythonQtShell_LibraryBackendInterface_Album : public LibraryBackendInterface::Album
{
public:
    PythonQtShell_LibraryBackendInterface_Album():LibraryBackendInterface::Album(),_wrapper(NULL) {};
    PythonQtShell_LibraryBackendInterface_Album(const QString&  _artist, const QString&  _album_name, const QString&  _art_automatic, const QString&  _art_manual, const QUrl&  _first_url):LibraryBackendInterface::Album(_artist, _album_name, _art_automatic, _art_manual, _first_url),_wrapper(NULL) {};


  PythonQtInstanceWrapper* _wrapper; 
};

class PythonQtWrapper_LibraryBackendInterface_Album : public QObject
{ Q_OBJECT
public:
public slots:
LibraryBackendInterface::Album* new_LibraryBackendInterface_Album();
LibraryBackendInterface::Album* new_LibraryBackendInterface_Album(const QString&  _artist, const QString&  _album_name, const QString&  _art_automatic, const QString&  _art_manual, const QUrl&  _first_url);
LibraryBackendInterface::Album* new_LibraryBackendInterface_Album(const LibraryBackendInterface::Album& other) {
PythonQtShell_LibraryBackendInterface_Album* a = new PythonQtShell_LibraryBackendInterface_Album();
*((LibraryBackendInterface::Album*)a) = other;
return a; }
void delete_LibraryBackendInterface_Album(LibraryBackendInterface::Album* obj) { delete obj; } 
void py_set_art_automatic(LibraryBackendInterface::Album* theWrappedObject, QString  art_automatic){ theWrappedObject->art_automatic = art_automatic; }
QString  py_get_art_automatic(LibraryBackendInterface::Album* theWrappedObject){ return theWrappedObject->art_automatic; }
void py_set_artist(LibraryBackendInterface::Album* theWrappedObject, QString  artist){ theWrappedObject->artist = artist; }
QString  py_get_artist(LibraryBackendInterface::Album* theWrappedObject){ return theWrappedObject->artist; }
void py_set_album_name(LibraryBackendInterface::Album* theWrappedObject, QString  album_name){ theWrappedObject->album_name = album_name; }
QString  py_get_album_name(LibraryBackendInterface::Album* theWrappedObject){ return theWrappedObject->album_name; }
void py_set_first_url(LibraryBackendInterface::Album* theWrappedObject, QUrl  first_url){ theWrappedObject->first_url = first_url; }
QUrl  py_get_first_url(LibraryBackendInterface::Album* theWrappedObject){ return theWrappedObject->first_url; }
void py_set_art_manual(LibraryBackendInterface::Album* theWrappedObject, QString  art_manual){ theWrappedObject->art_manual = art_manual; }
QString  py_get_art_manual(LibraryBackendInterface::Album* theWrappedObject){ return theWrappedObject->art_manual; }
};





class PythonQtWrapper_LibraryQuery : public QObject
{ Q_OBJECT
public:
public slots:
LibraryQuery* new_LibraryQuery(const QueryOptions&  options = QueryOptions());
void delete_LibraryQuery(LibraryQuery* obj) { delete obj; } 
   void AddCompilationRequirement(LibraryQuery* theWrappedObject, bool  compilation);
   void AddWhere(LibraryQuery* theWrappedObject, const QString&  column, const QVariant&  value, const QString&  op = "=");
   bool  Next(LibraryQuery* theWrappedObject);
   void SetColumnSpec(LibraryQuery* theWrappedObject, const QString&  spec);
   void SetIncludeUnavailable(LibraryQuery* theWrappedObject, bool  include_unavailable);
   void SetLimit(LibraryQuery* theWrappedObject, int  limit);
   void SetOrderBy(LibraryQuery* theWrappedObject, const QString&  order_by);
   QVariant  Value(LibraryQuery* theWrappedObject, int  column) const;
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





class PythonQtWrapper_Player : public QObject
{ Q_OBJECT
public:
public slots:
void delete_Player(Player* obj) { delete obj; } 
   Engine::State  GetState(Player* theWrappedObject) const;
   int  GetVolume(Player* theWrappedObject) const;
   void Init(Player* theWrappedObject);
   void RegisterUrlHandler(Player* theWrappedObject, UrlHandler*  handler);
   void UnregisterUrlHandler(Player* theWrappedObject, UrlHandler*  handler);
   Engine::Base*  engine(Player* theWrappedObject) const;
};





class PythonQtShell_PlaylistParser : public PlaylistParser
{
public:
    PythonQtShell_PlaylistParser(LibraryBackendInterface*  library, QObject*  parent = 0):PlaylistParser(library, parent),_wrapper(NULL) {};

virtual void childEvent(QChildEvent*  arg__1);
virtual void customEvent(QEvent*  arg__1);
virtual bool  event(QEvent*  arg__1);
virtual bool  eventFilter(QObject*  arg__1, QEvent*  arg__2);
virtual void timerEvent(QTimerEvent*  arg__1);

  PythonQtInstanceWrapper* _wrapper; 
};

class PythonQtWrapper_PlaylistParser : public QObject
{ Q_OBJECT
public:
public slots:
PlaylistParser* new_PlaylistParser(LibraryBackendInterface*  library, QObject*  parent = 0);
void delete_PlaylistParser(PlaylistParser* obj) { delete obj; } 
   QList<Song >  LoadFromDevice(PlaylistParser* theWrappedObject, QIODevice*  device, const QString&  path_hint = QString(), const QDir&  dir_hint = QDir()) const;
   QList<Song >  LoadFromFile(PlaylistParser* theWrappedObject, const QString&  filename) const;
   void Save(PlaylistParser* theWrappedObject, const QList<Song >&  songs, const QString&  filename) const;
   QString  default_extension(PlaylistParser* theWrappedObject) const;
   QString  default_filter(PlaylistParser* theWrappedObject) const;
   QStringList  file_extensions(PlaylistParser* theWrappedObject) const;
   QString  filters(PlaylistParser* theWrappedObject) const;
};





class PythonQtWrapper_QueryOptions : public QObject
{ Q_OBJECT
public:
Q_ENUMS(QueryMode )
enum QueryMode{
  QueryMode_All = QueryOptions::QueryMode_All,   QueryMode_Duplicates = QueryOptions::QueryMode_Duplicates,   QueryMode_Untagged = QueryOptions::QueryMode_Untagged};
public slots:
QueryOptions* new_QueryOptions();
QueryOptions* new_QueryOptions(const QueryOptions& other) {
QueryOptions* a = new QueryOptions();
*((QueryOptions*)a) = other;
return a; }
void delete_QueryOptions(QueryOptions* obj) { delete obj; } 
   bool  Matches(QueryOptions* theWrappedObject, const Song&  song) const;
   QString  filter(QueryOptions* theWrappedObject) const;
   int  max_age(QueryOptions* theWrappedObject) const;
   QueryOptions::QueryMode  query_mode(QueryOptions* theWrappedObject) const;
   void set_filter(QueryOptions* theWrappedObject, const QString&  filter);
   void set_max_age(QueryOptions* theWrappedObject, int  max_age);
   void set_query_mode(QueryOptions* theWrappedObject, QueryOptions::QueryMode  query_mode);
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
   TaskManager*  task_manager(RadioModel* theWrappedObject) const;
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





class PythonQtShell_Song : public Song
{
public:
    PythonQtShell_Song():Song(),_wrapper(NULL) {};
    PythonQtShell_Song(const Song&  other):Song(other),_wrapper(NULL) {};


  PythonQtInstanceWrapper* _wrapper; 
};

class PythonQtWrapper_Song : public QObject
{ Q_OBJECT
public:
Q_ENUMS(FileType )
enum FileType{
  Type_Unknown = Song::Type_Unknown,   Type_Asf = Song::Type_Asf,   Type_Flac = Song::Type_Flac,   Type_Mp4 = Song::Type_Mp4,   Type_Mpc = Song::Type_Mpc,   Type_Mpeg = Song::Type_Mpeg,   Type_OggFlac = Song::Type_OggFlac,   Type_OggSpeex = Song::Type_OggSpeex,   Type_OggVorbis = Song::Type_OggVorbis,   Type_Aiff = Song::Type_Aiff,   Type_Wav = Song::Type_Wav,   Type_TrueAudio = Song::Type_TrueAudio,   Type_Stream = Song::Type_Stream};
public slots:
Song* new_Song();
Song* new_Song(const Song&  other);
void delete_Song(Song* obj) { delete obj; } 
   QString  static_Song_Decode(const QString&  tag, const QTextCodec*  codec = NULL);
   bool  HasProperMediaFile(Song* theWrappedObject) const;
   void Init(Song* theWrappedObject, const QString&  title, const QString&  artist, const QString&  album, qint64  beginning, qint64  end);
   void Init(Song* theWrappedObject, const QString&  title, const QString&  artist, const QString&  album, qint64  length_nanosec);
   void InitFromFile(Song* theWrappedObject, const QString&  filename, int  directory_id);
   void InitFromFilePartial(Song* theWrappedObject, const QString&  filename);
   bool  IsEditable(Song* theWrappedObject) const;
   bool  IsMetadataEqual(Song* theWrappedObject, const Song&  other) const;
   bool  IsOnSameAlbum(Song* theWrappedObject, const Song&  other) const;
   QString  static_Song_JoinSpec(const QString&  table);
   QImage  static_Song_LoadEmbeddedArt(const QString&  filename);
   QString  PrettyLength(Song* theWrappedObject) const;
   QString  PrettyTitle(Song* theWrappedObject) const;
   QString  PrettyTitleWithArtist(Song* theWrappedObject) const;
   QString  PrettyYear(Song* theWrappedObject) const;
   bool  Save(Song* theWrappedObject) const;
   QString  TextForFiletype(Song* theWrappedObject) const;
   QString  static_Song_TextForFiletype(Song::FileType  type);
   QString  TitleWithCompilationArtist(Song* theWrappedObject) const;
   const QString*  album(Song* theWrappedObject) const;
   const QString*  albumartist(Song* theWrappedObject) const;
   const QString*  art_automatic(Song* theWrappedObject) const;
   const QString*  art_manual(Song* theWrappedObject) const;
   const QString*  artist(Song* theWrappedObject) const;
   const QString*  basefilename(Song* theWrappedObject) const;
   qint64  beginning_nanosec(Song* theWrappedObject) const;
   int  bitrate(Song* theWrappedObject) const;
   float  bpm(Song* theWrappedObject) const;
   const QString*  comment(Song* theWrappedObject) const;
   const QString*  composer(Song* theWrappedObject) const;
   uint  ctime(Song* theWrappedObject) const;
   const QString*  cue_path(Song* theWrappedObject) const;
   int  directory_id(Song* theWrappedObject) const;
   int  disc(Song* theWrappedObject) const;
   qint64  end_nanosec(Song* theWrappedObject) const;
   int  filesize(Song* theWrappedObject) const;
   Song::FileType  filetype(Song* theWrappedObject) const;
   const QString*  genre(Song* theWrappedObject) const;
   bool  has_cue(Song* theWrappedObject) const;
   bool  has_embedded_cover(Song* theWrappedObject) const;
   bool  has_manually_unset_cover(Song* theWrappedObject) const;
   int  id(Song* theWrappedObject) const;
   const QImage*  image(Song* theWrappedObject) const;
   bool  is_compilation(Song* theWrappedObject) const;
   bool  is_stream(Song* theWrappedObject) const;
   bool  is_unavailable(Song* theWrappedObject) const;
   bool  is_valid(Song* theWrappedObject) const;
   int  lastplayed(Song* theWrappedObject) const;
   qint64  length_nanosec(Song* theWrappedObject) const;
   void manually_unset_cover(Song* theWrappedObject);
   uint  mtime(Song* theWrappedObject) const;
   bool  __eq__(Song* theWrappedObject, const Song&  other) const;
   int  playcount(Song* theWrappedObject) const;
   float  rating(Song* theWrappedObject) const;
   int  samplerate(Song* theWrappedObject) const;
   int  score(Song* theWrappedObject) const;
   void set_album(Song* theWrappedObject, const QString&  v);
   void set_albumartist(Song* theWrappedObject, const QString&  v);
   void set_art_automatic(Song* theWrappedObject, const QString&  v);
   void set_art_manual(Song* theWrappedObject, const QString&  v);
   void set_artist(Song* theWrappedObject, const QString&  v);
   void set_basefilename(Song* theWrappedObject, const QString&  v);
   void set_beginning_nanosec(Song* theWrappedObject, qint64  v);
   void set_bitrate(Song* theWrappedObject, int  v);
   void set_bpm(Song* theWrappedObject, float  v);
   void set_comment(Song* theWrappedObject, const QString&  v);
   void set_compilation(Song* theWrappedObject, bool  v);
   void set_composer(Song* theWrappedObject, const QString&  v);
   void set_ctime(Song* theWrappedObject, int  v);
   void set_cue_path(Song* theWrappedObject, const QString&  v);
   void set_directory_id(Song* theWrappedObject, int  v);
   void set_disc(Song* theWrappedObject, int  v);
   void set_embedded_cover(Song* theWrappedObject);
   void set_end_nanosec(Song* theWrappedObject, qint64  v);
   void set_filesize(Song* theWrappedObject, int  v);
   void set_filetype(Song* theWrappedObject, Song::FileType  v);
   void set_forced_compilation_off(Song* theWrappedObject, bool  v);
   void set_forced_compilation_on(Song* theWrappedObject, bool  v);
   void set_genre(Song* theWrappedObject, const QString&  v);
   void set_genre_id3(Song* theWrappedObject, int  id);
   void set_id(Song* theWrappedObject, int  id);
   void set_image(Song* theWrappedObject, const QImage&  i);
   void set_lastplayed(Song* theWrappedObject, int  v);
   void set_length_nanosec(Song* theWrappedObject, qint64  v);
   void set_mtime(Song* theWrappedObject, int  v);
   void set_playcount(Song* theWrappedObject, int  v);
   void set_rating(Song* theWrappedObject, float  v);
   void set_sampler(Song* theWrappedObject, bool  v);
   void set_samplerate(Song* theWrappedObject, int  v);
   void set_score(Song* theWrappedObject, int  v);
   void set_skipcount(Song* theWrappedObject, int  v);
   void set_title(Song* theWrappedObject, const QString&  v);
   void set_track(Song* theWrappedObject, int  v);
   void set_unavailable(Song* theWrappedObject, bool  v);
   void set_url(Song* theWrappedObject, const QUrl&  v);
   void set_valid(Song* theWrappedObject, bool  v);
   void set_year(Song* theWrappedObject, int  v);
   int  skipcount(Song* theWrappedObject) const;
   const QString*  title(Song* theWrappedObject) const;
   int  track(Song* theWrappedObject) const;
   const QUrl*  url(Song* theWrappedObject) const;
   int  year(Song* theWrappedObject) const;
};





class PythonQtShell_Subdirectory : public Subdirectory
{
public:
    PythonQtShell_Subdirectory():Subdirectory(),_wrapper(NULL) {};


  PythonQtInstanceWrapper* _wrapper; 
};

class PythonQtWrapper_Subdirectory : public QObject
{ Q_OBJECT
public:
public slots:
Subdirectory* new_Subdirectory();
Subdirectory* new_Subdirectory(const Subdirectory& other) {
PythonQtShell_Subdirectory* a = new PythonQtShell_Subdirectory();
*((Subdirectory*)a) = other;
return a; }
void delete_Subdirectory(Subdirectory* obj) { delete obj; } 
void py_set_mtime(Subdirectory* theWrappedObject, uint  mtime){ theWrappedObject->mtime = mtime; }
uint  py_get_mtime(Subdirectory* theWrappedObject){ return theWrappedObject->mtime; }
void py_set_path(Subdirectory* theWrappedObject, QString  path){ theWrappedObject->path = path; }
QString  py_get_path(Subdirectory* theWrappedObject){ return theWrappedObject->path; }
void py_set_directory_id(Subdirectory* theWrappedObject, int  directory_id){ theWrappedObject->directory_id = directory_id; }
int  py_get_directory_id(Subdirectory* theWrappedObject){ return theWrappedObject->directory_id; }
};





class PythonQtShell_TaskManager : public TaskManager
{
public:
    PythonQtShell_TaskManager(QObject*  parent = 0):TaskManager(parent),_wrapper(NULL) {};

virtual void childEvent(QChildEvent*  arg__1);
virtual void customEvent(QEvent*  arg__1);
virtual bool  event(QEvent*  arg__1);
virtual bool  eventFilter(QObject*  arg__1, QEvent*  arg__2);
virtual void timerEvent(QTimerEvent*  arg__1);

  PythonQtInstanceWrapper* _wrapper; 
};

class PythonQtWrapper_TaskManager : public QObject
{ Q_OBJECT
public:
public slots:
TaskManager* new_TaskManager(QObject*  parent = 0);
void delete_TaskManager(TaskManager* obj) { delete obj; } 
   QList<TaskManager_Task >  GetTasks(TaskManager* theWrappedObject);
   void SetTaskBlocksLibraryScans(TaskManager* theWrappedObject, int  id);
   void SetTaskFinished(TaskManager* theWrappedObject, int  id);
   void SetTaskProgress(TaskManager* theWrappedObject, int  id, int  progress, int  max = 0);
   int  StartTask(TaskManager* theWrappedObject, const QString&  name);
};





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


