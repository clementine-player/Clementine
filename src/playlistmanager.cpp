#include "playlistmanager.h"
#include "playlistview.h"
#include "playlist.h"

#include <QAbstractItemView>
#include <QSettings>
#include <QDebug>

PlaylistManager::PlaylistManager(QObject* parent): QObject(parent),
pTabWidget_( NULL ),
pCurrentPlaylist_(NULL),
pCurrentPlaylistView_(NULL),
playlistCount_(0)
{
}
void PlaylistManager::addPlaylist(const QString & playlistName /*= QString()*/){
  Q_ASSERT ( pTabWidget_ ) ; 
  PlaylistView * playListView = new PlaylistView(pTabWidget_);
  
  playListView->setObjectName(QString::fromUtf8("playlist"));
  playListView->setAcceptDrops(true);
  playListView->setDragEnabled(true);
  playListView->setDragDropMode(QAbstractItemView::DragDrop);
  playListView->setSelectionMode(QAbstractItemView::ExtendedSelection);
  playListView->setRootIsDecorated(false);
  playListView->setUniformRowHeights(true);
  playListView->setItemsExpandable(false);
  playListView->setSortingEnabled(true);
  playListView->setAllColumnsShowFocus(true);
  
  Playlist* playList = new Playlist(playListView) ;
  
  playList->IgnoreSorting(true);
  playListView->setModel(playList);
  playList->IgnoreSorting(false);
    
  QString title = playList->GetTitle(); 
  playlistCount_++ ; 
  if ( title.isEmpty() ) 
      title = tr("New playlist") +  QString::number(playlistCount_) ; 
  playList->SetPlaylistIndex( playlistCount_) ; 
  playList->SetTitle(title);
  pTabWidget_->addTab(playListView, title);
  
  SetCurrentPlaylist(playList);
  pCurrentPlaylistView_ = playListView ; 
  
  playlists_ << playList ; 
}
void PlaylistManager::SetCurrentPlaylist(Playlist* pPlaylist){
  pCurrentPlaylist_ = pPlaylist ; 
  
  emit CurrentPlaylistChanged( pCurrentPlaylist_ ) ; 
}
void PlaylistManager::Save() const{
  QSettings s ; 
  Q_FOREACH ( Playlist* p, playlists_ ) {
    qDebug() << "Saving" << p->GetTitle() ; 
    p->SaveR() ; 
  }
  s.setValue("numberofplaylists", playlistCount_ ) ; 
}
bool PlaylistManager::Restore(){
  Q_ASSERT ( pTabWidget_ ) ; 
  QSettings s ; 
  bool bOk ; 
  int nOfPlaylist = s.value("numberofplaylists").toInt(&bOk) ; 
  playlistCount_ = nOfPlaylist ;
  qDebug() << nOfPlaylist ; 
  if ( bOk == false || nOfPlaylist == 0 ) {
    qDebug()<< "No reading from settings"; 
    return false; 
  }
  int nCurrentIndex = -1 ;
  for ( int i=0;i<nOfPlaylist;++i){
    
    PlaylistView* playListView = new PlaylistView(pTabWidget_);
  
    playListView->setObjectName(QString::fromUtf8("playlist"));
    playListView->setAcceptDrops(true);
    playListView->setDragEnabled(true);
    playListView->setDragDropMode(QAbstractItemView::DragDrop);
    playListView->setSelectionMode(QAbstractItemView::ExtendedSelection);
    playListView->setRootIsDecorated(false);
    playListView->setUniformRowHeights(true);
    playListView->setItemsExpandable(false);
    playListView->setSortingEnabled(true);
    playListView->setAllColumnsShowFocus(true);
  
    Playlist* playList = new Playlist(playListView) ;
  
    playList->IgnoreSorting(true);
    playListView->setModel(playList);
    playList->IgnoreSorting(false);
    
    playList->SetPlaylistIndex( i+1 ) ; 
    playList->RestoreR() ; 
    
    pTabWidget_->addTab( playListView, playList->GetTitle() );
  
    SetCurrentPlaylist(playList);
    pCurrentPlaylistView_ = playListView ;
    playlists_ << playList ; 
  }
  return true ; 
}



