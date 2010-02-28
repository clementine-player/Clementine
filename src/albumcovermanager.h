#ifndef ALBUMCOVERMANAGER_H
#define ALBUMCOVERMANAGER_H

#include <QDialog>
#include <QIcon>

#include <boost/shared_ptr.hpp>

#include "ui_albumcovermanager.h"
#include "backgroundthread.h"
#include "albumcoverloader.h"

class LibraryBackend;

class AlbumCoverManager : public QDialog {
  Q_OBJECT
 public:
  AlbumCoverManager(QWidget *parent = 0);

  static const char* kSettingsGroup;

  void Reset();

 public slots:
  void SetBackend(boost::shared_ptr<LibraryBackend> backend);

 protected:
  void showEvent(QShowEvent *);
  void closeEvent(QCloseEvent *);

 private slots:
  void ArtistChanged(QListWidgetItem* current);
  void CoverLoaderInitialised();
  void CoverImageLoaded(quint64 id, const QImage& image);
  void UpdateFilter();

 private:
  enum ArtistItemType {
    All_Artists,
    Specific_Artist,
  };

 private:
  Ui::CoverManager ui_;
  boost::shared_ptr<LibraryBackend> backend_;

  QAction* filter_all_;
  QAction* filter_with_covers_;
  QAction* filter_without_covers_;

  BackgroundThread<AlbumCoverLoader>* cover_loader_;
  QMap<quint64, QListWidgetItem*> cover_loading_tasks_;

  QIcon artist_icon_;
  QIcon all_artists_icon_;
  QIcon no_cover_icon_;
};

#endif // ALBUMCOVERMANAGER_H
