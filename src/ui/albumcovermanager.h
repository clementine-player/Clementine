/* This file is part of Clementine.
   Copyright 2010, David Sansome <me@davidsansome.com>

   Clementine is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   Clementine is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with Clementine.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef ALBUMCOVERMANAGER_H
#define ALBUMCOVERMANAGER_H

#include <QMainWindow>
#include <QIcon>
#include <QModelIndex>

#include "gtest/gtest_prod.h"

#include "core/song.h"
#include "covers/albumcoverloaderoptions.h"
#include "covers/coversearchstatistics.h"

class AlbumCoverChoiceController;
class AlbumCoverExport;
class AlbumCoverExporter;
class AlbumCoverFetcher;
class AlbumCoverSearcher;
class Application;
class LibraryBackend;
class SongMimeData;
class Ui_CoverManager;

class QListWidgetItem;
class QMenu;
class QNetworkAccessManager;
class QPushButton;
class QProgressBar;

class AlbumCoverManager : public QMainWindow {
  Q_OBJECT
 public:
  AlbumCoverManager(Application* app, LibraryBackend* library_backend,
                    QWidget* parent = nullptr,
                    QNetworkAccessManager* network = 0);
  ~AlbumCoverManager();

  static const char* kSettingsGroup;

  LibraryBackend* backend() const;
  QIcon no_cover_icon() const { return no_cover_icon_; }

  void Reset();
  void Init();

  void EnableCoversButtons();
  void DisableCoversButtons();

  SongList GetSongsInAlbum(const QModelIndex& index) const;
  SongList GetSongsInAlbums(const QModelIndexList& indexes) const;
  SongMimeData* GetMimeDataForAlbums(const QModelIndexList& indexes) const;

signals:
  void AddToPlaylist(QMimeData* data);

 protected:
  void showEvent(QShowEvent*);
  void closeEvent(QCloseEvent*);

  // For the album view context menu events
  bool eventFilter(QObject* obj, QEvent* event);

 private slots:
  void ArtistChanged(QListWidgetItem* current);
  void CoverImageLoaded(quint64 id, const QImage& image);
  void UpdateFilter();
  void FetchAlbumCovers();
  void ExportCovers();
  void AlbumCoverFetched(quint64 id, const QImage& image,
                         const CoverSearchStatistics& statistics);
  void CancelRequests();

  // On the context menu
  void FetchSingleCover();

  void LoadCoverFromFile();
  void SaveCoverToFile();
  void LoadCoverFromURL();
  void SearchForCover();
  void UnsetCover();
  void ShowCover();

  // For adding albums to the playlist
  void AlbumDoubleClicked(const QModelIndex& index);
  void AddSelectedToPlaylist();
  void LoadSelectedToPlaylist();

  void UpdateCoverInList(QListWidgetItem* item, const QString& cover);
  void UpdateExportStatus(int exported, int bad, int count);

 private:
  enum ArtistItemType {
    All_Artists,
    Various_Artists,
    Specific_Artist,
  };

  enum Role {
    Role_ArtistName = Qt::UserRole + 1,
    Role_AlbumArtistName,
    Role_AlbumName,
    Role_PathAutomatic,
    Role_PathManual,
    Role_FirstUrl,
  };

  enum HideCovers {
    Hide_None,
    Hide_WithCovers,
    Hide_WithoutCovers,
  };

  QString InitialPathForOpenCoverDialog(const QString& path_automatic,
                                        const QString& first_file_name) const;

  QString EffectiveAlbumArtistName(const QListWidgetItem& item) const;

  // Returns the selected element in form of a Song ready to be used
  // by AlbumCoverChoiceController or invalid song if there's nothing
  // or multiple elements selected.
  Song GetSingleSelectionAsSong();
  // Returns the first of the selected elements in form of a Song ready
  // to be used by AlbumCoverChoiceController or invalid song if there's nothing
  // selected.
  Song GetFirstSelectedAsSong();

  Song ItemAsSong(QListWidgetItem* item);

  void UpdateStatusText();
  bool ShouldHide(const QListWidgetItem& item, const QString& filter,
                  HideCovers hide) const;
  void SaveAndSetCover(QListWidgetItem* item, const QImage& image);

 private:
  Ui_CoverManager* ui_;
  Application* app_;

  AlbumCoverChoiceController* album_cover_choice_controller_;

  QAction* filter_all_ = nullptr;
  QAction* filter_with_covers_ = nullptr;
  QAction* filter_without_covers_ = nullptr;

  AlbumCoverLoaderOptions cover_loader_options_;
  QMap<quint64, QListWidgetItem*> cover_loading_tasks_;

  AlbumCoverFetcher* cover_fetcher_;
  QMap<quint64, QListWidgetItem*> cover_fetching_tasks_;
  CoverSearchStatistics fetch_statistics_;

  AlbumCoverSearcher* cover_searcher_;
  AlbumCoverExport* cover_export_;
  AlbumCoverExporter* cover_exporter_;

  QImage GenerateNoCoverImage(const QIcon& no_cover_icon) const;
  bool ItemHasCover(const QListWidgetItem& item) const;

  QIcon artist_icon_;
  QIcon all_artists_icon_;
  const QIcon no_cover_icon_;
  const QImage no_cover_image_;
  const QIcon no_cover_item_icon_;

  QMenu* context_menu_;
  QList<QListWidgetItem*> context_menu_items_;

  QProgressBar* progress_bar_;
  QPushButton* abort_progress_;
  int jobs_;

  LibraryBackend* library_backend_;

  FRIEND_TEST(AlbumCoverManagerTest, HidesItemsWithCover);
  FRIEND_TEST(AlbumCoverManagerTest, HidesItemsWithoutCover);
  FRIEND_TEST(AlbumCoverManagerTest, HidesItemsWithFilter);
};

#endif  // ALBUMCOVERMANAGER_H
