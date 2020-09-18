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

#include "ui/albumcoverchoicecontroller.h"

#include <QAction>
#include <QDialog>
#include <QDragEnterEvent>
#include <QFileDialog>
#include <QGuiApplication>
#include <QImageWriter>
#include <QLabel>
#include <QList>
#include <QMenu>
#include <QMimeData>
#include <QScreen>
#include <QUrl>
#include <QWindow>

#include "core/application.h"
#include "core/logging.h"
#include "core/utilities.h"
#include "covers/albumcoverfetcher.h"
#include "covers/albumcoverloader.h"
#include "covers/currentartloader.h"
#include "library/librarybackend.h"
#include "ui/albumcovermanager.h"
#include "ui/albumcoversearcher.h"
#include "ui/coverfromurldialog.h"
#include "ui/iconloader.h"

const char* AlbumCoverChoiceController::kLoadImageFileFilter = QT_TR_NOOP(
    "Images (*.png *.jpg *.jpeg *.bmp *.gif *.xpm *.pbm *.pgm *.ppm *.xbm)");
const char* AlbumCoverChoiceController::kSaveImageFileFilter =
    QT_TR_NOOP("Images (*.png *.jpg *.jpeg *.bmp *.xpm *.pbm *.ppm *.xbm)");
const char* AlbumCoverChoiceController::kAllFilesFilter =
    QT_TR_NOOP("All files (*)");

QSet<QString>* AlbumCoverChoiceController::sImageExtensions = nullptr;

AlbumCoverChoiceController::AlbumCoverChoiceController(QWidget* parent)
    : QWidget(parent),
      app_(nullptr),
      cover_searcher_(nullptr),
      cover_fetcher_(nullptr),
      save_file_dialog_(nullptr),
      cover_from_url_dialog_(nullptr),
      album_cover_popup_(nullptr) {
  cover_from_file_ =
      new QAction(IconLoader::Load("document-open", IconLoader::Base),
                  tr("Load cover from disk..."), this);
  cover_to_file_ =
      new QAction(IconLoader::Load("document-save", IconLoader::Base),
                  tr("Save cover to disk..."), this);
  cover_from_url_ = new QAction(IconLoader::Load("download", IconLoader::Base),
                                tr("Load cover from URL..."), this);
  search_for_cover_ =
      new QAction(IconLoader::Load("edit-find", IconLoader::Base),
                  tr("Search for album covers..."), this);
  unset_cover_ = new QAction(IconLoader::Load("list-remove", IconLoader::Base),
                             tr("Unset cover"), this);
  show_cover_ = new QAction(IconLoader::Load("zoom-in", IconLoader::Base),
                            tr("Show fullsize..."), this);

  search_cover_auto_ = new QAction(tr("Search automatically"), this);
  search_cover_auto_->setCheckable(true);
  search_cover_auto_->setChecked(false);

  separator_ = new QAction(this);
  separator_->setSeparator(true);
}

AlbumCoverChoiceController::~AlbumCoverChoiceController() {}

void AlbumCoverChoiceController::SetApplication(Application* app) {
  app_ = app;

  cover_fetcher_ = new AlbumCoverFetcher(app_->cover_providers(), this);
  cover_searcher_ = new AlbumCoverSearcher(QIcon(":/nocover.png"), app, this);
  cover_searcher_->Init(cover_fetcher_);

  connect(cover_fetcher_,
          SIGNAL(AlbumCoverFetched(quint64, QImage, CoverSearchStatistics)),
          this,
          SLOT(AlbumCoverFetched(quint64, QImage, CoverSearchStatistics)));
}

QList<QAction*> AlbumCoverChoiceController::GetAllActions() {
  return QList<QAction*>() << cover_from_file_ << cover_to_file_ << separator_
                           << cover_from_url_ << search_for_cover_
                           << unset_cover_ << show_cover_;
}

QString AlbumCoverChoiceController::LoadCoverFromFile(Song* song) {
  QString cover = QFileDialog::getOpenFileName(
      this, tr("Load cover from disk"),
      GetInitialPathForFileDialog(*song, QString()),
      tr(kLoadImageFileFilter) + ";;" + tr(kAllFilesFilter));

  if (cover.isNull()) return QString();

  // Can we load the image?
  QImage image(cover);

  if (!image.isNull()) {
    SaveCover(song, cover);
    return cover;
  } else {
    return QString();
  }
}

void AlbumCoverChoiceController::SaveCoverToFile(const Song& song,
                                                 const QImage& image) {
  QString initial_file_name =
      "/" +
      (song.effective_album().isEmpty() ? tr("Unknown")
                                        : song.effective_album()) +
      ".jpg";

  QString save_filename = QFileDialog::getSaveFileName(
      this, tr("Save album cover"),
      GetInitialPathForFileDialog(song, initial_file_name),
      tr(kSaveImageFileFilter) + ";;" + tr(kAllFilesFilter));

  if (save_filename.isNull()) return;

  QString extension = save_filename.right(4);
  if (!extension.startsWith('.') ||
      !QImageWriter::supportedImageFormats().contains(
          extension.right(3).toUtf8())) {
    save_filename.append(".jpg");
  }

  image.save(save_filename);
}

QString AlbumCoverChoiceController::GetInitialPathForFileDialog(
    const Song& song, const QString& filename) {
  // art automatic is first to show user which cover the album may be
  // using now; the song is using it if there's no manual path but we
  // cannot use manual path here because it can contain cached paths
  if (!song.art_automatic().isEmpty() && !song.has_embedded_cover()) {
    return song.art_automatic();

    // if no automatic art, start in the song's folder
  } else if (!song.url().isEmpty() && song.url().toLocalFile().contains('/')) {
    return song.url().toLocalFile().section('/', 0, -2) + filename;

    // fallback - start in home
  } else {
    return QDir::home().absolutePath() + filename;
  }
}

QString AlbumCoverChoiceController::LoadCoverFromURL(Song* song) {
  if (!cover_from_url_dialog_) {
    cover_from_url_dialog_ = new CoverFromURLDialog(this);
  }

  QImage image = cover_from_url_dialog_->Exec();

  if (!image.isNull()) {
    QString cover = SaveCoverInCache(song->artist(), song->album(), image);
    SaveCover(song, cover);

    return cover;
  } else {
    return QString();
  }
}

QString AlbumCoverChoiceController::SearchForCover(Song* song) {
  // Get something sensible to stick in the search box.
  // We search for the 'effective' values, but we cache the covers with the
  // Song's artist() and album().
  QImage image = cover_searcher_->Exec(song->effective_albumartist(),
                                       song->effective_album());

  if (!image.isNull()) {
    QString cover = SaveCoverInCache(song->artist(), song->album(), image);
    SaveCover(song, cover);

    return cover;
  } else {
    return QString();
  }
}

QString AlbumCoverChoiceController::UnsetCover(Song* song) {
  QString cover = Song::kManuallyUnsetCover;
  SaveCover(song, cover);

  return cover;
}

bool AlbumCoverChoiceController::ToggleCover(const Song& song) {
  if (album_cover_popup_ != nullptr) {
    album_cover_popup_->accept();
    album_cover_popup_ = nullptr;
    return false;
  }

  album_cover_popup_ = ShowCoverPrivate(song);

  // keep track of our window to prevent endless stacking
  connect(album_cover_popup_, SIGNAL(finished(int)), this,
          SLOT(AlbumCoverPopupClosed()));

  return true;
}

void AlbumCoverChoiceController::ShowCover(const Song& song) {
  ShowCoverPrivate(song);
}

QDialog* AlbumCoverChoiceController::ShowCoverPrivate(const Song& song) {
  QDialog* dialog = new QDialog(this);
  dialog->setAttribute(Qt::WA_DeleteOnClose, true);

  // Use (Album)Artist - Album as the window title
  QString title_text(song.effective_albumartist());
  if (!song.effective_album().isEmpty())
    title_text += " - " + song.effective_album();

  QLabel* label = new QLabel(dialog);
  label->setPixmap(AlbumCoverLoader::TryLoadPixmap(
      song.art_automatic(), song.art_manual(), song.url().toLocalFile()));

  // add (WxHpx) to the title before possibly resizing
  title_text += " (" + QString::number(label->pixmap()->width()) + "x" +
                QString::number(label->pixmap()->height()) + "px)";

  // if the cover is larger than the screen, resize the window
  // 85% seems to be enough to account for title bar and taskbar etc.
#if (QT_VERSION >= QT_VERSION_CHECK(5, 14, 0))
  QScreen* screen = QWidget::screen();
#else
  QScreen* screen =
      (window() && window()->windowHandle() ? window()->windowHandle()->screen()
                                            : QGuiApplication::primaryScreen());
#endif

  QRect screenGeometry = screen->availableGeometry();
  int desktop_height = screenGeometry.height();
  int desktop_width = screenGeometry.width();

  // resize differently if monitor is in portrait mode
  if (desktop_width < desktop_height) {
    const int new_width = (double)desktop_width * 0.95;
    if (new_width < label->pixmap()->width()) {
      label->setPixmap(
          label->pixmap()->scaledToWidth(new_width, Qt::SmoothTransformation));
    }
  } else {
    const int new_height = (double)desktop_height * 0.85;
    if (new_height < label->pixmap()->height()) {
      label->setPixmap(label->pixmap()->scaledToHeight(
          new_height, Qt::SmoothTransformation));
    }
  }

  dialog->setWindowTitle(title_text);
  dialog->setFixedSize(label->pixmap()->size());
  dialog->show();

  return dialog;
}

void AlbumCoverChoiceController::AlbumCoverPopupClosed() {
  album_cover_popup_ = nullptr;
}

void AlbumCoverChoiceController::SearchCoverAutomatically(const Song& song) {
  qint64 id = cover_fetcher_->FetchAlbumCover(song.effective_albumartist(),
                                              song.effective_album(), false);
  cover_fetching_tasks_[id] = song;
}

void AlbumCoverChoiceController::AlbumCoverFetched(
    quint64 id, const QImage& image, const CoverSearchStatistics& statistics) {
  Song song;
  if (cover_fetching_tasks_.contains(id)) {
    song = cover_fetching_tasks_.take(id);
  }

  if (!image.isNull()) {
    QString cover = SaveCoverInCache(song.artist(), song.album(), image);
    SaveCover(&song, cover);
  }

  emit AutomaticCoverSearchDone();
}

void AlbumCoverChoiceController::SaveCover(Song* song, const QString& cover) {
  if (song->is_valid() && song->id() != -1) {
    song->set_art_manual(cover);
    app_->library_backend()->UpdateManualAlbumArtAsync(
        song->artist(), song->albumartist(), song->album(), cover);

    if (song->url() == app_->current_art_loader()->last_song().url()) {
      app_->current_art_loader()->LoadArt(*song);
    }
  }
}

QString AlbumCoverChoiceController::SaveCoverInCache(const QString& artist,
                                                     const QString& album,
                                                     const QImage& image) {
  // Hash the artist and album into a filename for the image
  QString filename(Utilities::Sha1CoverHash(artist, album).toHex() + ".jpg");
  QString path(AlbumCoverLoader::ImageCacheDir() + "/" + filename);

  // Make sure this directory exists first
  QDir dir;
  dir.mkdir(AlbumCoverLoader::ImageCacheDir());

  // Save the image to disk
  image.save(path, "JPG");

  return path;
}

bool AlbumCoverChoiceController::IsKnownImageExtension(const QString& suffix) {
  if (!sImageExtensions) {
    sImageExtensions = new QSet<QString>();
    (*sImageExtensions) << "png"
                        << "jpg"
                        << "jpeg"
                        << "bmp"
                        << "gif"
                        << "xpm"
                        << "pbm"
                        << "pgm"
                        << "ppm"
                        << "xbm";
  }

  return sImageExtensions->contains(suffix);
}

bool AlbumCoverChoiceController::CanAcceptDrag(const QDragEnterEvent* e) {
  for (const QUrl& url : e->mimeData()->urls()) {
    const QString suffix = QFileInfo(url.toLocalFile()).suffix().toLower();
    if (IsKnownImageExtension(suffix)) return true;
  }
  return e->mimeData()->hasImage();
}

QString AlbumCoverChoiceController::SaveCover(Song* song, const QDropEvent* e) {
  for (const QUrl& url : e->mimeData()->urls()) {
    const QString filename = url.toLocalFile();
    const QString suffix = QFileInfo(filename).suffix().toLower();

    if (IsKnownImageExtension(suffix)) {
      SaveCover(song, filename);
      return filename;
    }
  }

  if (e->mimeData()->hasImage()) {
    QImage image = qvariant_cast<QImage>(e->mimeData()->imageData());
    if (!image.isNull()) {
      QString cover_path =
          SaveCoverInCache(song->artist(), song->album(), image);
      SaveCover(song, cover_path);
      return cover_path;
    }
  }

  return QString();
}
