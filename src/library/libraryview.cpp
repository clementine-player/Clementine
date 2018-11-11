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

#include "libraryview.h"

#include <QPainter>
#include <QContextMenuEvent>
#include <QHelpEvent>
#include <QMenu>
#include <QMessageBox>
#include <QSet>
#include <QSettings>
#include <QSortFilterProxyModel>
#include <QToolTip>
#include <QWhatsThis>

#include "librarydirectorymodel.h"
#include "libraryfilterwidget.h"
#include "librarymodel.h"
#include "libraryitem.h"
#include "librarybackend.h"
#include "core/application.h"
#include "core/deletefiles.h"
#include "core/logging.h"
#include "core/mimedata.h"
#include "core/musicstorage.h"
#include "core/utilities.h"
#include "devices/devicemanager.h"
#include "devices/devicestatefiltermodel.h"
#include "smartplaylists/wizard.h"
#include "ui/iconloader.h"
#include "ui/organisedialog.h"
#include "ui/organiseerrordialog.h"

using smart_playlists::Wizard;

const char* LibraryView::kSettingsGroup = "LibraryView";

LibraryItemDelegate::LibraryItemDelegate(QObject* parent)
    : QStyledItemDelegate(parent) {}

void LibraryItemDelegate::paint(QPainter* painter,
                                const QStyleOptionViewItem& opt,
                                const QModelIndex& index) const {
  const bool is_divider = index.data(LibraryModel::Role_IsDivider).toBool();

  if (is_divider) {
    QString text(index.data().toString());

    painter->save();

    QRect text_rect(opt.rect);

    // Does this item have an icon?
    QPixmap pixmap;
    QVariant decoration = index.data(Qt::DecorationRole);
    if (!decoration.isNull()) {
      if (decoration.canConvert<QPixmap>()) {
        pixmap = decoration.value<QPixmap>();
      } else if (decoration.canConvert<QIcon>()) {
        pixmap = decoration.value<QIcon>().pixmap(opt.decorationSize);
      }
    }

    // Draw the icon at the left of the text rectangle
    if (!pixmap.isNull()) {
      QRect icon_rect(text_rect.topLeft(), opt.decorationSize);
      const int padding = (text_rect.height() - icon_rect.height()) / 2;
      icon_rect.adjust(padding, padding, padding, padding);
      text_rect.moveLeft(icon_rect.right() + padding + 6);

      if (pixmap.size() != opt.decorationSize) {
        pixmap = pixmap.scaled(opt.decorationSize, Qt::KeepAspectRatio);
      }

      painter->drawPixmap(icon_rect, pixmap);
    } else {
      text_rect.setLeft(text_rect.left() + 30);
    }

    // Draw the text
    QFont bold_font(opt.font);
    bold_font.setBold(true);

    painter->setPen(opt.palette.color(QPalette::Text));
    painter->setFont(bold_font);
    painter->drawText(text_rect, text);

    // Draw the line under the item
    QColor line_color = opt.palette.color(QPalette::Text);
    QLinearGradient grad_color(opt.rect.bottomLeft(), opt.rect.bottomRight());
    const double fade_start_end = (opt.rect.width() / 3.0) / opt.rect.width();
    line_color.setAlphaF(0.0);
    grad_color.setColorAt(0, line_color);
    line_color.setAlphaF(0.5);
    grad_color.setColorAt(fade_start_end, line_color);
    grad_color.setColorAt(1.0 - fade_start_end, line_color);
    line_color.setAlphaF(0.0);
    grad_color.setColorAt(1, line_color);
    painter->setPen(QPen(grad_color, 1));
    painter->drawLine(opt.rect.bottomLeft(), opt.rect.bottomRight());

    painter->restore();
  } else {
    QStyledItemDelegate::paint(painter, opt, index);
  }
}

bool LibraryItemDelegate::helpEvent(QHelpEvent* event, QAbstractItemView* view,
                                    const QStyleOptionViewItem& option,
                                    const QModelIndex& index) {
  Q_UNUSED(option);

  if (!event || !view) return false;

  QHelpEvent* he = static_cast<QHelpEvent*>(event);
  QString text = displayText(index.data(), QLocale::system());

  if (text.isEmpty() || !he) return false;

  switch (event->type()) {
    case QEvent::ToolTip: {
      QRect displayed_text;
      QSize real_text;
      bool is_elided = false;

      real_text = sizeHint(option, index);
      displayed_text = view->visualRect(index);
      is_elided = displayed_text.width() < real_text.width();

      if (is_elided) {
        QToolTip::showText(he->globalPos(), text, view);
      } else if (index.data(Qt::ToolTipRole).isValid()) {
        // If the item has a tooltip text, display it
        QString tooltip_text = index.data(Qt::ToolTipRole).toString();
        QToolTip::showText(he->globalPos(), tooltip_text, view);
      } else {
        // in case that another text was previously displayed
        QToolTip::hideText();
      }
      return true;
    }

    case QEvent::QueryWhatsThis:
      return true;

    case QEvent::WhatsThis:
      QWhatsThis::showText(he->globalPos(), text, view);
      return true;

    default:
      break;
  }
  return false;
}

LibraryView::LibraryView(QWidget* parent)
    : AutoExpandingTreeView(parent),
      app_(nullptr),
      filter_(nullptr),
      total_song_count_(-1),
      context_menu_(nullptr),
      is_in_keyboard_search_(false) {
  QIcon nomusic = IconLoader::Load("nomusic", IconLoader::Other);
  nomusic_ = nomusic.pixmap(nomusic.availableSizes().last());
  setItemDelegate(new LibraryItemDelegate(this));
  setAttribute(Qt::WA_MacShowFocusRect, false);
  setHeaderHidden(true);
  setAllColumnsShowFocus(true);
  setDragEnabled(true);
  setDragDropMode(QAbstractItemView::DragOnly);
  setSelectionMode(QAbstractItemView::ExtendedSelection);

  setStyleSheet("QTreeView::item{padding-top:1px;}");
}

LibraryView::~LibraryView() {}

void LibraryView::SaveFocus() {
  QModelIndex current = currentIndex();
  QVariant type = model()->data(current, LibraryModel::Role_Type);
  if (!type.isValid() ||
      !(type.toInt() == LibraryItem::Type_Song ||
        type.toInt() == LibraryItem::Type_Container ||
        type.toInt() == LibraryItem::Type_Divider)) {
    return;
  }

  last_selected_path_.clear();
  last_selected_song_ = Song();
  last_selected_container_ = QString();
  last_selected_text_ = QString();

  switch (type.toInt()) {
    case LibraryItem::Type_Song: {
      QModelIndex index =
          qobject_cast<QSortFilterProxyModel*>(model())->mapToSource(current);
      SongList songs = app_->library_model()->GetChildSongs(index);
      if (!songs.isEmpty()) {
        last_selected_song_ = songs.last();
        last_selected_text_ = songs.last().title();
      }
      break;
    }

    case LibraryItem::Type_Container:
    case LibraryItem::Type_Divider: {
      QString text =
          model()->data(current, LibraryModel::Role_Key).toString();
      last_selected_container_ = text;
      last_selected_text_ = model()->data(current, LibraryModel::Role_DisplayText).toString();
      break;
    }

    default:
      return;
  }

  SaveContainerPath(current);
}

void LibraryView::SaveContainerPath(const QModelIndex& child) {
  QModelIndex current = model()->parent(child);
  QVariant type = model()->data(current, LibraryModel::Role_Type);
  if (!type.isValid() ||
      !(type.toInt() == LibraryItem::Type_Container ||
        type.toInt() == LibraryItem::Type_Divider)) {
    return;
  }

  QString text = model()->data(current, LibraryModel::Role_SortText).toString();
  last_selected_path_ << text;
  SaveContainerPath(current);
}

void LibraryView::RestoreFocus() {
  if (last_selected_container_.isEmpty() &&
      last_selected_song_.url().isEmpty()) {
    return;
  }
  RestoreLevelFocus();
}

bool LibraryView::RestoreLevelFocus(const QModelIndex& parent) {
  if (model()->canFetchMore(parent)) {
    model()->fetchMore(parent);
  }
  int rows = model()->rowCount(parent);
  for (int i = 0; i < rows; i++) {
    QModelIndex current = model()->index(i, 0, parent);
    QVariant type = model()->data(current, LibraryModel::Role_Type);
    switch (type.toInt()) {
      case LibraryItem::Type_Song:
        if (!last_selected_song_.url().isEmpty()) {
          QModelIndex index = qobject_cast<QSortFilterProxyModel*>(model())
                                  ->mapToSource(current);
          SongList songs = app_->library_model()->GetChildSongs(index);
          for (const Song& song : songs) {
            if (song == last_selected_song_) {
              setCurrentIndex(current);
              return true;
            }
          }
        }
        break;

      case LibraryItem::Type_Container:
      case LibraryItem::Type_Divider: {
        QString text =
            model()->data(current, LibraryModel::Role_Key).toString();
        if (!last_selected_container_.isEmpty() &&
            last_selected_container_ == text) {
          emit expand(current);
          setCurrentIndex(current);
          return true;
        } else if (last_selected_path_.contains(text)) {
          emit expand(current);
          // If a selected container or song were not found, we've got into a
          // wrong subtree
          //  (happens with "unknown" all the time)
          if (!RestoreLevelFocus(current)) {
            emit collapse(current);
          } else {
            return true;
          }
        }
        break;
      }
    }
  }
  return false;
}

void LibraryView::ReloadSettings() {
  QSettings s;
  s.beginGroup(kSettingsGroup);

  SetAutoOpen(s.value("auto_open", true).toBool());
  if (app_ != nullptr) {
    app_->library_model()->set_pretty_covers(
        s.value("pretty_covers", true).toBool());
    app_->library_model()->set_show_dividers(
        s.value("show_dividers", true).toBool());
  }
}

void LibraryView::SetApplication(Application* app) {
  app_ = app;
  ReloadSettings();
}

void LibraryView::SetFilter(LibraryFilterWidget* filter) { filter_ = filter; }

void LibraryView::TotalSongCountUpdated(int count) {
  bool old = total_song_count_;
  total_song_count_ = count;
  if (old != total_song_count_) update();

  if (total_song_count_ == 0)
    setCursor(Qt::PointingHandCursor);
  else
    unsetCursor();
}

void LibraryView::paintEvent(QPaintEvent* event) {
  if (total_song_count_ == 0) {
    QPainter p(viewport());
    QRect rect(viewport()->rect());

    // Draw the confused clementine
    QRect image_rect((rect.width() - nomusic_.width()) / 2, 50,
                     nomusic_.width(), nomusic_.height());
    p.drawPixmap(image_rect, nomusic_);

    // Draw the title text
    QFont bold_font;
    bold_font.setBold(true);
    p.setFont(bold_font);

    QFontMetrics metrics(bold_font);

    QRect title_rect(0, image_rect.bottom() + 20, rect.width(),
                     metrics.height());
    p.drawText(title_rect, Qt::AlignHCenter, tr("Your library is empty!"));

    // Draw the other text
    p.setFont(QFont());

    QRect text_rect(0, title_rect.bottom() + 5, rect.width(), metrics.height());
    p.drawText(text_rect, Qt::AlignHCenter, tr("Click here to add some music"));
  } else {
    QTreeView::paintEvent(event);
  }
}

void LibraryView::mouseReleaseEvent(QMouseEvent* e) {
  QTreeView::mouseReleaseEvent(e);

  if (total_song_count_ == 0) {
    emit ShowConfigDialog();
  }
}

void LibraryView::contextMenuEvent(QContextMenuEvent* e) {
  if (!context_menu_) {
    context_menu_ = new QMenu(this);
    add_to_playlist_ = context_menu_->addAction(
        IconLoader::Load("media-playback-start", IconLoader::Base),
        tr("Append to current playlist"), this, SLOT(AddToPlaylist()));
    load_ = context_menu_->addAction(
        IconLoader::Load("media-playback-start", IconLoader::Base),
        tr("Replace current playlist"), this, SLOT(Load()));
    open_in_new_playlist_ = context_menu_->addAction(
        IconLoader::Load("document-new", IconLoader::Base),
        tr("Open in new playlist"), this, SLOT(OpenInNewPlaylist()));

    context_menu_->addSeparator();
    add_to_playlist_enqueue_ = context_menu_->addAction(
        IconLoader::Load("go-next", IconLoader::Base), tr("Queue track"), this,
        SLOT(AddToPlaylistEnqueue()));
    add_to_playlist_enqueue_next_ = context_menu_->addAction(
        IconLoader::Load("go-next", IconLoader::Base), tr("Play next"), this,
        SLOT(AddToPlaylistEnqueueNext()));
    context_menu_->addSeparator();
    search_for_this_ = context_menu_->addAction(
        IconLoader::Load("system-search", IconLoader::Base),
        tr("Search for this"), this, SLOT(SearchForThis()));
    context_menu_->addSeparator();
    new_smart_playlist_ = context_menu_->addAction(
        IconLoader::Load("document-new", IconLoader::Base),
        tr("New smart playlist..."), this, SLOT(NewSmartPlaylist()));
    edit_smart_playlist_ = context_menu_->addAction(
        IconLoader::Load("edit-rename", IconLoader::Base),
        tr("Edit smart playlist..."), this, SLOT(EditSmartPlaylist()));
    delete_smart_playlist_ = context_menu_->addAction(
        IconLoader::Load("edit-delete", IconLoader::Base),
        tr("Delete smart playlist"), this, SLOT(DeleteSmartPlaylist()));

    context_menu_->addSeparator();
    organise_ = context_menu_->addAction(
        IconLoader::Load("edit-copy", IconLoader::Base),
        tr("Organise files..."), this, SLOT(Organise()));
    copy_to_device_ = context_menu_->addAction(
        IconLoader::Load("multimedia-player-ipod-mini-blue", IconLoader::Base),
        tr("Copy to device..."), this, SLOT(CopyToDevice()));
    delete_ = context_menu_->addAction(
        IconLoader::Load("edit-delete", IconLoader::Base),
        tr("Delete from disk..."), this, SLOT(Delete()));

    context_menu_->addSeparator();
    edit_track_ = context_menu_->addAction(
        IconLoader::Load("edit-rename", IconLoader::Base),
        tr("Edit track information..."), this, SLOT(EditTracks()));
    edit_tracks_ = context_menu_->addAction(
        IconLoader::Load("edit-rename", IconLoader::Base),
        tr("Edit tracks information..."), this, SLOT(EditTracks()));
    show_in_browser_ = context_menu_->addAction(
        IconLoader::Load("document-open-folder", IconLoader::Base),
        tr("Show in file browser..."), this, SLOT(ShowInBrowser()));

    context_menu_->addSeparator();
    show_in_various_ = context_menu_->addAction(tr("Show in various artists"),
                                                this, SLOT(ShowInVarious()));
    no_show_in_various_ = context_menu_->addAction(
        tr("Don't show in various artists"), this, SLOT(NoShowInVarious()));

    context_menu_->addSeparator();

    context_menu_->addMenu(filter_->menu());

    copy_to_device_->setDisabled(
        app_->device_manager()->connected_devices_model()->rowCount() == 0);
    connect(app_->device_manager()->connected_devices_model(),
            SIGNAL(IsEmptyChanged(bool)), copy_to_device_,
            SLOT(setDisabled(bool)));
  }

  context_menu_index_ = indexAt(e->pos());
  if (!context_menu_index_.isValid()) return;

  context_menu_index_ = qobject_cast<QSortFilterProxyModel*>(model())
                            ->mapToSource(context_menu_index_);

  QModelIndexList selected_indexes =
      qobject_cast<QSortFilterProxyModel*>(model())
          ->mapSelectionToSource(selectionModel()->selection())
          .indexes();

  // number of smart playlists selected
  int smart_playlists = 0;
  // is the smart playlists header selected?
  int smart_playlists_header = 0;
  // number of non smart playlists selected
  int regular_elements = 0;
  // number of editable non smart playlists selected
  int regular_editable = 0;
  // number of container elements selected
  int container_elements = 0;

  for (const QModelIndex& index : selected_indexes) {
    int type =
        app_->library_model()->data(index, LibraryModel::Role_Type).toInt();

    if (type == LibraryItem::Type_SmartPlaylist) {
      smart_playlists++;
    } else if (type == LibraryItem::Type_PlaylistContainer) {
      smart_playlists_header++;
    } else if (type == LibraryItem::Type_Container) {
      container_elements++;
      // To preserve expected behavior, since a container is "regular"
      regular_elements++;
    } else {
      regular_elements++;
    }

    if (app_->library_model()
            ->data(index, LibraryModel::Role_Editable)
            .toBool()) {
      regular_editable++;
    }
  }

  // TODO: check if custom plugin actions should be enabled / visible
  const int songs_selected =
      smart_playlists + smart_playlists_header + regular_elements;
  const bool regular_elements_only =
      songs_selected == regular_elements && regular_elements > 0;
  const bool smart_playlists_only =
      songs_selected == smart_playlists + smart_playlists_header;
  const bool only_smart_playlist_selected =
      smart_playlists == 1 && songs_selected == 1;
  const bool one_regular_song_only =
      regular_elements_only && container_elements == 0 && regular_elements == 1;
  const bool one_container_only =
      container_elements == 1 && songs_selected == 1;

  // in all modes
  load_->setEnabled(songs_selected);
  add_to_playlist_->setEnabled(songs_selected);
  open_in_new_playlist_->setEnabled(songs_selected);
  add_to_playlist_enqueue_->setEnabled(songs_selected);

  // allow mixed smart playlists / regular elements selected
  show_in_browser_->setVisible(!smart_playlists_only);
  edit_tracks_->setVisible(!smart_playlists_only && regular_editable > 1);
  // if neither edit_track not edit_tracks are available, we show disabled
  // edit_track element
  edit_track_->setVisible(!smart_playlists_only && (regular_editable <= 1));
  edit_track_->setEnabled(regular_editable == 1);

  // only when no smart playlists selected
  organise_->setVisible(regular_elements_only);
  copy_to_device_->setVisible(regular_elements_only);
  delete_->setVisible(regular_elements_only);
  show_in_various_->setVisible(regular_elements_only);
  no_show_in_various_->setVisible(regular_elements_only);

  // only when a single container or one song is selected exclusively
  search_for_this_->setVisible(one_container_only || one_regular_song_only);

  // only when all selected items are editable
  organise_->setEnabled(regular_elements == regular_editable);
  copy_to_device_->setEnabled(regular_elements == regular_editable);
  delete_->setEnabled(regular_elements == regular_editable);

  // only when no regular elements selected
  new_smart_playlist_->setVisible(smart_playlists_only);
  edit_smart_playlist_->setVisible(smart_playlists_only);
  delete_smart_playlist_->setVisible(smart_playlists_only);

  edit_smart_playlist_->setEnabled(only_smart_playlist_selected);
  delete_smart_playlist_->setEnabled(only_smart_playlist_selected);

  context_menu_->popup(e->globalPos());
}

void LibraryView::ShowInVarious() { ShowInVarious(true); }

void LibraryView::NoShowInVarious() { ShowInVarious(false); }

void LibraryView::ShowInVarious(bool on) {
  if (!context_menu_index_.isValid()) return;

  // Map is from album name -> all artists sharing that album name, built from
  // each selected
  // song. We put through "Various Artists" changes one album at a time, to make
  // sure the old album
  // node gets removed (due to all children removed), before the new one gets
  // added
  QMultiMap<QString, QString> albums;
  for (const Song& song : GetSelectedSongs()) {
    if (albums.find(song.album(), song.artist()) == albums.end())
      albums.insert(song.album(), song.artist());
  }

  // If we have only one album and we are putting it into Various Artists, check
  // to see
  // if there are other Artists in this album and prompt the user if they'd like
  // them moved, too
  if (on && albums.keys().count() == 1) {
    const QString album = albums.keys().first();
    QList<Song> all_of_album = app_->library_backend()->GetSongsByAlbum(album);
    QSet<QString> other_artists;
    for (const Song& s : all_of_album) {
      if (!albums.contains(album, s.artist()) &&
          !other_artists.contains(s.artist())) {
        other_artists.insert(s.artist());
      }
    }
    if (other_artists.count() > 0) {
      if (QMessageBox::question(this, tr("There are other songs in this album"),
                                tr("Would you like to move the other songs in "
                                   "this album to Various Artists as well?"),
                                QMessageBox::Yes | QMessageBox::No,
                                QMessageBox::Yes) == QMessageBox::Yes) {
        for (const QString& s : other_artists) {
          albums.insert(album, s);
        }
      }
    }
  }

  for (const QString& album : QSet<QString>::fromList(albums.keys())) {
    app_->library_backend()->ForceCompilation(album, albums.values(album), on);
  }
}

void LibraryView::Load() {
  QMimeData* data = model()->mimeData(selectedIndexes());
  if (MimeData* mime_data = qobject_cast<MimeData*>(data)) {
    mime_data->clear_first_ = true;
  }
  emit AddToPlaylistSignal(data);
}

void LibraryView::AddToPlaylist() {
  emit AddToPlaylistSignal(model()->mimeData(selectedIndexes()));
}

void LibraryView::AddToPlaylistEnqueue() {
  QMimeData* data = model()->mimeData(selectedIndexes());
  if (MimeData* mime_data = qobject_cast<MimeData*>(data)) {
    mime_data->enqueue_now_ = true;
  }
  emit AddToPlaylistSignal(data);
}

void LibraryView::AddToPlaylistEnqueueNext() {
  QMimeData* data = model()->mimeData(selectedIndexes());
  if (MimeData* mime_data = qobject_cast<MimeData*>(data)) {
    mime_data->enqueue_next_now_ = true;
  }
  emit AddToPlaylistSignal(data);
}

void LibraryView::OpenInNewPlaylist() {
  QMimeData* data = model()->mimeData(selectedIndexes());
  if (MimeData* mime_data = qobject_cast<MimeData*>(data)) {
    mime_data->open_in_new_playlist_ = true;
  }
  emit AddToPlaylistSignal(data);
}

void LibraryView::keyboardSearch(const QString& search) {
  is_in_keyboard_search_ = true;
  QTreeView::keyboardSearch(search);
  is_in_keyboard_search_ = false;
}

void LibraryView::scrollTo(const QModelIndex& index, ScrollHint hint) {
  if (is_in_keyboard_search_)
    QTreeView::scrollTo(index, QAbstractItemView::PositionAtTop);
  else
    QTreeView::scrollTo(index, hint);
}

// get selected songs
SongList LibraryView::GetSelectedSongs() const {
  QModelIndexList selected_indexes =
      qobject_cast<QSortFilterProxyModel*>(model())
          ->mapSelectionToSource(selectionModel()->selection())
          .indexes();
  return app_->library_model()->GetChildSongs(selected_indexes);
}

void LibraryView::Organise() {
  if (!organise_dialog_)
    organise_dialog_.reset(new OrganiseDialog(app_->task_manager(),
                                              app_->library_backend()));

  organise_dialog_->SetDestinationModel(
      app_->library_model()->directory_model());
  organise_dialog_->SetCopy(false);
  if (organise_dialog_->SetSongs(GetSelectedSongs()))
    organise_dialog_->show();
  else {
    QMessageBox::warning(
        this, tr("Error"),
        tr("None of the selected songs were suitable for copying to a device"));
  }
}

void LibraryView::Delete() {
  if (QMessageBox::warning(this, tr("Delete files"),
                           tr("These files will be permanently deleted from "
                              "disk, are you sure you want to continue?"),
                           QMessageBox::Yes,
                           QMessageBox::Cancel) != QMessageBox::Yes)
    return;

  // We can cheat and always take the storage of the first directory, since
  // they'll all be FilesystemMusicStorage in a library and deleting doesn't
  // check the actual directory.
  std::shared_ptr<MusicStorage> storage =
      app_->library_model()
          ->directory_model()
          ->index(0, 0)
          .data(MusicStorage::Role_Storage)
          .value<std::shared_ptr<MusicStorage>>();

  DeleteFiles* delete_files = new DeleteFiles(app_->task_manager(), storage);
  connect(delete_files, SIGNAL(Finished(SongList)),
          SLOT(DeleteFinished(SongList)));
  delete_files->Start(GetSelectedSongs());
}

void LibraryView::EditTracks() {
  if (!edit_tag_dialog_) {
    edit_tag_dialog_.reset(new EditTagDialog(app_, this));
  }
  edit_tag_dialog_->SetSongs(GetSelectedSongs());
  edit_tag_dialog_->show();
}

void LibraryView::CopyToDevice() {
  if (!organise_dialog_)
    // Don't notify song has been replaced if copying to device, so
    // don't associate the organise dialog with the library backend.
    // Could improve this behavior if the device has a separate set of saved
    // playlists that are somehow in sync with the library.
    organise_dialog_.reset(new OrganiseDialog(app_->task_manager()));

  organise_dialog_->SetDestinationModel(
      app_->device_manager()->connected_devices_model(), true);
  organise_dialog_->SetCopy(true);
  organise_dialog_->SetSongs(GetSelectedSongs());
  organise_dialog_->show();
}

void LibraryView::DeleteFinished(const SongList& songs_with_errors) {
  if (songs_with_errors.isEmpty()) return;

  OrganiseErrorDialog* dialog = new OrganiseErrorDialog(this);
  dialog->Show(OrganiseErrorDialog::Type_Delete, songs_with_errors);
  // It deletes itself when the user closes it
}

void LibraryView::FilterReturnPressed() {
  if (!currentIndex().isValid()) {
    // Pick the first thing that isn't a divider
    for (int row = 0; row < model()->rowCount(); ++row) {
      QModelIndex idx(model()->index(row, 0));
      if (idx.data(LibraryModel::Role_Type) != LibraryItem::Type_Divider) {
        setCurrentIndex(idx);
        break;
      }
    }
  }

  if (!currentIndex().isValid()) return;

  emit doubleClicked(currentIndex());
}

void LibraryView::SearchForThis() {
  SaveFocus();
  if (!last_selected_text_.isEmpty()) {
    filter_->ShowInLibrary(last_selected_text_.simplified());
  }
}

void LibraryView::NewSmartPlaylist() {
  Wizard* wizard = new Wizard(app_, app_->library_backend(), this);
  wizard->setAttribute(Qt::WA_DeleteOnClose);
  connect(wizard, SIGNAL(accepted()), SLOT(NewSmartPlaylistFinished()));

  wizard->show();
}

void LibraryView::EditSmartPlaylist() {
  Wizard* wizard = new Wizard(app_, app_->library_backend(), this);
  wizard->setAttribute(Qt::WA_DeleteOnClose);
  connect(wizard, SIGNAL(accepted()), SLOT(EditSmartPlaylistFinished()));

  wizard->show();
  wizard->SetGenerator(
      app_->library_model()->CreateGenerator(context_menu_index_));
}

void LibraryView::DeleteSmartPlaylist() {
  app_->library_model()->DeleteGenerator(context_menu_index_);
}

void LibraryView::NewSmartPlaylistFinished() {
  const Wizard* wizard = qobject_cast<Wizard*>(sender());
  app_->library_model()->AddGenerator(wizard->CreateGenerator());
}

void LibraryView::EditSmartPlaylistFinished() {
  const Wizard* wizard = qobject_cast<Wizard*>(sender());
  app_->library_model()->UpdateGenerator(context_menu_index_,
                                         wizard->CreateGenerator());
}

void LibraryView::ShowInBrowser() {
  QList<QUrl> urls;
  for (const Song& song : GetSelectedSongs()) {
    urls << song.url();
  }

  Utilities::OpenInFileBrowser(urls);
}
