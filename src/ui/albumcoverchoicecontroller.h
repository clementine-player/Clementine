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
#include <QWidget>

class AlbumCoverFetcher;
class AlbumCoverSearcher;
class CoverFromURLDialog;
class LibraryBackend;
class Song;

// Controller for the common album cover related menu options.
class AlbumCoverChoiceController : public QWidget {
  Q_OBJECT

 public:
  AlbumCoverChoiceController(LibraryBackend* library, QWidget* parent = 0);
  ~AlbumCoverChoiceController();

  static const char* kImageFileFilter;
  static const char* kAllFilesFilter;

  // Returns QAction* for every operation implemented by this controller.
  // The list contains QAction* for:
  // 1. loading cover from file
  // 2. loading cover from URL
  // 3. searching for cover using last.fm
  // 4. unsetting the cover manually
  // 5. showing the cover in original size
  QList<QAction*> PrepareAlbumChoiceMenu(QObject* parent);

  // All of the methods below require a currently selected song as an
  // input parameter. Also - LoadCoverFromFile, LoadCoverFromURL,
  // SearchForCover, UnsetCover and SaveCover all update manual path 
  // of the given song in library to the new cover.

  // Lets the user choose a cover from disk. If no cover will be chosen or the chosen
  // cover will not be a proper image, this returns an empty string. Otherwise, the
  // path to the chosen cover will be returned.
  QString LoadCoverFromFile(Song* song);

  // Downloads the cover from an URL given by user. This returns the downloaded image
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

  // Saves the chosen cover as manual cover path of this song in library.
  void SaveCover(Song* song, const QString& cover);

  // Saves the given image in cache as a cover for 'artist' - 'album'. 
  // The method returns path of the cached image.
  QString SaveCoverInCache(const QString& artist, const QString& album, const QImage& image);

private:

#ifdef HAVE_LIBLASTFM
  AlbumCoverSearcher* cover_searcher_;
  AlbumCoverFetcher* cover_fetcher_;
#endif

  CoverFromURLDialog* cover_from_url_dialog_;

  LibraryBackend* library_;
};

#endif // ALBUMCOVERCHOICECONTROLLER_H
