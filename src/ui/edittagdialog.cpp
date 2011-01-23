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

#include "albumcovermanager.h"
#include "edittagdialog.h"
#include "trackselectiondialog.h"
#include "ui_edittagdialog.h"
#include "core/albumcoverloader.h"
#include "core/utilities.h"
#include "library/library.h"
#include "library/librarybackend.h"
#include "playlist/playlistdelegates.h"
#include "ui/coverfromurldialog.h"

#ifdef HAVE_LIBLASTFM
# include "albumcoversearcher.h"
# include "core/albumcoverfetcher.h"
#endif

#include <QDateTime>
#include <QFileDialog>
#include <QFuture>
#include <QFutureWatcher>
#include <QLabel>
#include <QMenu>
#include <QMessageBox>
#include <QPushButton>
#include <QShortcut>
#include <QtConcurrentRun>
#include <QtDebug>

const char* EditTagDialog::kHintText = QT_TR_NOOP("(different across multiple songs)");
const char* EditTagDialog::kTagFetchText = QT_TR_NOOP("Complete tags automatically");
const char* EditTagDialog::kTagFetchOnLoadText = QT_TR_NOOP("Generating audio fingerprint and fetching results...");

EditTagDialog::EditTagDialog(QWidget* parent)
  : QDialog(parent),
    ui_(new Ui_EditTagDialog),
    cover_from_url_dialog_(NULL),
    backend_(NULL),
    loading_(false),
    ignore_edits_(false),
#ifdef HAVE_LIBTUNEPIMP
    tag_fetcher_(new TagFetcher()),
#endif
#ifdef HAVE_LIBLASTFM
    cover_searcher_(new AlbumCoverSearcher(QIcon(":/nocover.png"), this)),
    cover_fetcher_(new AlbumCoverFetcher(this)),
#endif
    cover_loader_(new BackgroundThreadImplementation<AlbumCoverLoader, AlbumCoverLoader>(this)),
    cover_art_id_(0),
    cover_art_is_set_(false),
    resultsDialog_(new TrackSelectionDialog(this))
{
  cover_loader_->Start(true);
  cover_loader_->Worker()->SetDefaultOutputImage(QImage(":nocover.png"));
  connect(cover_loader_->Worker().get(), SIGNAL(ImageLoaded(quint64,QImage)),
          SLOT(ArtLoaded(quint64,QImage)));
#ifdef HAVE_LIBTUNEPIMP
  connect(tag_fetcher_, SIGNAL(FetchFinished(QString, SongList)),
          this, SLOT(FetchTagFinished(QString, SongList)), Qt::QueuedConnection);
  connect(resultsDialog_, SIGNAL(SongChoosen(QString, Song)), SLOT(FetchTagSongChoosen(QString, Song)));
#endif

#ifdef HAVE_LIBLASTFM
  cover_searcher_->Init(cover_fetcher_);
#endif

  ui_->setupUi(this);
  ui_->splitter->setSizes(QList<int>() << 200 << width() - 200);
  ui_->loading_container->hide();
#ifdef HAVE_LIBTUNEPIMP
  ui_->fetch_tag->setText(tr(kTagFetchText));
#else
  ui_->fetch_tag->setVisible(false);
#endif

  // An editable field is one that has a label as a buddy.  The label is
  // important because it gets turned bold when the field is changed.
  foreach (QLabel* label, findChildren<QLabel*>()) {
    QWidget* widget = label->buddy();
    if (widget) {
      // Store information about the field
      fields_ << FieldData(label, widget, widget->objectName());

      // Connect the Reset signal
      if (dynamic_cast<ExtendedEditor*>(widget)) {
        connect(widget, SIGNAL(Reset()), SLOT(ResetField()));
      }

      // Connect the edited signal
      if (qobject_cast<QLineEdit*>(widget)) {
        connect(widget, SIGNAL(textChanged(QString)), SLOT(FieldValueEdited()));
      } else if (qobject_cast<QPlainTextEdit*>(widget)) {
        connect(widget, SIGNAL(textChanged()), SLOT(FieldValueEdited()));
      } else if (qobject_cast<QSpinBox*>(widget)) {
        connect(widget, SIGNAL(valueChanged(int)), SLOT(FieldValueEdited()));
      }
    }
  }

  // Set the colour of all the labels on the summary page
  const bool light = palette().color(QPalette::Base).value() > 128;
  const QColor color = palette().color(QPalette::Dark);
  QPalette summary_label_palette(palette());
  summary_label_palette.setColor(QPalette::WindowText,
      light ? color.darker(150) : color.lighter(125));

  foreach (QLabel* label, ui_->summary_tab->findChildren<QLabel*>()) {
    if (label->property("field_label").toBool()) {
      label->setPalette(summary_label_palette);
    }
  }

  // Pretend the summary text is just a label
  ui_->summary->setMaximumHeight(ui_->art->height() - ui_->summary_art_button->height() - 4);

  connect(ui_->song_list->selectionModel(),
          SIGNAL(selectionChanged(QItemSelection,QItemSelection)),
          SLOT(SelectionChanged()));
  connect(ui_->button_box, SIGNAL(clicked(QAbstractButton*)),
                           SLOT(ButtonClicked(QAbstractButton*)));
  connect(ui_->rating, SIGNAL(RatingChanged(float)),
                       SLOT(SongRated(float)));
  connect(ui_->playcount_reset, SIGNAL(clicked()), SLOT(ResetPlayCounts()));
#ifdef HAVE_LIBTUNEPIMP
  connect(ui_->fetch_tag, SIGNAL(clicked()), SLOT(FetchTag()));
  ui_->fetch_tag->setText(tr(kTagFetchText));
#endif

  // Set up the album cover menu
  cover_menu_ = new QMenu(this);
  choose_cover_ = cover_menu_->addAction(
        IconLoader::Load("document-open"), tr("Load cover from disk..."),
        this, SLOT(LoadCoverFromFile()));
  download_cover_ = cover_menu_->addAction(
        IconLoader::Load("download"), tr("Load cover from URL..."),
        this, SLOT(LoadCoverFromURL()));
  search_for_cover_ = cover_menu_->addAction(
        IconLoader::Load("find"), tr("Search for album covers..."),
        this, SLOT(SearchCover()));
  unset_cover_ = cover_menu_->addAction(
        IconLoader::Load("list-remove"), tr("Unset cover"),
        this, SLOT(UnsetCover()));
  show_cover_ = cover_menu_->addAction(
        IconLoader::Load("zoom-in"), tr("Show fullsize..."),
        this, SLOT(ZoomCover()));
  ui_->summary_art_button->setMenu(cover_menu_);

  ui_->art->installEventFilter(this);

  // Add the next/previous buttons
  previous_button_ = new QPushButton(IconLoader::Load("go-previous"), tr("Previous"), this);
  next_button_ = new QPushButton(IconLoader::Load("go-next"), tr("Next"), this);
  ui_->button_box->addButton(previous_button_, QDialogButtonBox::ResetRole);
  ui_->button_box->addButton(next_button_, QDialogButtonBox::ResetRole);

  connect(previous_button_, SIGNAL(clicked()), SLOT(PreviousSong()));
  connect(next_button_, SIGNAL(clicked()), SLOT(NextSong()));

  // Set some shortcuts for the buttons
  new QShortcut(QKeySequence::Back, previous_button_, SLOT(click()));
  new QShortcut(QKeySequence::Forward, next_button_, SLOT(click()));
  new QShortcut(QKeySequence::MoveToPreviousPage, previous_button_, SLOT(click()));
  new QShortcut(QKeySequence::MoveToNextPage, next_button_, SLOT(click()));
}

EditTagDialog::~EditTagDialog() {
#ifdef HAVE_LIBTUNEPIMP
  delete tag_fetcher_;
# endif
  delete ui_;
  if(cover_from_url_dialog_) {
    delete cover_from_url_dialog_;
  }
}

bool EditTagDialog::SetLoading(const QString& message) {
  const bool loading = !message.isEmpty();
  if (loading == loading_)
    return false;
  loading_ = loading;

  ui_->loading_container->setVisible(loading);
  ui_->button_box->setEnabled(!loading);
  ui_->tab_widget->setEnabled(!loading);
  ui_->song_list->setEnabled(!loading);
  ui_->fetch_tag->setEnabled(!loading);
  ui_->loading_label->setText(message);
  return true;
}

QList<EditTagDialog::Data> EditTagDialog::LoadData(const SongList& songs) const {
  QList<Data> ret;

  foreach (const Song& song, songs) {
    if (song.IsEditable()) {
      // Try reloading the tags from file
      Song copy(song);
      copy.InitFromFile(copy.filename(), copy.directory_id());

      if (copy.is_valid())
        ret << Data(copy);
    }
  }

  return ret;
}

void EditTagDialog::SetSongs(const SongList& s, const PlaylistItemList& items) {
  // Show the loading indicator
  if (!SetLoading(tr("Loading tracks") + "..."))
    return;

  data_.clear();
  playlist_items_ = items;
  ui_->song_list->clear();

  // Reload tags in the background
  QFuture<QList<Data> > future = QtConcurrent::run(this, &EditTagDialog::LoadData, s);
  QFutureWatcher<QList<Data> >* watcher = new QFutureWatcher<QList<Data> >(this);
  watcher->setFuture(future);
  connect(watcher, SIGNAL(finished()), SLOT(SetSongsFinished()));
}

void EditTagDialog::SetSongsFinished() {
  QFutureWatcher<QList<Data> >* watcher = dynamic_cast<QFutureWatcher<QList<Data> >*>(sender());
  if (!watcher)
    return;
  watcher->deleteLater();

  if (!SetLoading(QString()))
    return;

  data_ = watcher->result();
  if (data_.count() == 0)
    return;

  // Add the filenames to the list
  foreach (const Data& data, data_) {
    ui_->song_list->addItem(data.current_.basefilename());
  }

  // Select all
  ui_->song_list->selectAll();

  // Hide the list if there's only one song in it
  const bool multiple = data_.count() != 1;
  ui_->song_list->setVisible(multiple);
  previous_button_->setEnabled(multiple);
  next_button_->setEnabled(multiple);

  // Display tag fetcher if there's only one song
  ui_->fetch_tag->setVisible(!multiple);

  ui_->tab_widget->setCurrentWidget(multiple ? ui_->tags_tab : ui_->summary_tab);
}

void EditTagDialog::SetTagCompleter(LibraryBackend* backend) {
  backend_ = backend;
  new TagCompleter(backend, Playlist::Column_Artist, ui_->artist);
  new TagCompleter(backend, Playlist::Column_Album, ui_->album);
  new TagCompleter(backend, Playlist::Column_AlbumArtist, ui_->albumartist);
  new TagCompleter(backend, Playlist::Column_Genre, ui_->genre);
  new TagCompleter(backend, Playlist::Column_Composer, ui_->composer);
}

QVariant EditTagDialog::Data::value(const Song& song, const QString& id) {
  if (id == "title")       return song.title();
  if (id == "artist")      return song.artist();
  if (id == "album")       return song.album();
  if (id == "albumartist") return song.albumartist();
  if (id == "composer")    return song.composer();
  if (id == "genre")       return song.genre();
  if (id == "comment")     return song.comment();
  if (id == "track")       return song.track();
  if (id == "disc")        return song.disc();
  if (id == "year")        return song.year();
  qDebug() << "Unknown ID" << id;
  return QVariant();
}

void EditTagDialog::Data::set_value(const QString& id, const QVariant& value) {
  if (id == "title")       current_.set_title(value.toString());
  if (id == "artist")      current_.set_artist(value.toString());
  if (id == "album")       current_.set_album(value.toString());
  if (id == "albumartist") current_.set_albumartist(value.toString());
  if (id == "composer")    current_.set_composer(value.toString());
  if (id == "genre")       current_.set_genre(value.toString());
  if (id == "comment")     current_.set_comment(value.toString());
  if (id == "track")       current_.set_track(value.toInt());
  if (id == "disc")        current_.set_disc(value.toInt());
  if (id == "year")        current_.set_year(value.toInt());
}

bool EditTagDialog::DoesValueVary(const QModelIndexList& sel, const QString& id) const {
  QVariant value = data_[sel.first().row()].current_value(id);
  for (int i=1 ; i<sel.count() ; ++i) {
    if (value != data_[sel[i].row()].current_value(id))
      return true;
  }
  return false;
}

bool EditTagDialog::IsValueModified(const QModelIndexList& sel, const QString& id) const {
  foreach (const QModelIndex& i, sel) {
    if (data_[i.row()].original_value(id) != data_[i.row()].current_value(id))
      return true;
  }
  return false;
}

void EditTagDialog::InitFieldValue(const FieldData& field, const QModelIndexList& sel) {
  const bool varies = DoesValueVary(sel, field.id_);
  const bool modified = IsValueModified(sel, field.id_);

  if (ExtendedEditor* editor = dynamic_cast<ExtendedEditor*>(field.editor_)) {
    editor->clear();
    editor->clear_hint();
    if (varies) {
      editor->set_hint(EditTagDialog::kHintText);
    } else {
      editor->set_text(data_[sel[0].row()].current_value(field.id_).toString());
    }
  }

  QFont new_font(font());
  new_font.setBold(modified);
  field.label_->setFont(new_font);
  field.editor_->setFont(new_font);
}

void EditTagDialog::UpdateFieldValue(const FieldData& field, const QModelIndexList& sel) {
  // Get the value from the field
  QVariant value;
  if (ExtendedEditor* editor = dynamic_cast<ExtendedEditor*>(field.editor_)) {
    value = editor->text();
  }

  // Did we get it?
  if (!value.isValid()) {
    return;
  }

  // Set it in each selected song
  foreach (const QModelIndex& i, sel) {
    data_[i.row()].set_value(field.id_, value);
  }

  // Update the boldness
  const bool modified = IsValueModified(sel, field.id_);

  QFont new_font(font());
  new_font.setBold(modified);
  field.label_->setFont(new_font);
  field.editor_->setFont(new_font);
}

void EditTagDialog::ResetFieldValue(const FieldData& field, const QModelIndexList& sel) {
  // Reset each selected song
  foreach (const QModelIndex& i, sel) {
    Data& data = data_[i.row()];
    data.set_value(field.id_, data.original_value(field.id_));
  }

  // Reset the field
  InitFieldValue(field, sel);
}

void EditTagDialog::SelectionChanged() {
  const QModelIndexList sel = ui_->song_list->selectionModel()->selectedIndexes();
  if (sel.isEmpty())
    return;

  // Set the editable fields
  ignore_edits_ = true;
  foreach (const FieldData& field, fields_) {
    InitFieldValue(field, sel);
  }
  ignore_edits_ = false;

  // If we're editing multiple songs then we have to disable certain tabs
  const bool multiple = sel.count() > 1;
  ui_->tab_widget->setTabEnabled(ui_->tab_widget->indexOf(ui_->summary_tab), !multiple);

  if (!multiple) {
    const Song& song = data_[sel.first().row()].original_;
    UpdateSummaryTab(song);
    UpdateStatisticsTab(song);
  }
}

static void SetText(QLabel* label, int value, const QString& suffix, const QString& def = QString()) {
  label->setText(value <= 0 ? def : (QString::number(value) + " " + suffix));
}

void EditTagDialog::UpdateSummaryTab(const Song& song) {
  cover_art_id_ = cover_loader_->Worker()->LoadImageAsync(song);

  QString summary = "<b>" + Qt::escape(song.PrettyTitleWithArtist()) + "</b><br/>";

  bool art_is_set = true;
  if (song.art_manual() == AlbumCoverLoader::kManuallyUnsetCover) {
    summary += Qt::escape(tr("Cover art manually unset"));
    art_is_set = false;
  } else if (!song.art_manual().isEmpty()) {
    summary += Qt::escape(tr("Cover art set from %1").arg(song.art_manual()));
  } else if (song.art_automatic() == AlbumCoverLoader::kEmbeddedCover) {
    summary += Qt::escape(tr("Cover art from embedded image"));
  } else if (!song.art_automatic().isEmpty()) {
    summary += Qt::escape(tr("Cover art loaded automatically from %1").arg(song.art_manual()));
  } else {
    summary += Qt::escape(tr("Cover art not set"));
    art_is_set = false;
  }

  ui_->summary->setText(summary);

#ifndef HAVE_LIBLASTFM
  choose_cover_->setEnabled(false);
  search_for_cover_->setEnabled(false);
#endif

  unset_cover_->setEnabled(art_is_set);
  show_cover_->setEnabled(art_is_set);
  ui_->summary_art_button->setEnabled(song.id() != -1);

  ui_->length->setText(Utilities::PrettyTime(song.length()));
  SetText(ui_->bpm, song.bpm(), tr("bpm"));
  SetText(ui_->samplerate, song.samplerate(), "Hz");
  SetText(ui_->bitrate, song.bitrate(), tr("kbps"));
  ui_->mtime->setText(QDateTime::fromTime_t(song.mtime()).toString(
        QLocale::system().dateTimeFormat(QLocale::LongFormat)));
  ui_->ctime->setText(QDateTime::fromTime_t(song.ctime()).toString(
        QLocale::system().dateTimeFormat(QLocale::LongFormat)));
  ui_->filesize->setText(Utilities::PrettySize(song.filesize()));
  ui_->filetype->setText(song.TextForFiletype());
  ui_->filename->setText(QDir::toNativeSeparators(song.filename()));
}

void EditTagDialog::UpdateStatisticsTab(const Song& song) {
  ui_->playcount->setText(QString::number(qMax(0, song.playcount())));
  ui_->skipcount->setText(QString::number(qMax(0, song.skipcount())));
  ui_->score->setText(QString::number(qMax(0, song.score())));
  ui_->rating->set_rating(song.rating());

  ui_->lastplayed->setText(song.lastplayed() <= 0 ? tr("Never") :
      QDateTime::fromTime_t(song.lastplayed()).toString(
          QLocale::system().dateTimeFormat(QLocale::LongFormat)));
}

void EditTagDialog::ArtLoaded(quint64 id, const QImage& image) {
  if (id == cover_art_id_) {
    ui_->art->setPixmap(QPixmap::fromImage(image));
  }
}

void EditTagDialog::FieldValueEdited() {
  if (ignore_edits_)
    return;

  const QModelIndexList sel = ui_->song_list->selectionModel()->selectedIndexes();
  if (sel.isEmpty())
    return;

  QWidget* w = qobject_cast<QWidget*>(sender());

  // Find the field
  foreach (const FieldData& field, fields_) {
    if (field.editor_ == w) {
      UpdateFieldValue(field, sel);
      return;
    }
  }
}

void EditTagDialog::ResetField() {
  const QModelIndexList sel = ui_->song_list->selectionModel()->selectedIndexes();
  if (sel.isEmpty())
    return;

  QWidget* w = qobject_cast<QWidget*>(sender());

  // Find the field
  foreach (const FieldData& field, fields_) {
    if (field.editor_ == w) {
      ignore_edits_ = true;
      ResetFieldValue(field, sel);
      ignore_edits_ = false;
      return;
    }
  }
}

void EditTagDialog::LoadCoverFromFile() {
#ifdef HAVE_LIBLASTFM
  const QModelIndexList sel = ui_->song_list->selectionModel()->selectedIndexes();
  if (sel.isEmpty())
    return;
  const Song& song = data_[sel.first().row()].original_;

  // Figure out the initial path.  Logic copied from
  // AlbumCoverManager::InitialPathForOpenCoverDialog
  QString dir;
  if (!song.art_automatic().isEmpty() && song.art_automatic() != AlbumCoverLoader::kEmbeddedCover) {
    dir = song.art_automatic();
  } else {
    dir = song.filename().section('/', 0, -1);
  }

  QString cover = QFileDialog::getOpenFileName(
      this, tr("Choose manual cover"), dir,
      tr(AlbumCoverManager::kImageFileFilter) + ";;" + tr(AlbumCoverManager::kAllFilesFilter));
  if (cover.isNull())
    return;

  // Can we load the image?
  QImage image(cover);
  if (image.isNull())
    return;

  // Update database
  SetAlbumArt(cover);
#endif
}

void EditTagDialog::LoadCoverFromURL() {
  // TODO: duplication
  const QModelIndexList sel = ui_->song_list->selectionModel()->selectedIndexes();
  if (sel.isEmpty())
    return;
  const Song& song = data_[sel.first().row()].original_;

  if(!cover_from_url_dialog_) {
    cover_from_url_dialog_ = new CoverFromURLDialog(this);
  }

  QImage image = cover_from_url_dialog_->Exec();
  if (image.isNull())
    return;

  SetAlbumArt(AlbumCoverManager::SaveCoverInCache(song.artist(), song.album(), image));
}

void EditTagDialog::SearchCover() {
#ifdef HAVE_LIBLASTFM
  const QModelIndexList sel = ui_->song_list->selectionModel()->selectedIndexes();
  if (sel.isEmpty())
    return;
  const Song& song = data_[sel.first().row()].original_;

  // Get something sensible to stick in the search box
  QString query = song.artist();
  if (!query.isEmpty())
    query += " ";
  query += song.album();

  QImage image = cover_searcher_->Exec(query);
  if (image.isNull())
    return;

  SetAlbumArt(AlbumCoverManager::SaveCoverInCache(song.artist(), song.album(), image));
#endif
}

void EditTagDialog::UnsetCover() {
  SetAlbumArt(AlbumCoverLoader::kManuallyUnsetCover);
}

void EditTagDialog::ZoomCover() {
  const QModelIndexList sel = ui_->song_list->selectionModel()->selectedIndexes();
  if (sel.isEmpty())
    return;
  const Song& song = data_[sel.first().row()].original_;

  QDialog* dialog = new QDialog(this);
  dialog->setAttribute(Qt::WA_DeleteOnClose, true);
  dialog->setWindowTitle(song.title());

  QLabel* label = new QLabel(dialog);
  label->setPixmap(AlbumCoverLoader::TryLoadPixmap(
      song.art_automatic(), song.art_manual(), song.filename()));

  dialog->resize(label->pixmap()->size());
  dialog->show();
}

void EditTagDialog::SetAlbumArt(const QString& path) {
  const QModelIndexList sel = ui_->song_list->selectionModel()->selectedIndexes();
  if (sel.isEmpty())
    return;
  Song* song = &data_[sel.first().row()].original_;
  if (!song->is_valid() || song->id() == -1)
    return;

  song->set_art_manual(path);
  backend_->UpdateManualAlbumArtAsync(song->artist(), song->album(), path);
  UpdateSummaryTab(*song);

  // Now check if we have any other songs cached that share that artist and
  // album (and would therefore be changed as well)
  for (int i=0 ; i<data_.count() ; ++i) {
    if (i == sel.first().row()) // Already changed this one
      continue;

    Song* other_song = &data_[i].original_;
    if (song->artist() == other_song->artist() &&
        song->album()  == other_song->album()) {
      other_song->set_art_manual(path);
    }
  }
}

void EditTagDialog::NextSong() {
  int row = (ui_->song_list->currentRow() + 1) % ui_->song_list->count();
  ui_->song_list->setCurrentRow(row);
}

void EditTagDialog::PreviousSong() {
  int row = (ui_->song_list->currentRow() - 1 + ui_->song_list->count()) % ui_->song_list->count();
  ui_->song_list->setCurrentRow(row);
}

void EditTagDialog::ButtonClicked(QAbstractButton* button) {
  if (button == ui_->button_box->button(QDialogButtonBox::Discard)) {
    reject();
  }
}

void EditTagDialog::SaveData(const QList<Data>& data) {
  for (int i=0 ; i<data.count() ; ++i) {
    const Data& ref = data[i];
    if (ref.current_.IsMetadataEqual(ref.original_))
      continue;

    if (!ref.current_.Save()) {
      emit Error(tr("An error occurred writing metadata to '%1'").arg(ref.current_.filename()));
    }
  }
}

void EditTagDialog::accept() {
  // Show the loading indicator
  if (!SetLoading(tr("Saving tracks") + "..."))
    return;

  // Save tags in the background
  QFuture<void> future = QtConcurrent::run(this, &EditTagDialog::SaveData, data_);
  QFutureWatcher<void>* watcher = new QFutureWatcher<void>(this);
  watcher->setFuture(future);
  connect(watcher, SIGNAL(finished()), SLOT(AcceptFinished()));
}

void EditTagDialog::AcceptFinished() {
  QFutureWatcher<void>* watcher = dynamic_cast<QFutureWatcher<void>*>(sender());
  if (!watcher)
    return;
  watcher->deleteLater();

  if (!SetLoading(QString()))
    return;

  QDialog::accept();
}

bool EditTagDialog::eventFilter(QObject* o, QEvent* e) {
  if (o == ui_->art && e->type() == QEvent::MouseButtonRelease) {
    cover_menu_->popup(static_cast<QMouseEvent*>(e)->globalPos());
  }
  return false;
}

void EditTagDialog::showEvent(QShowEvent* e) {
  // Set the dialog's height to the smallest possible
  resize(width(), sizeHint().height());

  QDialog::showEvent(e);
}

void EditTagDialog::SongRated(float rating) {
  const QModelIndexList sel = ui_->song_list->selectionModel()->selectedIndexes();
  if (sel.isEmpty())
    return;
  Song* song = &data_[sel.first().row()].original_;
  if (!song->is_valid() || song->id() == -1)
    return;

  song->set_rating(rating);
  backend_->UpdateSongRatingAsync(song->id(), rating);
}

void EditTagDialog::ResetPlayCounts() {
  const QModelIndexList sel = ui_->song_list->selectionModel()->selectedIndexes();
  if (sel.isEmpty())
    return;
  Song* song = &data_[sel.first().row()].original_;
  if (!song->is_valid() || song->id() == -1)
    return;

  if (QMessageBox::question(this, tr("Reset play counts"),
        tr("Are you sure you want to reset this song's statistics?"),
        QMessageBox::Reset, QMessageBox::Cancel) != QMessageBox::Reset) {
    return;
  }

  song->set_playcount(0);
  song->set_skipcount(0);
  song->set_lastplayed(-1);
  song->set_score(0);
  backend_->ResetStatisticsAsync(song->id());
  UpdateStatisticsTab(*song);
}

void EditTagDialog::FetchTag() {
#ifdef HAVE_LIBTUNEPIMP
  const QModelIndexList sel = ui_->song_list->selectionModel()->selectedIndexes();
  if (sel.isEmpty())
    return;
  Song* song = &data_[sel.first().row()].original_;
  if (!song->is_valid() || song->id() == -1)
    return;
  tag_fetcher_->FetchFromFile(song->filename());
  ui_->fetch_tag->setDisabled(true); // disable button, will be re-enabled later
  ui_->fetch_tag->setText(tr(kTagFetchOnLoadText));
  QApplication::setOverrideCursor(QCursor(Qt::BusyCursor));
#endif
}

void EditTagDialog::FetchTagFinished(const QString& filename, const SongList& songs_guessed) {
#ifdef HAVE_LIBTUNEPIMP
  // Restore cursor
  QApplication::restoreOverrideCursor();

  // Get current song
  const QModelIndexList sel = ui_->song_list->selectionModel()->selectedIndexes();
  if (sel.isEmpty())
    return;
  Song* current_song = &data_[sel.first().row()].original_;

  // Check if the user has closed edit tag dialog or if it has reopened a new one.
  // If so, ignore this signal; else open a dialog for selecting best result
  if(isVisible() && current_song->filename() == filename) {
    // Restore some components as "no working"
    ui_->fetch_tag->setDisabled(false);
    ui_->fetch_tag->setText(tr(kTagFetchText));

    // If no songs have been guessed, just display a message
    if(songs_guessed.empty()) {
      QMessageBox messageBox(this);
      messageBox.setWindowTitle(tr("Sorry"));
      messageBox.setText(tr("Clementine was unable to find results for this file"));
      messageBox.exec();
    } else {  // Else, display song's tags selection dialog only if edittagdialog is still opened
      resultsDialog_->Init(filename, songs_guessed);
      resultsDialog_->show();
    }
  }
#endif
}

void EditTagDialog::FetchTagSongChoosen(const QString& filename, const Song& song_choosen) {
  // Get current song
  const QModelIndexList sel = ui_->song_list->selectionModel()->selectedIndexes();
  if (sel.isEmpty())
    return;
  Song* current_song = &data_[sel.first().row()].original_;
  if (!current_song->is_valid() || current_song->id() == -1)
    return;
  // Check it's still the same song, using filename
  if(filename != current_song->filename()) {
    // different song: we shouldn't erase tags with wrong metadata
    return;
  }

  if(song_choosen.title() != "")
    ui_->title->set_text(song_choosen.title());
  if(song_choosen.album() != "")
    ui_->album->set_text(song_choosen.album());
  if(song_choosen.album() != "")
    ui_->artist->set_text(song_choosen.artist());
  if(song_choosen.track() > 0)
    ui_->track->set_text(QString::number(song_choosen.track()));
}

