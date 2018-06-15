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

#ifndef ALBUMCOVERCHOICECONTROLLER_H
#define ALBUMCOVERCHOICECONTROLLER_H

#include <QAction>
#include <QList>
#include <QMenu>
#include <QWidget>

class AlbumCoverFetcher;
class AlbumCoverSearcher;
class Application;
class CoverFromURLDialog;
class QFileDialog;
class Song;

struct CoverSearchStatistics;

// Controller for the common album cover related menu options.
class AlbumCoverChoiceController : public QWidget {
  Q_OBJECT

 public:
  static const char* kLoadImageFileFilter;
  static const char* kSaveImageFileFilter;
  static const char* kAllFilesFilter;

  AlbumCoverChoiceController(QWidget* parent = nullptr);
  ~AlbumCoverChoiceController();

  void SetApplication(Application* app);

  // Getters for all QActions implemented by this controller.

  QAction* cover_from_file_action() const { return cover_from_file_; }
  QAction* cover_to_file_action() const { return cover_to_file_; }
  QAction* cover_from_url_action() const { return cover_from_url_; }
  QAction* search_for_cover_action() const { return search_for_cover_; }
  QAction* unset_cover_action() const { return unset_cover_; }
  QAction* show_cover_action() const { return show_cover_; }
  QAction* search_cover_auto_action() const { return search_cover_auto_; }

  // Returns QAction* for every operation implemented by this controller.
  // The list contains QAction* for:
  // 1. loading cover from file
  // 2. loading cover from URL
  // 3. searching for cover using last.fm
  // 4. unsetting the cover manually
  // 5. showing the cover in original size
  QList<QAction*> GetAllActions();

  // All of the methods below require a currently selected song as an
  // input parameter. Also - LoadCoverFromFile, LoadCoverFromURL,
  // SearchForCover, UnsetCover and SaveCover all update manual path
  // of the given song in library to the new cover.

  // Lets the user choose a cover from disk. If no cover will be chosen or the
  // chosen
  // cover will not be a proper image, this returns an empty string. Otherwise,
  // the
  // path to the chosen cover will be returned.
  QString LoadCoverFromFile(Song* song);

  // Shows a dialog that allows user to save the given image on disk. The image
  // is supposed to be the cover of the given song's album.
  void SaveCoverToFile(const Song& song, const QImage& image);

  // Downloads the cover from an URL given by user. This returns the downloaded
  // image
  // or null image if something went wrong for example when user cancelled the
  // dialog.
  QString LoadCoverFromURL(Song* song);

  // Lets the user choose a cover among all that have been found on last.fm.
  // Returns the chosen cover or null cover if user didn't choose anything.
  QString SearchForCover(Song* song);

  // Returns a path which indicates that the cover has been unset manually.
  QString UnsetCover(Song* song);

  // Shows the cover of given song in it's original size.
  void ShowCover(const Song& song);
  bool ToggleCover(const Song& song);

  // Search for covers automatically
  void SearchCoverAutomatically(const Song& song);

  // Saves the chosen cover as manual cover path of this song in library.
  void SaveCover(Song* song, const QString& cover);

  // Saves the cover that the user picked through a drag and drop operation.
  QString SaveCover(Song* song, const QDropEvent* e);

  // Saves the given image in cache as a cover for 'artist' - 'album'.
  // The method returns path of the cached image.
  QString SaveCoverInCache(const QString& artist, const QString& album,
                           const QImage& image);

  static bool CanAcceptDrag(const QDragEnterEvent* e);

 signals:
  void AutomaticCoverSearchDone();

 private slots:
  void AlbumCoverFetched(quint64 id, const QImage& image,
                         const CoverSearchStatistics& statistics);
  void AlbumCoverPopupClosed();

 private:
  QDialog* ShowCoverPrivate(const Song& song);
  QString GetInitialPathForFileDialog(const Song& song,
                                      const QString& filename);

  static bool IsKnownImageExtension(const QString& suffix);
  static QSet<QString>* sImageExtensions;

  Application* app_;
  AlbumCoverSearcher* cover_searcher_;
  AlbumCoverFetcher* cover_fetcher_;

  QFileDialog* save_file_dialog_;
  CoverFromURLDialog* cover_from_url_dialog_;

  QAction* cover_from_file_;
  QAction* cover_to_file_;
  QAction* separator_;
  QAction* cover_from_url_;
  QAction* search_for_cover_;
  QAction* unset_cover_;
  QAction* show_cover_;
  QAction* search_cover_auto_;

  QMap<quint64, Song> cover_fetching_tasks_;

  QDialog* album_cover_popup_;
};

#endif  // ALBUMCOVERCHOICECONTROLLER_H
