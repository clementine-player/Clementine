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

#include "ui/edittagdialog.h"
#include "ui_edittagdialog.h"

#include <limits>

#include <QDateTime>
#include <QDir>
#include <QFuture>
#include <QLabel>
#include <QMenu>
#include <QMessageBox>
#include <QPushButton>
#include <QShortcut>
#include <QtConcurrentRun>
#include <QtDebug>

#include "core/application.h"
#include "core/logging.h"
#include "core/tagreaderclient.h"
#include "core/utilities.h"
#include "covers/albumcoverloader.h"
#include "covers/coverproviders.h"
#include "library/librarybackend.h"
#include "library/library.h"
#include "playlist/playlistdelegates.h"
#include "ui/albumcoverchoicecontroller.h"
#include "ui/albumcovermanager.h"
#include "ui/coverfromurldialog.h"
#include "ui/iconloader.h"
#include "ui/trackselectiondialog.h"

const char* EditTagDialog::kHintText =
    QT_TR_NOOP("(different across multiple songs)");
const char* EditTagDialog::kSettingsGroup = "EditTagDialog";

EditTagDialog::EditTagDialog(Application* app, QWidget* parent)
    : QDialog(parent),
      ui_(new Ui_EditTagDialog),
      app_(app),
      album_cover_choice_controller_(new AlbumCoverChoiceController(this)),
      loading_(false),
      ignore_edits_(false),
      tag_fetcher_(new TagFetcher(this)),
      cover_art_id_(0),
      cover_art_is_set_(false),
      results_dialog_(new TrackSelectionDialog(this)) {
  QIcon nocover = IconLoader::Load("nocover", IconLoader::Other);
  cover_options_.default_output_image_ =
      AlbumCoverLoader::ScaleAndPad(cover_options_,
          nocover.pixmap(nocover.availableSizes().last())
                 .toImage());

  connect(app_->album_cover_loader(),
          SIGNAL(ImageLoaded(quint64, QImage, QImage)),
          SLOT(ArtLoaded(quint64, QImage, QImage)));

  connect(tag_fetcher_, SIGNAL(ResultAvailable(Song, SongList)),
          results_dialog_, SLOT(FetchTagFinished(Song, SongList)),
          Qt::QueuedConnection);
  connect(tag_fetcher_, SIGNAL(Progress(Song, QString)), results_dialog_,
          SLOT(FetchTagProgress(Song, QString)));
  connect(results_dialog_, SIGNAL(SongChosen(Song, Song)),
          SLOT(FetchTagSongChosen(Song, Song)));
  connect(results_dialog_, SIGNAL(finished(int)), tag_fetcher_, SLOT(Cancel()));

  album_cover_choice_controller_->SetApplication(app_);

  ui_->setupUi(this);
  ui_->splitter->setSizes(QList<int>() << 200 << width() - 200);
  ui_->loading_label->hide();

  ui_->fetch_tag->setIcon(IconLoader::Load("musicbrainz", 
                          IconLoader::Provider));

  // An editable field is one that has a label as a buddy.  The label is
  // important because it gets turned bold when the field is changed.
  for (QLabel* label : findChildren<QLabel*>()) {
    QWidget* widget = label->buddy();
    if (widget) {
      // Store information about the field
      fields_ << FieldData(label, widget, widget->objectName());

      // Connect the Reset signal
      if (dynamic_cast<ExtendedEditor*>(widget)) {
        connect(widget, SIGNAL(Reset()), SLOT(ResetField()));
      }

      // Connect the changed signal (emitted when value is changed
      // programmatically or non-programmatically)
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
  const QColor color = palette().color(QPalette::WindowText);
  QPalette summary_label_palette(palette());
  summary_label_palette.setColor(
      QPalette::WindowText, light ? color.lighter(150) : color.darker(150));

  for (QLabel* label : ui_->summary_tab->findChildren<QLabel*>()) {
    if (label->property("field_label").toBool()) {
      label->setPalette(summary_label_palette);
    }
  }

  // Pretend the summary text is just a label
  ui_->summary->setMaximumHeight(ui_->art->height() -
                                 ui_->summary_art_button->height() - 4);

  connect(ui_->song_list->selectionModel(),
          SIGNAL(selectionChanged(QItemSelection, QItemSelection)),
          SLOT(SelectionChanged()));
  connect(ui_->button_box, SIGNAL(clicked(QAbstractButton*)),
          SLOT(ButtonClicked(QAbstractButton*)));
  connect(ui_->rating, SIGNAL(RatingChanged(float)), SLOT(SongRated(float)));
  connect(ui_->playcount_reset, SIGNAL(clicked()), SLOT(ResetPlayCounts()));
  connect(ui_->fetch_tag, SIGNAL(clicked()), SLOT(FetchTag()));

  // Set up the album cover menu
  cover_menu_ = new QMenu(this);

  QList<QAction*> actions = album_cover_choice_controller_->GetAllActions();

  connect(album_cover_choice_controller_->cover_from_file_action(),
          SIGNAL(triggered()), this, SLOT(LoadCoverFromFile()));
  connect(album_cover_choice_controller_->cover_to_file_action(),
          SIGNAL(triggered()), this, SLOT(SaveCoverToFile()));
  connect(album_cover_choice_controller_->cover_from_url_action(),
          SIGNAL(triggered()), this, SLOT(LoadCoverFromURL()));
  connect(album_cover_choice_controller_->search_for_cover_action(),
          SIGNAL(triggered()), this, SLOT(SearchForCover()));
  connect(album_cover_choice_controller_->unset_cover_action(),
          SIGNAL(triggered()), this, SLOT(UnsetCover()));
  connect(album_cover_choice_controller_->show_cover_action(),
          SIGNAL(triggered()), this, SLOT(ShowCover()));

  cover_menu_->addActions(actions);

  ui_->summary_art_button->setMenu(cover_menu_);

  ui_->art->installEventFilter(this);
  ui_->art->setAcceptDrops(true);

  // Add the next/previous buttons
  previous_button_ = new QPushButton(
      IconLoader::Load("go-previous", IconLoader::Base), tr("Previous"), this);
  next_button_ = new QPushButton(IconLoader::Load("go-next", IconLoader::Base),
                                 tr("Next"), this);
  ui_->button_box->addButton(previous_button_, QDialogButtonBox::ResetRole);
  ui_->button_box->addButton(next_button_, QDialogButtonBox::ResetRole);

  connect(previous_button_, SIGNAL(clicked()), SLOT(PreviousSong()));
  connect(next_button_, SIGNAL(clicked()), SLOT(NextSong()));

  // Set some shortcuts for the buttons
  new QShortcut(QKeySequence::Back, previous_button_, SLOT(click()));
  new QShortcut(QKeySequence::Forward, next_button_, SLOT(click()));
  new QShortcut(QKeySequence::MoveToPreviousPage, previous_button_,
                SLOT(click()));
  new QShortcut(QKeySequence::MoveToNextPage, next_button_, SLOT(click()));

  // Show the shortcuts as tooltips
  previous_button_->setToolTip(QString("%1 (%2 / %3)").arg(
      previous_button_->text(),
      QKeySequence(QKeySequence::Back).toString(QKeySequence::NativeText),
      QKeySequence(QKeySequence::MoveToPreviousPage)
          .toString(QKeySequence::NativeText)));
  next_button_->setToolTip(QString("%1 (%2 / %3)").arg(
      next_button_->text(),
      QKeySequence(QKeySequence::Forward).toString(QKeySequence::NativeText),
      QKeySequence(QKeySequence::MoveToNextPage)
          .toString(QKeySequence::NativeText)));

  new TagCompleter(app_->library_backend(), Playlist::Column_Artist,
                   ui_->artist);
  new TagCompleter(app_->library_backend(), Playlist::Column_Album, ui_->album);
  new TagCompleter(app_->library_backend(), Playlist::Column_AlbumArtist,
                   ui_->albumartist);
  new TagCompleter(app_->library_backend(), Playlist::Column_Genre, ui_->genre);
  new TagCompleter(app_->library_backend(), Playlist::Column_Composer,
                   ui_->composer);
  new TagCompleter(app_->library_backend(), Playlist::Column_Performer,
                   ui_->performer);
  new TagCompleter(app_->library_backend(), Playlist::Column_Grouping,
                   ui_->grouping);
}

EditTagDialog::~EditTagDialog() { delete ui_; }

bool EditTagDialog::SetLoading(const QString& message) {
  const bool loading = !message.isEmpty();
  if (loading == loading_) return false;
  loading_ = loading;

  ui_->button_box->setEnabled(!loading);
  ui_->tab_widget->setEnabled(!loading);
  ui_->song_list->setEnabled(!loading);
  ui_->fetch_tag->setEnabled(!loading);
  ui_->loading_label->setVisible(loading);
  ui_->loading_label->set_text(message);
  return true;
}

QList<EditTagDialog::Data> EditTagDialog::LoadData(
    const SongList& songs) const {
  QList<Data> ret;

  for (const Song& song : songs) {
    if (song.IsEditable()) {
      // Try reloading the tags from file
      Song copy(song);
      TagReaderClient::Instance()->ReadFileBlocking(copy.url().toLocalFile(),
                                                    &copy);

      if (copy.is_valid()) {
        copy.MergeUserSetData(song);
        ret << Data(copy);
      }
    }
  }

  return ret;
}

void EditTagDialog::SetSongs(const SongList& s, const PlaylistItemList& items) {
  // Show the loading indicator
  if (!SetLoading(tr("Loading tracks") + "...")) return;

  data_.clear();
  playlist_items_ = items;
  ui_->song_list->clear();

  // Reload tags in the background
  QFuture<QList<Data>> future =
      QtConcurrent::run(this, &EditTagDialog::LoadData, s);
  NewClosure(future, this,
             SLOT(SetSongsFinished(QFuture<QList<EditTagDialog::Data>>)),
             future);
}

void EditTagDialog::SetSongsFinished(QFuture<QList<Data>> future) {
  if (!SetLoading(QString())) return;

  data_ = future.result();
  if (data_.count() == 0) {
    // If there were no valid songs, disable everything
    ui_->song_list->setEnabled(false);
    ui_->tab_widget->setEnabled(false);

    // Show a summary with empty information
    UpdateSummaryTab(Song());
    ui_->tab_widget->setCurrentWidget(ui_->summary_tab);

    SetSongListVisibility(false);
    return;
  }

  // Add the filenames to the list
  for (const Data& data : data_) {
    ui_->song_list->addItem(data.current_.basefilename());
  }

  // Select all
  ui_->song_list->setCurrentRow(0);
  ui_->song_list->selectAll();

  // Hide the list if there's only one song in it
  SetSongListVisibility(data_.count() != 1);
}

void EditTagDialog::SetSongListVisibility(bool visible) {
  ui_->song_list->setVisible(visible);
  previous_button_->setEnabled(visible);
  next_button_->setEnabled(visible);
}

QVariant EditTagDialog::Data::value(const Song& song, const QString& id) {
  if (id == "title") return song.title();
  if (id == "artist") return song.artist();
  if (id == "album") return song.album();
  if (id == "albumartist") return song.albumartist();
  if (id == "composer") return song.composer();
  if (id == "performer") return song.performer();
  if (id == "grouping") return song.grouping();
  if (id == "genre") return song.genre();
  if (id == "comment") return song.comment();
  if (id == "lyrics") return song.lyrics();
  if (id == "track") return song.track();
  if (id == "disc") return song.disc();
  if (id == "year") return song.year();
  qLog(Warning) << "Unknown ID" << id;
  return QVariant();
}

void EditTagDialog::Data::set_value(const QString& id, const QVariant& value) {
  if (id == "title")
    current_.set_title(value.toString());
  else if (id == "artist")
    current_.set_artist(value.toString());
  else if (id == "album")
    current_.set_album(value.toString());
  else if (id == "albumartist")
    current_.set_albumartist(value.toString());
  else if (id == "composer")
    current_.set_composer(value.toString());
  else if (id == "performer")
    current_.set_performer(value.toString());
  else if (id == "grouping")
    current_.set_grouping(value.toString());
  else if (id == "genre")
    current_.set_genre(value.toString());
  else if (id == "comment")
    current_.set_comment(value.toString());
  else if (id == "lyrics")
    current_.set_lyrics(value.toString());
  else if (id == "track")
    current_.set_track(value.toInt());
  else if (id == "disc")
    current_.set_disc(value.toInt());
  else if (id == "year")
    current_.set_year(value.toInt());
  else
    qLog(Warning) << "Unknown ID" << id;
}

bool EditTagDialog::DoesValueVary(const QModelIndexList& sel,
                                  const QString& id) const {
  QVariant value = data_[sel.first().row()].current_value(id);
  for (int i = 1; i < sel.count(); ++i) {
    if (value != data_[sel[i].row()].current_value(id)) return true;
  }
  return false;
}

bool EditTagDialog::IsValueModified(const QModelIndexList& sel,
                                    const QString& id) const {
  for (const QModelIndex& i : sel) {
    if (data_[i.row()].original_value(id) != data_[i.row()].current_value(id))
      return true;
  }
  return false;
}

void EditTagDialog::InitFieldValue(const FieldData& field,
                                   const QModelIndexList& sel) {
  const bool varies = DoesValueVary(sel, field.id_);

  if (ExtendedEditor* editor = dynamic_cast<ExtendedEditor*>(field.editor_)) {
    editor->clear();
    editor->clear_hint();
    if (varies) {
      editor->set_hint(tr(EditTagDialog::kHintText));
    } else {
      editor->set_text(data_[sel[0].row()].current_value(field.id_).toString());
    }
  }

  UpdateModifiedField(field, sel);
}

void EditTagDialog::UpdateFieldValue(const FieldData& field,
                                     const QModelIndexList& sel) {
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
  for (const QModelIndex& i : sel) {
    data_[i.row()].set_value(field.id_, value);
  }

  UpdateModifiedField(field, sel);
}

void EditTagDialog::UpdateModifiedField(const FieldData& field, const QModelIndexList& sel) {
  const bool modified = IsValueModified(sel, field.id_);

  // Update the boldness
  QFont new_font(font());
  new_font.setBold(modified);
  field.label_->setFont(new_font);
  field.editor_->setFont(new_font);
}

void EditTagDialog::ResetFieldValue(const FieldData& field,
                                    const QModelIndexList& sel) {
  // Reset each selected song
  for (const QModelIndex& i : sel) {
    Data& data = data_[i.row()];
    data.set_value(field.id_, data.original_value(field.id_));
  }

  // Reset the field
  InitFieldValue(field, sel);
}

void EditTagDialog::SelectionChanged() {
  const QModelIndexList sel =
      ui_->song_list->selectionModel()->selectedIndexes();
  if (sel.isEmpty()) return;

  // Set the editable fields
  UpdateUI(sel);

  // If we're editing multiple songs then we have to disable certain tabs
  const bool multiple = sel.count() > 1;
  ui_->tab_widget->setTabEnabled(ui_->tab_widget->indexOf(ui_->summary_tab),
                                 !multiple);

  if (!multiple) {
    const Song& song = data_[sel.first().row()].original_;
    UpdateSummaryTab(song);
    UpdateStatisticsTab(song);
  }
}

void EditTagDialog::UpdateUI(const QModelIndexList& sel){
  ignore_edits_ = true;
  for (const FieldData& field : fields_) {
    InitFieldValue(field, sel);
  }
  ignore_edits_ = false;
}

static void SetText(QLabel* label, int value, const QString& suffix,
                    const QString& def = QString()) {
  label->setText(value <= 0 ? def : (QString::number(value) + " " + suffix));
}

static void SetDate(QLabel* label, uint time) {
  if (time == std::numeric_limits<uint>::max()) {  // -1
    label->setText(QObject::tr("Unknown"));
  } else {
    label->setText(QDateTime::fromTime_t(time).toString(
        QLocale::system().dateTimeFormat(QLocale::LongFormat)));
  }
}

void EditTagDialog::UpdateSummaryTab(const Song& song) {
  cover_art_id_ =
      app_->album_cover_loader()->LoadImageAsync(cover_options_, song);

  QString summary =
      "<b>" + Qt::escape(song.PrettyTitleWithArtist()) + "</b><br/>";

  bool art_is_set = true;
  if (song.has_manually_unset_cover()) {
    summary += Qt::escape(tr("Cover art manually unset"));
    art_is_set = false;
  } else if (!song.art_manual().isEmpty()) {
    summary += Qt::escape(tr("Cover art set from %1").arg(song.art_manual()));
  } else if (song.has_embedded_cover()) {
    summary += Qt::escape(tr("Cover art from embedded image"));
  } else if (!song.art_automatic().isEmpty()) {
    summary += Qt::escape(
        tr("Cover art loaded automatically from %1").arg(song.art_automatic()));
  } else {
    summary += Qt::escape(tr("Cover art not set"));
    art_is_set = false;
  }

  ui_->summary->setText(summary);

  album_cover_choice_controller_->unset_cover_action()->setEnabled(art_is_set);
  album_cover_choice_controller_->show_cover_action()->setEnabled(art_is_set);
  ui_->summary_art_button->setEnabled(song.id() != -1);

  ui_->length->setText(Utilities::PrettyTimeNanosec(song.length_nanosec()));
  SetText(ui_->bpm, song.bpm(), tr("bpm"));
  SetText(ui_->samplerate, song.samplerate(), "Hz");
  SetText(ui_->bitrate, song.bitrate(), tr("kbps"));
  SetDate(ui_->mtime, song.mtime());
  SetDate(ui_->ctime, song.ctime());

  if (song.filesize() == -1) {
    ui_->filesize->setText(tr("Unknown"));
  } else {
    ui_->filesize->setText(Utilities::PrettySize(song.filesize()));
  }

  ui_->filetype->setText(song.TextForFiletype());

  if (song.url().scheme() == "file")
    ui_->filename->setText(QDir::toNativeSeparators(song.url().toLocalFile()));
  else
    ui_->filename->setText(song.url().toString());

  album_cover_choice_controller_->search_for_cover_action()->setEnabled(
      app_->cover_providers()->HasAnyProviders());
}

void EditTagDialog::UpdateStatisticsTab(const Song& song) {
  ui_->playcount->setText(QString::number(qMax(0, song.playcount())));
  ui_->skipcount->setText(QString::number(qMax(0, song.skipcount())));
  ui_->score->setText(QString::number(qMax(0, song.score())));
  ui_->rating->set_rating(song.rating());

  ui_->lastplayed->setText(
      song.lastplayed() <= 0
          ? tr("Never")
          : QDateTime::fromTime_t(song.lastplayed()).toString(
                QLocale::system().dateTimeFormat(QLocale::LongFormat)));
}

void EditTagDialog::ArtLoaded(quint64 id, const QImage& scaled,
                              const QImage& original) {
  if (id == cover_art_id_) {
    ui_->art->setPixmap(QPixmap::fromImage(scaled));
    original_ = original;
  }
}

void EditTagDialog::FieldValueEdited() {
  if (ignore_edits_) return;

  const QModelIndexList sel =
      ui_->song_list->selectionModel()->selectedIndexes();
  if (sel.isEmpty()) return;

  QWidget* w = qobject_cast<QWidget*>(sender());

  // Find the field
  for (const FieldData& field : fields_) {
    if (field.editor_ == w) {
      UpdateFieldValue(field, sel);
      return;
    }
  }
}

void EditTagDialog::ResetField() {
  const QModelIndexList sel =
      ui_->song_list->selectionModel()->selectedIndexes();
  if (sel.isEmpty()) return;

  QWidget* w = qobject_cast<QWidget*>(sender());

  // Find the field
  for (const FieldData& field : fields_) {
    if (field.editor_ == w) {
      ignore_edits_ = true;
      ResetFieldValue(field, sel);
      ignore_edits_ = false;
      return;
    }
  }
}

Song* EditTagDialog::GetFirstSelected() {
  const QModelIndexList sel =
      ui_->song_list->selectionModel()->selectedIndexes();
  if (sel.isEmpty()) return nullptr;
  return &data_[sel.first().row()].original_;
}

void EditTagDialog::LoadCoverFromFile() {
  Song* song = GetFirstSelected();
  if (!song) return;

  const QModelIndexList sel =
      ui_->song_list->selectionModel()->selectedIndexes();

  QString cover = album_cover_choice_controller_->LoadCoverFromFile(song);

  if (!cover.isEmpty()) UpdateCoverOf(*song, sel, cover);
}

void EditTagDialog::SaveCoverToFile() {
  Song* song = GetFirstSelected();
  if (!song) return;

  album_cover_choice_controller_->SaveCoverToFile(*song, original_);
}

void EditTagDialog::LoadCoverFromURL() {
  Song* song = GetFirstSelected();
  if (!song) return;

  const QModelIndexList sel =
      ui_->song_list->selectionModel()->selectedIndexes();

  QString cover = album_cover_choice_controller_->LoadCoverFromURL(song);

  if (!cover.isEmpty()) UpdateCoverOf(*song, sel, cover);
}

void EditTagDialog::SearchForCover() {
  Song* song = GetFirstSelected();
  if (!song) return;

  const QModelIndexList sel =
      ui_->song_list->selectionModel()->selectedIndexes();

  QString cover = album_cover_choice_controller_->SearchForCover(song);

  if (!cover.isEmpty()) UpdateCoverOf(*song, sel, cover);
}

void EditTagDialog::UnsetCover() {
  Song* song = GetFirstSelected();
  if (!song) return;

  const QModelIndexList sel =
      ui_->song_list->selectionModel()->selectedIndexes();

  QString cover = album_cover_choice_controller_->UnsetCover(song);
  UpdateCoverOf(*song, sel, cover);
}

void EditTagDialog::ShowCover() {
  Song* song = GetFirstSelected();
  if (!song) {
    return;
  }

  album_cover_choice_controller_->ShowCover(*song);
}

void EditTagDialog::UpdateCoverOf(const Song& selected,
                                  const QModelIndexList& sel,
                                  const QString& cover) {
  if (!selected.is_valid() || selected.id() == -1) return;

  UpdateSummaryTab(selected);

  // Now check if we have any other songs cached that share that artist and
  // album (and would therefore be changed as well)
  for (int i = 0; i < data_.count(); ++i) {
    if (i == sel.first().row())  // Already changed this one
      continue;

    Song* other_song = &data_[i].original_;
    if (selected.artist() == other_song->artist() &&
        selected.album() == other_song->album()) {
      other_song->set_art_manual(cover);
    }
  }
}

void EditTagDialog::NextSong() {
  if (ui_->song_list->count() == 0) {
    return;
  }

  int row = (ui_->song_list->currentRow() + 1) % ui_->song_list->count();
  ui_->song_list->setCurrentRow(row);
}

void EditTagDialog::PreviousSong() {
  if (ui_->song_list->count() == 0) {
    return;
  }

  int row = (ui_->song_list->currentRow() - 1 + ui_->song_list->count()) %
            ui_->song_list->count();
  ui_->song_list->setCurrentRow(row);
}

void EditTagDialog::ButtonClicked(QAbstractButton* button) {
  if (button == ui_->button_box->button(QDialogButtonBox::Discard)) {
    reject();
  }
}

void EditTagDialog::SaveData(const QList<Data>& data) {
  for (int i = 0; i < data.count(); ++i) {
    const Data& ref = data[i];

    // Remove the item from list to show progress
    for(int i=0;i < ui_->song_list->count();i++) {
        if(ui_->song_list->item(i)->text() == ref.current_.basefilename()) {
           ui_->song_list->takeItem(i);
           break;
        }
    }
    // No changes
    if (ref.current_.IsMetadataEqual(ref.original_)) continue;

    if (!TagReaderClient::Instance()->SaveFileBlocking(
            ref.current_.url().toLocalFile(), ref.current_)) {
      emit Error(tr("An error occurred writing metadata to '%1'")
                     .arg(ref.current_.url().toLocalFile()));
    }
  }
}

void EditTagDialog::accept() {
  // Show the loading indicator
  if (!SetLoading(tr("Saving tracks") + "...")) return;

  // Save tags in the background
  QFuture<void> future =
      QtConcurrent::run(this, &EditTagDialog::SaveData, data_);
  NewClosure(future, this, SLOT(AcceptFinished()));
}

void EditTagDialog::AcceptFinished() {
  if (!SetLoading(QString())) return;

  // Ask library to rescan the edited tracks to update
  // GUI.
  SongList editedTracks;
  for(Data d: data_) {
      editedTracks << d.current_;
      qDebug() << Q_FUNC_INFO << " edited " << d.current_.url().path();
  }
  app_->library()->Rescan(editedTracks);

  QDialog::accept();
}

bool EditTagDialog::eventFilter(QObject* o, QEvent* e) {
  if (o == ui_->art) {
    switch (e->type()) {
      case QEvent::MouseButtonRelease:
        cover_menu_->popup(static_cast<QMouseEvent*>(e)->globalPos());
        break;

      case QEvent::DragEnter: {
        QDragEnterEvent* event = static_cast<QDragEnterEvent*>(e);
        if (AlbumCoverChoiceController::CanAcceptDrag(event)) {
          event->acceptProposedAction();
        }
        break;
      }

      case QEvent::Drop: {
        const QDropEvent* event = static_cast<QDropEvent*>(e);
        const QModelIndexList sel =
            ui_->song_list->selectionModel()->selectedIndexes();
        Song* song = GetFirstSelected();

        const QString cover =
            album_cover_choice_controller_->SaveCover(song, event);
        if (!cover.isEmpty()) {
          UpdateCoverOf(*song, sel, cover);
        }

        break;
      }

      default:
        break;
    }
  }
  return false;
}

void EditTagDialog::showEvent(QShowEvent* e) {
  // Set the dialog's height to the smallest possible
  resize(width(), sizeHint().height());

  // Restore the tab that was current last time.
  QSettings s;
  s.beginGroup(kSettingsGroup);
  ui_->tab_widget->setCurrentIndex(s.value("current_tab").toInt());

  QDialog::showEvent(e);
}

void EditTagDialog::hideEvent(QHideEvent* e) {
  // Save the current tab
  QSettings s;
  s.beginGroup(kSettingsGroup);
  s.setValue("current_tab", ui_->tab_widget->currentIndex());

  QDialog::hideEvent(e);
}

void EditTagDialog::SongRated(float rating) {
  const QModelIndexList sel =
      ui_->song_list->selectionModel()->selectedIndexes();
  if (sel.isEmpty()) return;
  Song* song = &data_[sel.first().row()].original_;
  if (!song->is_valid() || song->id() == -1) return;

  song->set_rating(rating);
  app_->library_backend()->UpdateSongRatingAsync(song->id(), rating);
}

void EditTagDialog::ResetPlayCounts() {
  const QModelIndexList sel =
      ui_->song_list->selectionModel()->selectedIndexes();
  if (sel.isEmpty()) return;
  Song* song = &data_[sel.first().row()].original_;
  if (!song->is_valid() || song->id() == -1) return;

  if (QMessageBox::question(
          this, tr("Reset play counts"),
          tr("Are you sure you want to reset this song's statistics?"),
          QMessageBox::Reset, QMessageBox::Cancel) != QMessageBox::Reset) {
    return;
  }

  song->set_playcount(0);
  song->set_skipcount(0);
  song->set_lastplayed(-1);
  song->set_score(0);
  app_->library_backend()->ResetStatisticsAsync(song->id());
  UpdateStatisticsTab(*song);
}

void EditTagDialog::FetchTag() {
  const QModelIndexList sel =
      ui_->song_list->selectionModel()->selectedIndexes();

  SongList songs;

  for (const QModelIndex& index : sel) {
    Song song = data_[index.row()].original_;
    if (!song.is_valid()) {
      continue;
    }

    songs << song;
  }

  if (songs.isEmpty()) return;

  results_dialog_->Init(songs);
  tag_fetcher_->StartFetch(songs);

  results_dialog_->show();
}

void EditTagDialog::FetchTagSongChosen(const Song& original_song,
                                       const Song& new_metadata) {
  const QString filename = original_song.url().toLocalFile();

  // Find the song with this filename
  auto data_it =
      std::find_if(data_.begin(), data_.end(), [&filename](const Data& d) {
        return d.original_.url().toLocalFile() == filename;
      });
  if (data_it == data_.end()) {
    qLog(Warning) << "Could not find song to filename: " << filename;
    return;
  }

  // Update song data
  data_it->current_.set_title(new_metadata.title());
  data_it->current_.set_artist(new_metadata.artist());
  data_it->current_.set_album(new_metadata.album());
  data_it->current_.set_track(new_metadata.track());
  data_it->current_.set_year(new_metadata.year());

  // Is it currently being displayed in the UI?
  if (ui_->song_list->currentRow() == std::distance(data_.begin(), data_it)) {
    // Yes! Additionally update UI
    const QModelIndexList sel =
        ui_->song_list->selectionModel()->selectedIndexes();
    UpdateUI(sel);
  }
}
