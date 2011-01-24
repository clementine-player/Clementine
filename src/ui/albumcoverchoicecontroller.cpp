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

#include "core/albumcoverfetcher.h"
#include "core/albumcoverloader.h"
#include "library/librarybackend.h"
#include "ui/albumcoverchoicecontroller.h"
#include "ui/albumcovermanager.h"
#include "ui/albumcoversearcher.h"
#include "ui/coverfromurldialog.h"
#include "ui/iconloader.h"

#include <QCryptographicHash>
#include <QDialog>
#include <QFileDialog>
#include <QLabel>

const char* AlbumCoverChoiceController::kImageFileFilter =
  QT_TR_NOOP("Images (*.png *.jpg *.jpeg *.bmp *.gif *.xpm *.pbm *.pgm *.ppm *.xbm *.tiff)");
const char* AlbumCoverChoiceController::kAllFilesFilter =
  QT_TR_NOOP("All files (*)");

AlbumCoverChoiceController::AlbumCoverChoiceController(LibraryBackend* library, QWidget* parent)
  : QWidget(parent),
#ifdef HAVE_LIBLASTFM
    cover_searcher_(new AlbumCoverSearcher(QIcon(":/nocover.png"), this)),
    cover_fetcher_(new AlbumCoverFetcher(this)),
#endif
    cover_from_url_dialog_(NULL),
    library_(library)
{
#ifdef HAVE_LIBLASTFM
  cover_searcher_->Init(cover_fetcher_);
#endif
}

AlbumCoverChoiceController::~AlbumCoverChoiceController()
{
}

QString AlbumCoverChoiceController::LoadCoverFromFile(Song* song) {
  QString dir;

  if (!song->art_automatic().isEmpty() && song->art_automatic() != AlbumCoverLoader::kEmbeddedCover) {
    dir = song->art_automatic();
  } else if (!song->filename().isEmpty() && song->filename().contains('/')) {
    // we get rid of the filename because it's extension is screwing with the dialog's
    // filters
    dir = song->filename().section('/', 0, -2);
  } else {
    dir = "";
  }

  QString cover = QFileDialog::getOpenFileName(
      this, tr("Choose manual cover"), dir,
      tr(kImageFileFilter) + ";;" + tr(kAllFilesFilter));

  if (cover.isNull())
    return QString();

  // Can we load the image?
  QImage image(cover);

  if(!image.isNull()) {
    SaveCover(song, cover);
    return cover;
  } else {
    return QString();
  }
}

QString AlbumCoverChoiceController::LoadCoverFromURL(Song* song) {
  if(!cover_from_url_dialog_) {
    cover_from_url_dialog_ = new CoverFromURLDialog(this);
  }

  QImage image = cover_from_url_dialog_->Exec();

  if(!image.isNull()) {
    QString cover = SaveCoverInCache(song->artist(), song->album(), image);
    SaveCover(song, cover);

    return cover;
  } else {
    return QString();
  }
}

QString AlbumCoverChoiceController::SearchForCover(Song* song) {
#ifdef HAVE_LIBLASTFM
  // Get something sensible to stick in the search box
  QString query = song->artist();
  if (!query.isEmpty())
    query += " ";
  query += song->album();

  QImage image = cover_searcher_->Exec(query);

  if(!image.isNull()) {
    QString cover = SaveCoverInCache(song->artist(), song->album(), image);
    SaveCover(song, cover);

    return cover;
  } else {
    return QString();
  }
#else
  return QString();
#endif
}

QString AlbumCoverChoiceController::UnsetCover(Song* song) {
  QString cover = AlbumCoverLoader::kManuallyUnsetCover;
  SaveCover(song, cover);

  return cover;
}

void AlbumCoverChoiceController::ShowCover(const Song& song) {
  QDialog* dialog = new QDialog(this);
  dialog->setAttribute(Qt::WA_DeleteOnClose, true);
  dialog->setWindowTitle(song.title());

  QLabel* label = new QLabel(dialog);
  label->setPixmap(AlbumCoverLoader::TryLoadPixmap(
      song.art_automatic(), song.art_manual(), song.filename()));

  dialog->resize(label->pixmap()->size());
  dialog->show();
}

void AlbumCoverChoiceController::SaveCover(Song* song, const QString &cover) {
  if(song->is_valid() && song->id() != -1) {
    song->set_art_manual(cover);
    library_->UpdateManualAlbumArtAsync(song->artist(), song->album(), cover);
  }
}

QString AlbumCoverChoiceController::SaveCoverInCache(
    const QString& artist, const QString& album, const QImage& image) {

  // Hash the artist and album into a filename for the image
  QCryptographicHash hash(QCryptographicHash::Sha1);
  hash.addData(artist.toLower().toUtf8().constData());
  hash.addData(album.toLower().toUtf8().constData());

  QString filename = hash.result().toHex() + ".jpg";
  QString path = AlbumCoverLoader::ImageCacheDir() + "/" + filename;

  // Make sure this directory exists first
  QDir dir;
  dir.mkdir(AlbumCoverLoader::ImageCacheDir());

  // Save the image to disk
  image.save(path, "JPG");

  return path;

}
