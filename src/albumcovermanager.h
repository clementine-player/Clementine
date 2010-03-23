#ifndef ALBUMCOVERMANAGER_H
#define ALBUMCOVERMANAGER_H

#include <QDialog>
#include <QIcon>

#include <boost/shared_ptr.hpp>

#include "ui_albumcovermanager.h"
#include "backgroundthread.h"
#include "albumcoverloader.h"

class LibraryBackendInterface;
class AlbumCoverFetcher;

class QNetworkAccessManager;

class AlbumCoverManager : public QDialog {
  Q_OBJECT
 public:
  AlbumCoverManager(QNetworkAccessManager* network, QWidget *parent = 0);
  ~AlbumCoverManager();

  static const char* kSettingsGroup;

  void Reset();

 public slots:
  void SetBackend(boost::shared_ptr<LibraryBackendInterface> backend);

 protected:
  void showEvent(QShowEvent *);
  void closeEvent(QCloseEvent *);
  bool event(QEvent *);

  // For the album view context menu events
  bool eventFilter(QObject *obj, QEvent *event);

 private slots:
  void ArtistChanged(QListWidgetItem* current);
  void CoverLoaderInitialised();
  void CoverImageLoaded(quint64 id, const QImage& image);
  void UpdateFilter();
  void FetchAlbumCovers();
  void AlbumCoverFetched(quint64 id, const QImage& image);

  // On the context menu
  void ShowFullsize();
  void FetchSingleCover();
  void ChooseManualCover();
  void UnsetCover();

 private:
  enum ArtistItemType {
    All_Artists,
    Various_Artists,
    Specific_Artist,
  };

  enum Role {
    Role_ArtistName = Qt::UserRole + 1,
    Role_AlbumName,
    Role_PathAutomatic,
    Role_PathManual,
  };

  void CancelRequests();

 private:
  bool constructed_;

  Ui::CoverManager ui_;
  boost::shared_ptr<LibraryBackendInterface> backend_;

  QAction* filter_all_;
  QAction* filter_with_covers_;
  QAction* filter_without_covers_;

  BackgroundThread<AlbumCoverLoader>* cover_loader_;
  QMap<quint64, QListWidgetItem*> cover_loading_tasks_;

  AlbumCoverFetcher* cover_fetcher_;
  QMap<quint64, QListWidgetItem*> cover_fetching_tasks_;

  QIcon artist_icon_;
  QIcon all_artists_icon_;
  QIcon no_cover_icon_;

  QMenu* context_menu_;
  QList<QListWidgetItem*> context_menu_items_;
};

#endif // ALBUMCOVERMANAGER_H
