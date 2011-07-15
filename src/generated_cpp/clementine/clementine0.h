#include <PythonQt.h>
#include <QObject>
#include <QVariant>
#include <albumcoverfetcher.h>
#include <albumcoverfetchersearch.h>
#include <coverprovider.h>
#include <coverproviders.h>
#include <directory.h>
#include <librarybackend.h>
#include <libraryquery.h>
#include <libraryview.h>
#include <network.h>
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
#include <qevent.h>
#include <qfont.h>
#include <qicon.h>
#include <qimage.h>
#include <qiodevice.h>
#include <qitemselectionmodel.h>
#include <qkeysequence.h>
#include <qlist.h>
#include <qlocale.h>
#include <qmargins.h>
#include <qmimedata.h>
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
#include <qstyle.h>
#include <qurl.h>
#include <qwidget.h>
#include <internetmodel.h>
#include <internetservice.h>
#include <song.h>
#include <specialplaylisttype.h>
#include <taskmanager.h>
#include <urlhandler.h>



class PythonQtWrapper_AlbumCoverFetcherSearch : public QObject
{ Q_OBJECT
public:
public slots:
void delete_AlbumCoverFetcherSearch(AlbumCoverFetcherSearch* obj) { delete obj; } 
   void Cancel(AlbumCoverFetcherSearch* theWrappedObject);
   void Start(AlbumCoverFetcherSearch* theWrappedObject);
};





class PythonQtShell_CoverProvider : public CoverProvider
{
public:
    PythonQtShell_CoverProvider(const QString&  name, QObject*  parent):CoverProvider(name, parent),_wrapper(NULL) {};

virtual void CancelSearch(int  id);
virtual bool  StartSearch(const QString&  artist, const QString&  album, int  id);
virtual void childEvent(QChildEvent*  arg__1);
virtual void customEvent(QEvent*  arg__1);
virtual bool  event(QEvent*  arg__1);
virtual bool  eventFilter(QObject*  arg__1, QEvent*  arg__2);
virtual void timerEvent(QTimerEvent*  arg__1);

  PythonQtInstanceWrapper* _wrapper; 
};

class PythonQtPublicPromoter_CoverProvider : public CoverProvider
{ public:
inline void promoted_CancelSearch(int  id) { CoverProvider::CancelSearch(id); }
};

class PythonQtWrapper_CoverProvider : public QObject
{ Q_OBJECT
public:
public slots:
CoverProvider* new_CoverProvider(const QString&  name, QObject*  parent);
void delete_CoverProvider(CoverProvider* obj) { delete obj; } 
   void CancelSearch(CoverProvider* theWrappedObject, int  id);
   QString  name(CoverProvider* theWrappedObject) const;
};





class PythonQtWrapper_CoverProviders : public QObject
{ Q_OBJECT
public:
public slots:
   void AddProvider(CoverProviders* theWrappedObject, CoverProvider*  provider);
   bool  HasAnyProviders(CoverProviders* theWrappedObject) const;
   QList<CoverProvider* >  List(CoverProviders* theWrappedObject) const;
   int  NextId(CoverProviders* theWrappedObject);
   void RemoveProvider(CoverProviders* theWrappedObject, CoverProvider*  provider);
   CoverProviders*  static_CoverProviders_instance();
};





class PythonQtShell_CoverSearchResult : public CoverSearchResult
{
public:
    PythonQtShell_CoverSearchResult():CoverSearchResult(),_wrapper(NULL) {};


  PythonQtInstanceWrapper* _wrapper; 
};

class PythonQtWrapper_CoverSearchResult : public QObject
{ Q_OBJECT
public:
public slots:
CoverSearchResult* new_CoverSearchResult();
CoverSearchResult* new_CoverSearchResult(const CoverSearchResult& other) {
PythonQtShell_CoverSearchResult* a = new PythonQtShell_CoverSearchResult();
*((CoverSearchResult*)a) = other;
return a; }
void delete_CoverSearchResult(CoverSearchResult* obj) { delete obj; } 
void py_set_provider(CoverSearchResult* theWrappedObject, QString  provider){ theWrappedObject->provider = provider; }
QString  py_get_provider(CoverSearchResult* theWrappedObject){ return theWrappedObject->provider; }
void py_set_description(CoverSearchResult* theWrappedObject, QString  description){ theWrappedObject->description = description; }
QString  py_get_description(CoverSearchResult* theWrappedObject){ return theWrappedObject->description; }
void py_set_image_url(CoverSearchResult* theWrappedObject, QString  image_url){ theWrappedObject->image_url = image_url; }
QString  py_get_image_url(CoverSearchResult* theWrappedObject){ return theWrappedObject->image_url; }
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





class PythonQtShell_LibraryView : public LibraryView
{
public:
    PythonQtShell_LibraryView(QWidget*  parent = 0):LibraryView(parent),_wrapper(NULL) {};


  PythonQtInstanceWrapper* _wrapper; 
};

class PythonQtWrapper_LibraryView : public QObject
{ Q_OBJECT
public:
public slots:
LibraryView* new_LibraryView(QWidget*  parent = 0);
void delete_LibraryView(LibraryView* obj) { delete obj; } 
   QList<Song >  GetSelectedSongs(LibraryView* theWrappedObject) const;
   void SetTaskManager(LibraryView* theWrappedObject, TaskManager*  task_manager);
   void keyboardSearch(LibraryView* theWrappedObject, const QString&  search);
   void scrollTo(LibraryView* theWrappedObject, const QModelIndex&  index, QAbstractItemView::ScrollHint  hint = QAbstractItemView::EnsureVisible);
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





class PythonQtShell_Player : public Player
{
public:
    PythonQtShell_Player(PlaylistManagerInterface*  playlists, QObject*  parent = 0):Player(playlists, parent),_wrapper(NULL) {};

virtual void CurrentMetadataChanged(const Song&  metadata);
virtual PlaylistItemPtr  GetCurrentItem() const;
virtual PlaylistItemPtr  GetItemAt(int  pos) const;
virtual Engine::State  GetState() const;
virtual int  GetVolume() const;
virtual void Mute();
virtual void Next();
virtual void Pause();
virtual void Play();
virtual void PlayAt(int  i, Engine::TrackChangeFlags  change, bool  reshuffle);
virtual void PlayPause();
virtual void Previous();
virtual void RegisterUrlHandler(UrlHandler*  handler);
virtual void ReloadSettings();
virtual void SeekBackward();
virtual void SeekForward();
virtual void SeekTo(int  seconds);
virtual void SetVolume(int  value);
virtual void ShowOSD();
virtual void Stop();
virtual void UnregisterUrlHandler(UrlHandler*  handler);
virtual void VolumeDown();
virtual void VolumeUp();
virtual void childEvent(QChildEvent*  arg__1);
virtual void customEvent(QEvent*  arg__1);
virtual Engine::Base*  engine() const;
virtual bool  event(QEvent*  arg__1);
virtual bool  eventFilter(QObject*  arg__1, QEvent*  arg__2);
virtual PlaylistManagerInterface*  playlists() const;
virtual void timerEvent(QTimerEvent*  arg__1);

  PythonQtInstanceWrapper* _wrapper; 
};

class PythonQtPublicPromoter_Player : public Player
{ public:
inline void promoted_CurrentMetadataChanged(const Song&  metadata) { Player::CurrentMetadataChanged(metadata); }
inline PlaylistItemPtr  promoted_GetCurrentItem() const { return Player::GetCurrentItem(); }
inline PlaylistItemPtr  promoted_GetItemAt(int  pos) const { return Player::GetItemAt(pos); }
inline Engine::State  promoted_GetState() const { return Player::GetState(); }
inline int  promoted_GetVolume() const { return Player::GetVolume(); }
inline void promoted_Mute() { Player::Mute(); }
inline void promoted_Next() { Player::Next(); }
inline void promoted_Pause() { Player::Pause(); }
inline void promoted_Play() { Player::Play(); }
inline void promoted_PlayAt(int  i, Engine::TrackChangeFlags  change, bool  reshuffle) { Player::PlayAt(i, change, reshuffle); }
inline void promoted_PlayPause() { Player::PlayPause(); }
inline void promoted_Previous() { Player::Previous(); }
inline void promoted_RegisterUrlHandler(UrlHandler*  handler) { Player::RegisterUrlHandler(handler); }
inline void promoted_ReloadSettings() { Player::ReloadSettings(); }
inline void promoted_SeekBackward() { Player::SeekBackward(); }
inline void promoted_SeekForward() { Player::SeekForward(); }
inline void promoted_SeekTo(int  seconds) { Player::SeekTo(seconds); }
inline void promoted_SetVolume(int  value) { Player::SetVolume(value); }
inline void promoted_ShowOSD() { Player::ShowOSD(); }
inline void promoted_Stop() { Player::Stop(); }
inline void promoted_UnregisterUrlHandler(UrlHandler*  handler) { Player::UnregisterUrlHandler(handler); }
inline void promoted_VolumeDown() { Player::VolumeDown(); }
inline void promoted_VolumeUp() { Player::VolumeUp(); }
inline Engine::Base*  promoted_engine() const { return Player::engine(); }
inline PlaylistManagerInterface*  promoted_playlists() const { return Player::playlists(); }
};

class PythonQtWrapper_Player : public QObject
{ Q_OBJECT
public:
public slots:
Player* new_Player(PlaylistManagerInterface*  playlists, QObject*  parent = 0);
void delete_Player(Player* obj) { delete obj; } 
   PlaylistItemPtr  GetCurrentItem(Player* theWrappedObject) const;
   PlaylistItemPtr  GetItemAt(Player* theWrappedObject, int  pos) const;
   Engine::State  GetState(Player* theWrappedObject) const;
   int  GetVolume(Player* theWrappedObject) const;
   void Init(Player* theWrappedObject);
   void RegisterUrlHandler(Player* theWrappedObject, UrlHandler*  handler);
   void UnregisterUrlHandler(Player* theWrappedObject, UrlHandler*  handler);
   Engine::Base*  engine(Player* theWrappedObject) const;
   PlaylistManagerInterface*  playlists(Player* theWrappedObject) const;
};





class PythonQtShell_PlayerInterface : public PlayerInterface
{
public:
    PythonQtShell_PlayerInterface(QObject*  parent = 0):PlayerInterface(parent),_wrapper(NULL) {};

virtual void CurrentMetadataChanged(const Song&  metadata);
virtual PlaylistItemPtr  GetCurrentItem() const;
virtual PlaylistItemPtr  GetItemAt(int  pos) const;
virtual Engine::State  GetState() const;
virtual int  GetVolume() const;
virtual void Mute();
virtual void Next();
virtual void Pause();
virtual void Play();
virtual void PlayAt(int  i, Engine::TrackChangeFlags  change, bool  reshuffle);
virtual void PlayPause();
virtual void Previous();
virtual void RegisterUrlHandler(UrlHandler*  handler);
virtual void ReloadSettings();
virtual void SeekBackward();
virtual void SeekForward();
virtual void SeekTo(int  seconds);
virtual void SetVolume(int  value);
virtual void ShowOSD();
virtual void Stop();
virtual void UnregisterUrlHandler(UrlHandler*  handler);
virtual void VolumeDown();
virtual void VolumeUp();
virtual void childEvent(QChildEvent*  arg__1);
virtual void customEvent(QEvent*  arg__1);
virtual Engine::Base*  engine() const;
virtual bool  event(QEvent*  arg__1);
virtual bool  eventFilter(QObject*  arg__1, QEvent*  arg__2);
virtual PlaylistManagerInterface*  playlists() const;
virtual void timerEvent(QTimerEvent*  arg__1);

  PythonQtInstanceWrapper* _wrapper; 
};

class PythonQtWrapper_PlayerInterface : public QObject
{ Q_OBJECT
public:
public slots:
PlayerInterface* new_PlayerInterface(QObject*  parent = 0);
void delete_PlayerInterface(PlayerInterface* obj) { delete obj; } 
};





class PythonQtShell_Playlist : public Playlist
{
public:
    PythonQtShell_Playlist(PlaylistBackend*  backend, TaskManager*  task_manager, LibraryBackend*  library, int  id, const QString&  special_type = QString(), QObject*  parent = 0):Playlist(backend, task_manager, library, id, special_type, parent),_wrapper(NULL) {};

virtual QModelIndex  buddy(const QModelIndex&  index) const;
virtual bool  canFetchMore(const QModelIndex&  parent) const;
virtual void childEvent(QChildEvent*  arg__1);
virtual int  columnCount(const QModelIndex&  arg__1 = QModelIndex()) const;
virtual void customEvent(QEvent*  arg__1);
virtual QVariant  data(const QModelIndex&  index, int  role = Qt::DisplayRole) const;
virtual bool  dropMimeData(const QMimeData*  data, Qt::DropAction  action, int  row, int  column, const QModelIndex&  parent);
virtual bool  event(QEvent*  arg__1);
virtual bool  eventFilter(QObject*  arg__1, QEvent*  arg__2);
virtual void fetchMore(const QModelIndex&  parent);
virtual Qt::ItemFlags  flags(const QModelIndex&  index) const;
virtual QVariant  headerData(int  section, Qt::Orientation  orientation, int  role = Qt::DisplayRole) const;
virtual QModelIndex  index(int  row, int  column, const QModelIndex&  parent) const;
virtual bool  insertColumns(int  column, int  count, const QModelIndex&  parent);
virtual bool  insertRows(int  row, int  count, const QModelIndex&  parent);
virtual QMap<int , QVariant >  itemData(const QModelIndex&  index) const;
virtual QList<QModelIndex >  match(const QModelIndex&  start, int  role, const QVariant&  value, int  hits, Qt::MatchFlags  flags) const;
virtual QMimeData*  mimeData(const QList<QModelIndex >&  indexes) const;
virtual QStringList  mimeTypes() const;
virtual bool  removeColumns(int  column, int  count, const QModelIndex&  parent);
virtual bool  removeRows(int  row, int  count, const QModelIndex&  parent = QModelIndex());
virtual void revert();
virtual int  rowCount(const QModelIndex&  arg__1 = QModelIndex()) const;
virtual bool  setData(const QModelIndex&  index, const QVariant&  value, int  role);
virtual bool  setHeaderData(int  section, Qt::Orientation  orientation, const QVariant&  value, int  role);
virtual bool  setItemData(const QModelIndex&  index, const QMap<int , QVariant >&  roles);
virtual void sort(int  column, Qt::SortOrder  order);
virtual QSize  span(const QModelIndex&  index) const;
virtual bool  submit();
virtual Qt::DropActions  supportedDropActions() const;
virtual void timerEvent(QTimerEvent*  arg__1);

  PythonQtInstanceWrapper* _wrapper; 
};

class PythonQtPublicPromoter_Playlist : public Playlist
{ public:
inline int  promoted_columnCount(const QModelIndex&  arg__1 = QModelIndex()) const { return Playlist::columnCount(arg__1); }
inline QVariant  promoted_data(const QModelIndex&  index, int  role = Qt::DisplayRole) const { return Playlist::data(index, role); }
inline bool  promoted_dropMimeData(const QMimeData*  data, Qt::DropAction  action, int  row, int  column, const QModelIndex&  parent) { return Playlist::dropMimeData(data, action, row, column, parent); }
inline Qt::ItemFlags  promoted_flags(const QModelIndex&  index) const { return Playlist::flags(index); }
inline QVariant  promoted_headerData(int  section, Qt::Orientation  orientation, int  role = Qt::DisplayRole) const { return Playlist::headerData(section, orientation, role); }
inline QMimeData*  promoted_mimeData(const QList<QModelIndex >&  indexes) const { return Playlist::mimeData(indexes); }
inline QStringList  promoted_mimeTypes() const { return Playlist::mimeTypes(); }
inline bool  promoted_removeRows(int  row, int  count, const QModelIndex&  parent = QModelIndex()) { return Playlist::removeRows(row, count, parent); }
inline int  promoted_rowCount(const QModelIndex&  arg__1 = QModelIndex()) const { return Playlist::rowCount(arg__1); }
inline bool  promoted_setData(const QModelIndex&  index, const QVariant&  value, int  role) { return Playlist::setData(index, value, role); }
inline void promoted_sort(int  column, Qt::SortOrder  order) { Playlist::sort(column, order); }
inline Qt::DropActions  promoted_supportedDropActions() const { return Playlist::supportedDropActions(); }
};

class PythonQtWrapper_Playlist : public QObject
{ Q_OBJECT
public:
Q_ENUMS(Column Role LastFMStatus )
enum Column{
  Column_Title = Playlist::Column_Title,   Column_Artist = Playlist::Column_Artist,   Column_Album = Playlist::Column_Album,   Column_AlbumArtist = Playlist::Column_AlbumArtist,   Column_Composer = Playlist::Column_Composer,   Column_Length = Playlist::Column_Length,   Column_Track = Playlist::Column_Track,   Column_Disc = Playlist::Column_Disc,   Column_Year = Playlist::Column_Year,   Column_Genre = Playlist::Column_Genre,   Column_BPM = Playlist::Column_BPM,   Column_Bitrate = Playlist::Column_Bitrate,   Column_Samplerate = Playlist::Column_Samplerate,   Column_Filename = Playlist::Column_Filename,   Column_BaseFilename = Playlist::Column_BaseFilename,   Column_Filesize = Playlist::Column_Filesize,   Column_Filetype = Playlist::Column_Filetype,   Column_DateCreated = Playlist::Column_DateCreated,   Column_DateModified = Playlist::Column_DateModified,   Column_Rating = Playlist::Column_Rating,   Column_PlayCount = Playlist::Column_PlayCount,   Column_SkipCount = Playlist::Column_SkipCount,   Column_LastPlayed = Playlist::Column_LastPlayed,   Column_Score = Playlist::Column_Score,   Column_Comment = Playlist::Column_Comment,   ColumnCount = Playlist::ColumnCount};
enum Role{
  Role_IsCurrent = Playlist::Role_IsCurrent,   Role_IsPaused = Playlist::Role_IsPaused,   Role_StopAfter = Playlist::Role_StopAfter,   Role_QueuePosition = Playlist::Role_QueuePosition,   Role_CanSetRating = Playlist::Role_CanSetRating};
enum LastFMStatus{
  LastFM_New = Playlist::LastFM_New,   LastFM_Scrobbled = Playlist::LastFM_Scrobbled,   LastFM_Seeked = Playlist::LastFM_Seeked,   LastFM_Error = Playlist::LastFM_Error,   LastFM_Invalid = Playlist::LastFM_Invalid,   LastFM_Queued = Playlist::LastFM_Queued};
public slots:
Playlist* new_Playlist(PlaylistBackend*  backend, TaskManager*  task_manager, LibraryBackend*  library, int  id, const QString&  special_type = QString(), QObject*  parent = 0);
void delete_Playlist(Playlist* obj) { delete obj; } 
   void AddSongInsertVetoListener(Playlist* theWrappedObject, SongInsertVetoListener*  listener);
   bool  ApplyValidityOnCurrentSong(Playlist* theWrappedObject, const QUrl&  url, bool  valid);
   bool  static_Playlist_CompareItems(int  column, Qt::SortOrder  order, PlaylistItemPtr  a, PlaylistItemPtr  b);
   QList<PlaylistItemPtr >  GetAllItems(Playlist* theWrappedObject) const;
   QList<Song >  GetAllSongs(Playlist* theWrappedObject) const;
   quint64  GetTotalLength(Playlist* theWrappedObject) const;
   void InsertItems(Playlist* theWrappedObject, const QList<PlaylistItemPtr >&  items, int  pos = -1, bool  play_now = false, bool  enqueue = false);
   void InsertLibraryItems(Playlist* theWrappedObject, const QList<Song >&  items, int  pos = -1, bool  play_now = false, bool  enqueue = false);
   void InsertSmartPlaylist(Playlist* theWrappedObject, smart_playlists::GeneratorPtr  gen, int  pos = -1, bool  play_now = false, bool  enqueue = false);
   void InsertSongs(Playlist* theWrappedObject, const QList<Song >&  items, int  pos = -1, bool  play_now = false, bool  enqueue = false);
   void InsertSongsOrLibraryItems(Playlist* theWrappedObject, const QList<Song >&  items, int  pos = -1, bool  play_now = false, bool  enqueue = false);
   void InsertUrls(Playlist* theWrappedObject, const QList<QUrl >&  urls, int  pos = -1, bool  play_now = false, bool  enqueue = false);
   void InvalidateDeletedSongs(Playlist* theWrappedObject);
   void RateSong(Playlist* theWrappedObject, const QModelIndex&  index, double  rating);
   void ReloadItems(Playlist* theWrappedObject, const QList<int >&  rows);
   void RemoveDeletedSongs(Playlist* theWrappedObject);
   void RemoveItemsWithoutUndo(Playlist* theWrappedObject, const QList<int >&  indices);
   void RemoveSongInsertVetoListener(Playlist* theWrappedObject, SongInsertVetoListener*  listener);
   void Restore(Playlist* theWrappedObject);
   void Save(Playlist* theWrappedObject) const;
   void StopAfter(Playlist* theWrappedObject, int  row);
   void UpdateItems(Playlist* theWrappedObject, const QList<Song >&  songs);
   QString  static_Playlist_abbreviated_column_name(Playlist::Column  column);
   int  columnCount(Playlist* theWrappedObject, const QModelIndex&  arg__1 = QModelIndex()) const;
   QMap<int , Qt::Alignment >  column_alignments(Playlist* theWrappedObject) const;
   bool  static_Playlist_column_is_editable(Playlist::Column  column);
   QString  static_Playlist_column_name(Playlist::Column  column);
   const QModelIndex  current_index(Playlist* theWrappedObject) const;
   PlaylistItemPtr  current_item(Playlist* theWrappedObject) const;
   Song  current_item_metadata(Playlist* theWrappedObject) const;
   int  current_row(Playlist* theWrappedObject) const;
   QVariant  data(Playlist* theWrappedObject, const QModelIndex&  index, int  role = Qt::DisplayRole) const;
   bool  dropMimeData(Playlist* theWrappedObject, const QMimeData*  data, Qt::DropAction  action, int  row, int  column, const QModelIndex&  parent);
   Qt::ItemFlags  flags(Playlist* theWrappedObject, const QModelIndex&  index) const;
   Playlist::LastFMStatus  get_lastfm_status(Playlist* theWrappedObject) const;
   const bool  has_item_at(Playlist* theWrappedObject, int  index) const;
   bool  have_incremented_playcount(Playlist* theWrappedObject) const;
   QVariant  headerData(Playlist* theWrappedObject, int  section, Qt::Orientation  orientation, int  role = Qt::DisplayRole) const;
   int  id(Playlist* theWrappedObject) const;
   bool  is_dynamic(Playlist* theWrappedObject) const;
   const PlaylistItemPtr*  item_at(Playlist* theWrappedObject, int  index) const;
   int  last_played_row(Playlist* theWrappedObject) const;
   QList<PlaylistItemPtr >  library_items_by_id(Playlist* theWrappedObject, int  id) const;
   QMimeData*  mimeData(Playlist* theWrappedObject, const QList<QModelIndex >&  indexes) const;
   QStringList  mimeTypes(Playlist* theWrappedObject) const;
   int  next_row(Playlist* theWrappedObject) const;
   int  previous_row(Playlist* theWrappedObject) const;
   QSortFilterProxyModel*  proxy(Playlist* theWrappedObject) const;
   bool  removeRows(Playlist* theWrappedObject, int  row, int  count, const QModelIndex&  parent = QModelIndex());
   int  rowCount(Playlist* theWrappedObject, const QModelIndex&  arg__1 = QModelIndex()) const;
   qint64  scrobble_point_nanosec(Playlist* theWrappedObject) const;
   PlaylistSequence*  sequence(Playlist* theWrappedObject) const;
   bool  setData(Playlist* theWrappedObject, const QModelIndex&  index, const QVariant&  value, int  role);
   void set_column_align_center(Playlist* theWrappedObject, int  column);
   void set_column_align_left(Playlist* theWrappedObject, int  column);
   void set_column_align_right(Playlist* theWrappedObject, int  column);
   void set_column_alignments(Playlist* theWrappedObject, const QMap<int , Qt::Alignment >&  column_alignments);
   bool  static_Playlist_set_column_value(Song&  song, Playlist::Column  column, const QVariant&  value);
   void set_have_incremented_playcount(Playlist* theWrappedObject);
   void set_lastfm_status(Playlist* theWrappedObject, Playlist::LastFMStatus  status);
   void set_sequence(Playlist* theWrappedObject, PlaylistSequence*  v);
   void set_special_type(Playlist* theWrappedObject, const QString&  v);
   void sort(Playlist* theWrappedObject, int  column, Qt::SortOrder  order);
   QString  special_type(Playlist* theWrappedObject) const;
   bool  stop_after_current(Playlist* theWrappedObject) const;
   Qt::DropActions  supportedDropActions(Playlist* theWrappedObject) const;
   QUndoStack*  undo_stack(Playlist* theWrappedObject) const;
};





class PythonQtShell_PlaylistBackend : public PlaylistBackend
{
public:
    PythonQtShell_PlaylistBackend(QObject*  parent = 0):PlaylistBackend(parent),_wrapper(NULL) {};

virtual void childEvent(QChildEvent*  arg__1);
virtual void customEvent(QEvent*  arg__1);
virtual bool  event(QEvent*  arg__1);
virtual bool  eventFilter(QObject*  arg__1, QEvent*  arg__2);
virtual void timerEvent(QTimerEvent*  arg__1);

  PythonQtInstanceWrapper* _wrapper; 
};

class PythonQtWrapper_PlaylistBackend : public QObject
{ Q_OBJECT
public:
public slots:
PlaylistBackend* new_PlaylistBackend(QObject*  parent = 0);
void delete_PlaylistBackend(PlaylistBackend* obj) { delete obj; } 
   int  CreatePlaylist(PlaylistBackend* theWrappedObject, const QString&  name, const QString&  special_type);
   void RemovePlaylist(PlaylistBackend* theWrappedObject, int  id);
   void RenamePlaylist(PlaylistBackend* theWrappedObject, int  id, const QString&  new_name);
   void SavePlaylistAsync(PlaylistBackend* theWrappedObject, int  playlist, const QList<PlaylistItemPtr >&  items, int  last_played, smart_playlists::GeneratorPtr  dynamic);
   void SetLibrary(PlaylistBackend* theWrappedObject, LibraryBackend*  library);
   void SetPlaylistOrder(PlaylistBackend* theWrappedObject, const QList<int >&  ids);
};





class PythonQtShell_PlaylistContainer : public PlaylistContainer
{
public:
    PythonQtShell_PlaylistContainer(QWidget*  parent = 0):PlaylistContainer(parent),_wrapper(NULL) {};

virtual void actionEvent(QActionEvent*  arg__1);
virtual void changeEvent(QEvent*  arg__1);
virtual void childEvent(QChildEvent*  arg__1);
virtual void closeEvent(QCloseEvent*  arg__1);
virtual void contextMenuEvent(QContextMenuEvent*  arg__1);
virtual void customEvent(QEvent*  arg__1);
virtual int  devType() const;
virtual void dragEnterEvent(QDragEnterEvent*  arg__1);
virtual void dragLeaveEvent(QDragLeaveEvent*  arg__1);
virtual void dragMoveEvent(QDragMoveEvent*  arg__1);
virtual void dropEvent(QDropEvent*  arg__1);
virtual void enterEvent(QEvent*  arg__1);
virtual bool  event(QEvent*  arg__1);
virtual bool  eventFilter(QObject*  arg__1, QEvent*  arg__2);
virtual void focusInEvent(QFocusEvent*  arg__1);
virtual bool  focusNextPrevChild(bool  next);
virtual void focusOutEvent(QFocusEvent*  arg__1);
virtual int  heightForWidth(int  arg__1) const;
virtual void hideEvent(QHideEvent*  arg__1);
virtual void inputMethodEvent(QInputMethodEvent*  arg__1);
virtual QVariant  inputMethodQuery(Qt::InputMethodQuery  arg__1) const;
virtual void keyPressEvent(QKeyEvent*  arg__1);
virtual void keyReleaseEvent(QKeyEvent*  arg__1);
virtual void languageChange();
virtual void leaveEvent(QEvent*  arg__1);
virtual int  metric(QPaintDevice::PaintDeviceMetric  arg__1) const;
virtual QSize  minimumSizeHint() const;
virtual void mouseDoubleClickEvent(QMouseEvent*  arg__1);
virtual void mouseMoveEvent(QMouseEvent*  arg__1);
virtual void mousePressEvent(QMouseEvent*  arg__1);
virtual void mouseReleaseEvent(QMouseEvent*  arg__1);
virtual void moveEvent(QMoveEvent*  arg__1);
virtual QPaintEngine*  paintEngine() const;
virtual void paintEvent(QPaintEvent*  arg__1);
virtual void resizeEvent(QResizeEvent*  arg__1);
virtual void showEvent(QShowEvent*  arg__1);
virtual QSize  sizeHint() const;
virtual void tabletEvent(QTabletEvent*  arg__1);
virtual void timerEvent(QTimerEvent*  arg__1);
virtual void wheelEvent(QWheelEvent*  arg__1);

  PythonQtInstanceWrapper* _wrapper; 
};

class PythonQtPublicPromoter_PlaylistContainer : public PlaylistContainer
{ public:
inline void promoted_resizeEvent(QResizeEvent*  arg__1) { PlaylistContainer::resizeEvent(arg__1); }
};

class PythonQtWrapper_PlaylistContainer : public QObject
{ Q_OBJECT
public:
public slots:
PlaylistContainer* new_PlaylistContainer(QWidget*  parent = 0);
void delete_PlaylistContainer(PlaylistContainer* obj) { delete obj; } 
   void SetActions(PlaylistContainer* theWrappedObject, QAction*  new_playlist, QAction*  save_playlist, QAction*  load_playlist, QAction*  next_playlist, QAction*  previous_playlist);
   void SetManager(PlaylistContainer* theWrappedObject, PlaylistManager*  manager);
   void resizeEvent(PlaylistContainer* theWrappedObject, QResizeEvent*  arg__1);
};





class PythonQtShell_PlaylistItemPtr : public PlaylistItemPtr
{
public:
    PythonQtShell_PlaylistItemPtr():PlaylistItemPtr(),_wrapper(NULL) {};


  PythonQtInstanceWrapper* _wrapper; 
};

class PythonQtWrapper_PlaylistItemPtr : public QObject
{ Q_OBJECT
public:
public slots:
PlaylistItemPtr* new_PlaylistItemPtr();
PlaylistItemPtr* new_PlaylistItemPtr(const PlaylistItemPtr& other) {
PythonQtShell_PlaylistItemPtr* a = new PythonQtShell_PlaylistItemPtr();
*((PlaylistItemPtr*)a) = other;
return a; }
void delete_PlaylistItemPtr(PlaylistItemPtr* obj) { delete obj; } 
};





class PythonQtShell_PlaylistManager : public PlaylistManager
{
public:
    PythonQtShell_PlaylistManager(TaskManager*  task_manager, QObject*  parent = 0):PlaylistManager(task_manager, parent),_wrapper(NULL) {};

virtual void ChangePlaylistOrder(const QList<int >&  ids);
virtual void ClearCurrent();
virtual QList<Playlist* >  GetAllPlaylists() const;
virtual QString  GetPlaylistName(int  index) const;
virtual SpecialPlaylistType*  GetPlaylistType(const QString&  type) const;
virtual void InvalidateDeletedSongs();
virtual void Load(const QString&  filename);
virtual void New(const QString&  name, const QList<Song >&  songs = SongList(), const QString&  special_type = QString());
virtual void PlaySmartPlaylist(smart_playlists::GeneratorPtr  generator, bool  as_new, bool  clear);
virtual void RateCurrentSong(double  rating);
virtual void RateCurrentSong(int  rating);
virtual void RegisterSpecialPlaylistType(SpecialPlaylistType*  type);
virtual void Remove(int  id);
virtual void RemoveDeletedSongs();
virtual void Rename(int  id, const QString&  new_name);
virtual void Save(int  id, const QString&  filename);
virtual void SelectionChanged(const QItemSelection&  selection);
virtual void SetActivePaused();
virtual void SetActivePlaying();
virtual void SetActivePlaylist(int  id);
virtual void SetActiveStopped();
virtual void SetActiveStreamMetadata(const QUrl&  url, const Song&  song);
virtual void SetActiveToCurrent();
virtual void SetCurrentPlaylist(int  id);
virtual void ShuffleCurrent();
virtual void SongChangeRequestProcessed(const QUrl&  url, bool  valid);
virtual void UnregisterSpecialPlaylistType(SpecialPlaylistType*  type);
virtual Playlist*  active() const;
virtual int  active_id() const;
virtual QItemSelection  active_selection() const;
virtual void childEvent(QChildEvent*  arg__1);
virtual Playlist*  current() const;
virtual int  current_id() const;
virtual QItemSelection  current_selection() const;
virtual void customEvent(QEvent*  arg__1);
virtual bool  event(QEvent*  arg__1);
virtual bool  eventFilter(QObject*  arg__1, QEvent*  arg__2);
virtual LibraryBackend*  library_backend() const;
virtual PlaylistParser*  parser() const;
virtual Playlist*  playlist(int  id) const;
virtual PlaylistBackend*  playlist_backend() const;
virtual PlaylistContainer*  playlist_container() const;
virtual QItemSelection  selection(int  id) const;
virtual PlaylistSequence*  sequence() const;
virtual TaskManager*  task_manager() const;
virtual void timerEvent(QTimerEvent*  arg__1);

  PythonQtInstanceWrapper* _wrapper; 
};

class PythonQtPublicPromoter_PlaylistManager : public PlaylistManager
{ public:
inline void promoted_ChangePlaylistOrder(const QList<int >&  ids) { PlaylistManager::ChangePlaylistOrder(ids); }
inline void promoted_ClearCurrent() { PlaylistManager::ClearCurrent(); }
inline QList<Playlist* >  promoted_GetAllPlaylists() const { return PlaylistManager::GetAllPlaylists(); }
inline QString  promoted_GetPlaylistName(int  index) const { return PlaylistManager::GetPlaylistName(index); }
inline SpecialPlaylistType*  promoted_GetPlaylistType(const QString&  type) const { return PlaylistManager::GetPlaylistType(type); }
inline void promoted_InvalidateDeletedSongs() { PlaylistManager::InvalidateDeletedSongs(); }
inline void promoted_Load(const QString&  filename) { PlaylistManager::Load(filename); }
inline void promoted_New(const QString&  name, const QList<Song >&  songs = SongList(), const QString&  special_type = QString()) { PlaylistManager::New(name, songs, special_type); }
inline void promoted_PlaySmartPlaylist(smart_playlists::GeneratorPtr  generator, bool  as_new, bool  clear) { PlaylistManager::PlaySmartPlaylist(generator, as_new, clear); }
inline void promoted_RateCurrentSong(double  rating) { PlaylistManager::RateCurrentSong(rating); }
inline void promoted_RateCurrentSong(int  rating) { PlaylistManager::RateCurrentSong(rating); }
inline void promoted_RegisterSpecialPlaylistType(SpecialPlaylistType*  type) { PlaylistManager::RegisterSpecialPlaylistType(type); }
inline void promoted_Remove(int  id) { PlaylistManager::Remove(id); }
inline void promoted_RemoveDeletedSongs() { PlaylistManager::RemoveDeletedSongs(); }
inline void promoted_Rename(int  id, const QString&  new_name) { PlaylistManager::Rename(id, new_name); }
inline void promoted_Save(int  id, const QString&  filename) { PlaylistManager::Save(id, filename); }
inline void promoted_SelectionChanged(const QItemSelection&  selection) { PlaylistManager::SelectionChanged(selection); }
inline void promoted_SetActivePaused() { PlaylistManager::SetActivePaused(); }
inline void promoted_SetActivePlaying() { PlaylistManager::SetActivePlaying(); }
inline void promoted_SetActivePlaylist(int  id) { PlaylistManager::SetActivePlaylist(id); }
inline void promoted_SetActiveStopped() { PlaylistManager::SetActiveStopped(); }
inline void promoted_SetActiveStreamMetadata(const QUrl&  url, const Song&  song) { PlaylistManager::SetActiveStreamMetadata(url, song); }
inline void promoted_SetActiveToCurrent() { PlaylistManager::SetActiveToCurrent(); }
inline void promoted_SetCurrentPlaylist(int  id) { PlaylistManager::SetCurrentPlaylist(id); }
inline void promoted_ShuffleCurrent() { PlaylistManager::ShuffleCurrent(); }
inline void promoted_SongChangeRequestProcessed(const QUrl&  url, bool  valid) { PlaylistManager::SongChangeRequestProcessed(url, valid); }
inline void promoted_UnregisterSpecialPlaylistType(SpecialPlaylistType*  type) { PlaylistManager::UnregisterSpecialPlaylistType(type); }
inline Playlist*  promoted_active() const { return PlaylistManager::active(); }
inline int  promoted_active_id() const { return PlaylistManager::active_id(); }
inline QItemSelection  promoted_active_selection() const { return PlaylistManager::active_selection(); }
inline Playlist*  promoted_current() const { return PlaylistManager::current(); }
inline int  promoted_current_id() const { return PlaylistManager::current_id(); }
inline QItemSelection  promoted_current_selection() const { return PlaylistManager::current_selection(); }
inline LibraryBackend*  promoted_library_backend() const { return PlaylistManager::library_backend(); }
inline PlaylistParser*  promoted_parser() const { return PlaylistManager::parser(); }
inline Playlist*  promoted_playlist(int  id) const { return PlaylistManager::playlist(id); }
inline PlaylistBackend*  promoted_playlist_backend() const { return PlaylistManager::playlist_backend(); }
inline PlaylistContainer*  promoted_playlist_container() const { return PlaylistManager::playlist_container(); }
inline QItemSelection  promoted_selection(int  id) const { return PlaylistManager::selection(id); }
inline PlaylistSequence*  promoted_sequence() const { return PlaylistManager::sequence(); }
inline TaskManager*  promoted_task_manager() const { return PlaylistManager::task_manager(); }
};

class PythonQtWrapper_PlaylistManager : public QObject
{ Q_OBJECT
public:
public slots:
PlaylistManager* new_PlaylistManager(TaskManager*  task_manager, QObject*  parent = 0);
void delete_PlaylistManager(PlaylistManager* obj) { delete obj; } 
   QList<Playlist* >  GetAllPlaylists(PlaylistManager* theWrappedObject) const;
   QString  static_PlaylistManager_GetNameForNewPlaylist(const QList<Song >&  songs);
   QString  GetPlaylistName(PlaylistManager* theWrappedObject, int  index) const;
   SpecialPlaylistType*  GetPlaylistType(PlaylistManager* theWrappedObject, const QString&  type) const;
   void Init(PlaylistManager* theWrappedObject, LibraryBackend*  library_backend, PlaylistBackend*  playlist_backend, PlaylistSequence*  sequence, PlaylistContainer*  playlist_container);
   void InvalidateDeletedSongs(PlaylistManager* theWrappedObject);
   void RegisterSpecialPlaylistType(PlaylistManager* theWrappedObject, SpecialPlaylistType*  type);
   void RemoveDeletedSongs(PlaylistManager* theWrappedObject);
   void UnregisterSpecialPlaylistType(PlaylistManager* theWrappedObject, SpecialPlaylistType*  type);
   Playlist*  active(PlaylistManager* theWrappedObject) const;
   int  active_id(PlaylistManager* theWrappedObject) const;
   QItemSelection  active_selection(PlaylistManager* theWrappedObject) const;
   Playlist*  current(PlaylistManager* theWrappedObject) const;
   int  current_id(PlaylistManager* theWrappedObject) const;
   QItemSelection  current_selection(PlaylistManager* theWrappedObject) const;
   LibraryBackend*  library_backend(PlaylistManager* theWrappedObject) const;
   PlaylistParser*  parser(PlaylistManager* theWrappedObject) const;
   Playlist*  playlist(PlaylistManager* theWrappedObject, int  id) const;
   PlaylistBackend*  playlist_backend(PlaylistManager* theWrappedObject) const;
   PlaylistContainer*  playlist_container(PlaylistManager* theWrappedObject) const;
   QItemSelection  selection(PlaylistManager* theWrappedObject, int  id) const;
   PlaylistSequence*  sequence(PlaylistManager* theWrappedObject) const;
   TaskManager*  task_manager(PlaylistManager* theWrappedObject) const;
};





class PythonQtShell_PlaylistManagerInterface : public PlaylistManagerInterface
{
public:
    PythonQtShell_PlaylistManagerInterface(QObject*  parent):PlaylistManagerInterface(parent),_wrapper(NULL) {};

virtual void ChangePlaylistOrder(const QList<int >&  ids);
virtual void ClearCurrent();
virtual QList<Playlist* >  GetAllPlaylists() const;
virtual QString  GetPlaylistName(int  index) const;
virtual SpecialPlaylistType*  GetPlaylistType(const QString&  type) const;
virtual void InvalidateDeletedSongs();
virtual void Load(const QString&  filename);
virtual void New(const QString&  name, const QList<Song >&  songs = SongList(), const QString&  special_type = QString());
virtual void PlaySmartPlaylist(smart_playlists::GeneratorPtr  generator, bool  as_new, bool  clear);
virtual void RateCurrentSong(double  rating);
virtual void RateCurrentSong(int  rating);
virtual void RegisterSpecialPlaylistType(SpecialPlaylistType*  type);
virtual void Remove(int  id);
virtual void RemoveDeletedSongs();
virtual void Rename(int  id, const QString&  new_name);
virtual void Save(int  id, const QString&  filename);
virtual void SelectionChanged(const QItemSelection&  selection);
virtual void SetActivePaused();
virtual void SetActivePlaying();
virtual void SetActivePlaylist(int  id);
virtual void SetActiveStopped();
virtual void SetActiveStreamMetadata(const QUrl&  url, const Song&  song);
virtual void SetActiveToCurrent();
virtual void SetCurrentPlaylist(int  id);
virtual void ShuffleCurrent();
virtual void SongChangeRequestProcessed(const QUrl&  url, bool  valid);
virtual void UnregisterSpecialPlaylistType(SpecialPlaylistType*  type);
virtual Playlist*  active() const;
virtual int  active_id() const;
virtual QItemSelection  active_selection() const;
virtual void childEvent(QChildEvent*  arg__1);
virtual Playlist*  current() const;
virtual int  current_id() const;
virtual QItemSelection  current_selection() const;
virtual void customEvent(QEvent*  arg__1);
virtual bool  event(QEvent*  arg__1);
virtual bool  eventFilter(QObject*  arg__1, QEvent*  arg__2);
virtual LibraryBackend*  library_backend() const;
virtual PlaylistParser*  parser() const;
virtual Playlist*  playlist(int  id) const;
virtual PlaylistBackend*  playlist_backend() const;
virtual PlaylistContainer*  playlist_container() const;
virtual QItemSelection  selection(int  id) const;
virtual PlaylistSequence*  sequence() const;
virtual TaskManager*  task_manager() const;
virtual void timerEvent(QTimerEvent*  arg__1);

  PythonQtInstanceWrapper* _wrapper; 
};

class PythonQtWrapper_PlaylistManagerInterface : public QObject
{ Q_OBJECT
public:
public slots:
PlaylistManagerInterface* new_PlaylistManagerInterface(QObject*  parent);
void delete_PlaylistManagerInterface(PlaylistManagerInterface* obj) { delete obj; } 
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





class PythonQtWrapper_PlaylistSequence : public QObject
{ Q_OBJECT
public:
Q_ENUMS(ShuffleMode RepeatMode )
enum ShuffleMode{
  Shuffle_Off = PlaylistSequence::Shuffle_Off,   Shuffle_All = PlaylistSequence::Shuffle_All,   Shuffle_Album = PlaylistSequence::Shuffle_Album};
enum RepeatMode{
  Repeat_Off = PlaylistSequence::Repeat_Off,   Repeat_Track = PlaylistSequence::Repeat_Track,   Repeat_Album = PlaylistSequence::Repeat_Album,   Repeat_Playlist = PlaylistSequence::Repeat_Playlist};
public slots:
void delete_PlaylistSequence(PlaylistSequence* obj) { delete obj; } 
   QMenu*  repeat_menu(PlaylistSequence* theWrappedObject) const;
   PlaylistSequence::RepeatMode  repeat_mode(PlaylistSequence* theWrappedObject) const;
   QMenu*  shuffle_menu(PlaylistSequence* theWrappedObject) const;
   PlaylistSequence::ShuffleMode  shuffle_mode(PlaylistSequence* theWrappedObject) const;
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





class PythonQtPublicPromoter_InternetModel : public InternetModel
{ public:
inline Qt::ItemFlags  promoted_flags(const QModelIndex&  index) const { return InternetModel::flags(index); }
inline bool  promoted_hasChildren(const QModelIndex&  parent) const { return InternetModel::hasChildren(parent); }
inline QMimeData*  promoted_mimeData(const QList<QModelIndex >&  indexes) const { return InternetModel::mimeData(indexes); }
inline QStringList  promoted_mimeTypes() const { return InternetModel::mimeTypes(); }
inline int  promoted_rowCount(const QModelIndex&  parent) const { return InternetModel::rowCount(parent); }
};

class PythonQtWrapper_InternetModel : public QObject
{ Q_OBJECT
public:
Q_ENUMS(PlayBehaviour Role Type )
enum PlayBehaviour{
  PlayBehaviour_None = InternetModel::PlayBehaviour_None,   PlayBehaviour_UseSongLoader = InternetModel::PlayBehaviour_UseSongLoader,   PlayBehaviour_SingleItem = InternetModel::PlayBehaviour_SingleItem,   PlayBehaviour_DoubleClickAction = InternetModel::PlayBehaviour_DoubleClickAction};
enum Role{
  Role_Type = InternetModel::Role_Type,   Role_PlayBehaviour = InternetModel::Role_PlayBehaviour,   Role_Url = InternetModel::Role_Url,   Role_SongMetadata = InternetModel::Role_SongMetadata,   Role_CanLazyLoad = InternetModel::Role_CanLazyLoad,   Role_Service = InternetModel::Role_Service,   RoleCount = InternetModel::RoleCount};
enum Type{
  Type_Service = InternetModel::Type_Service,   TypeCount = InternetModel::TypeCount};
public slots:
void delete_InternetModel(InternetModel* obj) { delete obj; } 
   void AddService(InternetModel* theWrappedObject, InternetService*  service);
   bool  IsPlayable(InternetModel* theWrappedObject, const QModelIndex&  index) const;
   void ReloadSettings(InternetModel* theWrappedObject);
   void RemoveService(InternetModel* theWrappedObject, InternetService*  service);
   InternetService*  static_InternetModel_ServiceByName(const QString&  name);
   InternetService*  ServiceForIndex(InternetModel* theWrappedObject, const QModelIndex&  index) const;
   InternetService*  ServiceForItem(InternetModel* theWrappedObject, const QStandardItem*  item) const;
   void ShowContextMenu(InternetModel* theWrappedObject, const QModelIndex&  merged_model_index, const QPoint&  global_pos);
   Qt::ItemFlags  flags(InternetModel* theWrappedObject, const QModelIndex&  index) const;
   bool  hasChildren(InternetModel* theWrappedObject, const QModelIndex&  parent) const;
   QMimeData*  mimeData(InternetModel* theWrappedObject, const QList<QModelIndex >&  indexes) const;
   QStringList  mimeTypes(InternetModel* theWrappedObject) const;
   PlayerInterface*  player(InternetModel* theWrappedObject) const;
   int  rowCount(InternetModel* theWrappedObject, const QModelIndex&  parent) const;
   TaskManager*  task_manager(InternetModel* theWrappedObject) const;
};





class PythonQtShell_InternetService : public InternetService
{
public:
    PythonQtShell_InternetService(const QString&  name, InternetModel*  model, QObject*  parent = NULL):InternetService(name, model, parent),_wrapper(NULL) {};

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

class PythonQtPublicPromoter_InternetService : public InternetService
{ public:
inline QWidget*  promoted_HeaderWidget() const { return InternetService::HeaderWidget(); }
inline QString  promoted_Icon() { return InternetService::Icon(); }
inline void promoted_ItemDoubleClicked(QStandardItem*  item) { InternetService::ItemDoubleClicked(item); }
inline void promoted_ReloadSettings() { InternetService::ReloadSettings(); }
inline void promoted_ShowContextMenu(const QModelIndex&  index, const QPoint&  global_pos) { InternetService::ShowContextMenu(index, global_pos); }
};

class PythonQtWrapper_InternetService : public QObject
{ Q_OBJECT
public:
public slots:
InternetService* new_InternetService(const QString&  name, InternetModel*  model, QObject*  parent = NULL);
void delete_InternetService(InternetService* obj) { delete obj; } 
   QWidget*  HeaderWidget(InternetService* theWrappedObject) const;
   QString  Icon(InternetService* theWrappedObject);
   void ItemDoubleClicked(InternetService* theWrappedObject, QStandardItem*  item);
   void ReloadSettings(InternetService* theWrappedObject);
   void ShowContextMenu(InternetService* theWrappedObject, const QModelIndex&  index, const QPoint&  global_pos);
   InternetModel*  model(InternetService* theWrappedObject) const;
   QString  name(InternetService* theWrappedObject) const;
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
  Type_Unknown = Song::Type_Unknown,   Type_Asf = Song::Type_Asf,   Type_Flac = Song::Type_Flac,   Type_Mp4 = Song::Type_Mp4,   Type_Mpc = Song::Type_Mpc,   Type_Mpeg = Song::Type_Mpeg,   Type_OggFlac = Song::Type_OggFlac,   Type_OggSpeex = Song::Type_OggSpeex,   Type_OggVorbis = Song::Type_OggVorbis,   Type_Aiff = Song::Type_Aiff,   Type_Wav = Song::Type_Wav,   Type_TrueAudio = Song::Type_TrueAudio,   Type_Cdda = Song::Type_Cdda,   Type_Stream = Song::Type_Stream};
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
   bool  is_cdda(Song* theWrappedObject) const;
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





class PythonQtShell_SongInsertVetoListener : public SongInsertVetoListener
{
public:
    PythonQtShell_SongInsertVetoListener():SongInsertVetoListener(),_wrapper(NULL) {};

virtual QList<Song >  AboutToInsertSongs(const QList<Song >&  old_songs, const QList<Song >&  new_songs);
virtual void childEvent(QChildEvent*  arg__1);
virtual void customEvent(QEvent*  arg__1);
virtual bool  event(QEvent*  arg__1);
virtual bool  eventFilter(QObject*  arg__1, QEvent*  arg__2);
virtual void timerEvent(QTimerEvent*  arg__1);

  PythonQtInstanceWrapper* _wrapper; 
};

class PythonQtWrapper_SongInsertVetoListener : public QObject
{ Q_OBJECT
public:
public slots:
SongInsertVetoListener* new_SongInsertVetoListener();
void delete_SongInsertVetoListener(SongInsertVetoListener* obj) { delete obj; } 
};





class PythonQtShell_SpecialPlaylistType : public SpecialPlaylistType
{
public:
    PythonQtShell_SpecialPlaylistType():SpecialPlaylistType(),_wrapper(NULL) {};

virtual void DidYouMeanClicked(const QString&  text, Playlist*  playlist);
virtual void Search(const QString&  text, Playlist*  playlist);
virtual QString  empty_playlist_text(Playlist*  playlist) const;
virtual bool  has_special_search_behaviour(Playlist*  playlist) const;
virtual QIcon  icon(Playlist*  playlist) const;
virtual QString  name() const;
virtual QString  search_hint_text(Playlist*  playlist) const;

  PythonQtInstanceWrapper* _wrapper; 
};

class PythonQtPublicPromoter_SpecialPlaylistType : public SpecialPlaylistType
{ public:
inline void promoted_DidYouMeanClicked(const QString&  text, Playlist*  playlist) { SpecialPlaylistType::DidYouMeanClicked(text, playlist); }
inline void promoted_Search(const QString&  text, Playlist*  playlist) { SpecialPlaylistType::Search(text, playlist); }
inline QString  promoted_empty_playlist_text(Playlist*  playlist) const { return SpecialPlaylistType::empty_playlist_text(playlist); }
inline bool  promoted_has_special_search_behaviour(Playlist*  playlist) const { return SpecialPlaylistType::has_special_search_behaviour(playlist); }
inline QIcon  promoted_icon(Playlist*  playlist) const { return SpecialPlaylistType::icon(playlist); }
inline QString  promoted_search_hint_text(Playlist*  playlist) const { return SpecialPlaylistType::search_hint_text(playlist); }
};

class PythonQtWrapper_SpecialPlaylistType : public QObject
{ Q_OBJECT
public:
public slots:
SpecialPlaylistType* new_SpecialPlaylistType();
void delete_SpecialPlaylistType(SpecialPlaylistType* obj) { delete obj; } 
   void DidYouMeanClicked(SpecialPlaylistType* theWrappedObject, const QString&  text, Playlist*  playlist);
   void Search(SpecialPlaylistType* theWrappedObject, const QString&  text, Playlist*  playlist);
   QString  empty_playlist_text(SpecialPlaylistType* theWrappedObject, Playlist*  playlist) const;
   bool  has_special_search_behaviour(SpecialPlaylistType* theWrappedObject, Playlist*  playlist) const;
   QIcon  icon(SpecialPlaylistType* theWrappedObject, Playlist*  playlist) const;
   QString  search_hint_text(SpecialPlaylistType* theWrappedObject, Playlist*  playlist) const;
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


