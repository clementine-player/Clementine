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
#include "ui/albumcoverchoicecontroller.h"
#include "ui/albumcovermanager.h"
#include "ui/albumcoversearcher.h"
#include "ui/coverfromurldialog.h"
#include "ui/iconloader.h"

#include <QDialog>
#include <QFileDialog>
#include <QLabel>

const char* AlbumCoverChoiceController::kImageFileFilter =
  QT_TR_NOOP("Images (*.png *.jpg *.jpeg *.bmp *.gif *.xpm *.pbm *.pgm *.ppm *.xbm *.tiff)");
const char* AlbumCoverChoiceController::kAllFilesFilter =
  QT_TR_NOOP("All files (*)");

AlbumCoverChoiceController::AlbumCoverChoiceController(QWidget* parent)
  : QWidget(parent),
#ifdef HAVE_LIBLASTFM
    cover_searcher_(new AlbumCoverSearcher(QIcon(":/nocover.png"), this)),
    cover_fetcher_(new AlbumCoverFetcher(this)),
#endif
    cover_from_url_dialog_(NULL)
{
#ifdef HAVE_LIBLASTFM
  cover_searcher_->Init(cover_fetcher_);
#endif
}

AlbumCoverChoiceController::~AlbumCoverChoiceController()
{
  if(cover_from_url_dialog_) {
    delete cover_from_url_dialog_;
  }
}

QString AlbumCoverChoiceController::LoadCoverFromFile(const Song& song) {
#ifdef HAVE_LIBLASTFM
  QString dir;

  if (!song.art_automatic().isEmpty() && song.art_automatic() != AlbumCoverLoader::kEmbeddedCover) {
    dir = song.art_automatic();
  } else if (!song.filename().isEmpty() && song.filename().contains('/')) {
    // we get rid of the filename because it's extension is screwing with the dialog's
    // filters
    dir = song.filename().section('/', 0, -2);
  } else {
    dir = "";
  }

  QString cover = QFileDialog::getOpenFileName(
      this, tr("Choose manual cover"), dir,
      tr(kImageFileFilter) + ";;" + tr(kAllFilesFilter));

  if (cover.isNull())
    return "";

  // Can we load the image?
  QImage image(cover);
  if (image.isNull())
    return "";

  return cover;
#else
  return "";
#endif
}

QImage AlbumCoverChoiceController::LoadCoverFromURL() {
  if(!cover_from_url_dialog_) {
    cover_from_url_dialog_ = new CoverFromURLDialog(this);
  }

  QImage image = cover_from_url_dialog_->Exec();
  return image;
}

QImage AlbumCoverChoiceController::SearchForCover(const Song& song) const {
#ifdef HAVE_LIBLASTFM
  // Get something sensible to stick in the search box
  QString query = song.artist();
  if (!query.isEmpty())
    query += " ";
  query += song.album();

  QImage image = cover_searcher_->Exec(query);
  return image;
#else
  return QImage();
#endif
}

QString AlbumCoverChoiceController::UnsetCover() const {
  return AlbumCoverLoader::kManuallyUnsetCover;
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
