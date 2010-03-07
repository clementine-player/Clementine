#ifndef PLAYLISTMANAGER_H
#define PLAYLISTMANAGER_H

#include <QtCore/QObject>

class PlaylistView;
class QTabWidget;
class Playlist;

class PlaylistManager : public QObject{
  Q_OBJECT
  public:
    PlaylistManager(QObject* parent = 0);
    void addPlaylist( const QString& playlistName = QString() ) ; 
    
    // accessors
    void SetTabWidget( QTabWidget * pWidget ) { pTabWidget_ = pWidget ; } 
    QTabWidget* GetTabWidget() const {return pTabWidget_; }
    
    void SetCurrentPlaylist ( Playlist * pPlaylist ) ; 
    
    void Save() const ; 
    bool Restore() ; 
    
  private:
    QList<Playlist*> playlists_ ; 
    QTabWidget* pTabWidget_ ; 
    QList<QString> playlistNames_ ; 
    Playlist* pCurrentPlaylist_ ; 
    PlaylistView* pCurrentPlaylistView_ ; 
    
    int playlistCount_ ; 
  signals:
    void CurrentPlaylistChanged( Playlist* pPlaylist ) ; 
};

#endif // PLAYLISTMANAGER_H
