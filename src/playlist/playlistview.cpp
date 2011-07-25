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

#include "dynamicplaylistcontrols.h"
#include "playlist.h"
#include "playlistdelegates.h"
#include "playlistheader.h"
#include "playlistview.h"
#include "core/logging.h"

#include <QCleanlooksStyle>
#include <QPainter>
#include <QHeaderView>
#include <QSettings>
#include <QtDebug>
#include <QTimer>
#include <QKeyEvent>
#include <QApplication>
#include <QSortFilterProxyModel>
#include <QScrollBar>

#include <math.h>

const int PlaylistView::kStateVersion = 3;
const int PlaylistView::kGlowIntensitySteps = 24;
const int PlaylistView::kAutoscrollGraceTimeout = 60; // seconds
const int PlaylistView::kDropIndicatorWidth = 2;
const int PlaylistView::kDropIndicatorGradientWidth = 5;


PlaylistProxyStyle::PlaylistProxyStyle(QStyle* base)
  : QProxyStyle(base),
    cleanlooks_(new QCleanlooksStyle){
}

void PlaylistProxyStyle::drawControl(
    ControlElement element, const QStyleOption* option, QPainter* painter,
    const QWidget* widget) const {
  if (element == CE_Header) {
    const QStyleOptionHeader* header_option = qstyleoption_cast<const QStyleOptionHeader*>(option);
    const QRect& rect = header_option->rect;
    const QString& text = header_option->text;
    const QFontMetrics& font_metrics = header_option->fontMetrics;
    
    // spaces added to make transition less abrupt
    if (rect.width() < font_metrics.width(text + "  ")) {
      const Playlist::Column column = static_cast<Playlist::Column>(header_option->section);
      QStyleOptionHeader new_option(*header_option);
      new_option.text = Playlist::abbreviated_column_name(column);
      QProxyStyle::drawControl(element, &new_option, painter, widget);
      return;
    }
  }

  if (element == CE_ItemViewItem)
    cleanlooks_->drawControl(element, option, painter, widget);
  else
    QProxyStyle::drawControl(element, option, painter, widget);
}

void PlaylistProxyStyle::drawPrimitive(PrimitiveElement element, const QStyleOption* option, QPainter* painter, const QWidget* widget) const {
  if (element == QStyle::PE_PanelItemViewRow ||
      element == QStyle::PE_PanelItemViewItem)
    cleanlooks_->drawPrimitive(element, option, painter, widget);
  else
    QProxyStyle::drawPrimitive(element, option, painter, widget);
}


PlaylistView::PlaylistView(QWidget *parent)
  : QTreeView(parent),
    style_(new PlaylistProxyStyle(style())),
    playlist_(NULL),
    header_(new PlaylistHeader(Qt::Horizontal, this)),
    setting_initial_header_layout_(false),
    upgrading_from_qheaderview_(false),
    read_only_settings_(true),
    glow_enabled_(true),
    currently_glowing_(false),
    glow_intensity_step_(0),
    rating_delegate_(NULL),
    inhibit_autoscroll_timer_(new QTimer(this)),
    inhibit_autoscroll_(false),
    currently_autoscrolling_(false),
    row_height_(-1),
    currenttrack_play_(":currenttrack_play.png"),
    currenttrack_pause_(":currenttrack_pause.png"),
    cached_current_row_row_(-1),
    drop_indicator_row_(-1),
    drag_over_(false),
    dynamic_controls_(new DynamicPlaylistControls(this))
{
  setHeader(header_);
  header_->setMovable(true);
  setStyle(style_);
  setMouseTracking(true);

  connect(header_, SIGNAL(sectionResized(int,int,int)), SLOT(SaveGeometry()));
  connect(header_, SIGNAL(sectionMoved(int,int,int)), SLOT(SaveGeometry()));
  connect(header_, SIGNAL(sortIndicatorChanged(int,Qt::SortOrder)), SLOT(SaveGeometry()));
  connect(header_, SIGNAL(SectionVisibilityChanged(int,bool)), SLOT(SaveGeometry()));
  connect(header_, SIGNAL(sectionResized(int,int,int)), SLOT(InvalidateCachedCurrentPixmap()));
  connect(header_, SIGNAL(sectionMoved(int,int,int)), SLOT(InvalidateCachedCurrentPixmap()));
  connect(header_, SIGNAL(SectionVisibilityChanged(int,bool)), SLOT(InvalidateCachedCurrentPixmap()));
  connect(header_, SIGNAL(StretchEnabledChanged(bool)), SLOT(SaveSettings()));
  connect(header_, SIGNAL(ColumnAlignmentChanged()), SLOT(SaveSettings()));
  connect(header_, SIGNAL(StretchEnabledChanged(bool)), SLOT(StretchChanged(bool)));
  connect(header_, SIGNAL(MouseEntered()), SLOT(RatingHoverOut()));

  inhibit_autoscroll_timer_->setInterval(kAutoscrollGraceTimeout * 1000);
  inhibit_autoscroll_timer_->setSingleShot(true);
  connect(inhibit_autoscroll_timer_, SIGNAL(timeout()), SLOT(InhibitAutoscrollTimeout()));

  horizontalScrollBar()->installEventFilter(this);
  verticalScrollBar()->installEventFilter(this);

  setAlternatingRowColors(true);

  setAttribute(Qt::WA_MacShowFocusRect, false);

  dynamic_controls_->hide();

#ifdef Q_OS_DARWIN
  setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
#endif
}

void PlaylistView::SetItemDelegates(LibraryBackend* backend) {
  rating_delegate_ = new RatingItemDelegate(this);

  setItemDelegate(new PlaylistDelegateBase(this));
  setItemDelegateForColumn(Playlist::Column_Title, new TextItemDelegate(this));
  setItemDelegateForColumn(Playlist::Column_Album,
      new TagCompletionItemDelegate(this, backend, Playlist::Column_Album));
  setItemDelegateForColumn(Playlist::Column_Artist,
      new TagCompletionItemDelegate(this, backend, Playlist::Column_Artist));
  setItemDelegateForColumn(Playlist::Column_AlbumArtist,
      new TagCompletionItemDelegate(this, backend, Playlist::Column_AlbumArtist));
  setItemDelegateForColumn(Playlist::Column_Genre,
      new TagCompletionItemDelegate(this, backend, Playlist::Column_Genre));
  setItemDelegateForColumn(Playlist::Column_Composer,
      new TagCompletionItemDelegate(this, backend, Playlist::Column_Composer));
  setItemDelegateForColumn(Playlist::Column_Length, new LengthItemDelegate(this));
  setItemDelegateForColumn(Playlist::Column_Filesize, new SizeItemDelegate(this));
  setItemDelegateForColumn(Playlist::Column_Filetype, new FileTypeItemDelegate(this));
  setItemDelegateForColumn(Playlist::Column_DateCreated, new DateItemDelegate(this));
  setItemDelegateForColumn(Playlist::Column_DateModified, new DateItemDelegate(this));
  setItemDelegateForColumn(Playlist::Column_BPM, new PlaylistDelegateBase(this, tr("bpm")));
  setItemDelegateForColumn(Playlist::Column_Samplerate, new PlaylistDelegateBase(this, ("Hz")));
  setItemDelegateForColumn(Playlist::Column_Bitrate, new PlaylistDelegateBase(this, tr("kbps")));
  setItemDelegateForColumn(Playlist::Column_Filename, new NativeSeparatorsDelegate(this));
  setItemDelegateForColumn(Playlist::Column_Rating, rating_delegate_);
  setItemDelegateForColumn(Playlist::Column_LastPlayed, new LastPlayedItemDelegate(this));
}

void PlaylistView::SetPlaylist(Playlist* playlist) {
  if (playlist_) {
    disconnect(playlist_, SIGNAL(CurrentSongChanged(Song)),
               this, SLOT(MaybeAutoscroll()));
    disconnect(playlist_, SIGNAL(DynamicModeChanged(bool)),
               this, SLOT(DynamicModeChanged(bool)));
    disconnect(playlist_, SIGNAL(destroyed()), this, SLOT(PlaylistDestroyed()));

    disconnect(dynamic_controls_, SIGNAL(Repopulate()),
               playlist_, SLOT(RepopulateDynamicPlaylist()));
    disconnect(dynamic_controls_, SIGNAL(TurnOff()),
               playlist_, SLOT(TurnOffDynamicPlaylist()));
  }

  playlist_ = playlist;
  LoadGeometry();
  ReloadSettings();
  DynamicModeChanged(playlist->is_dynamic());
  setFocus();
  read_only_settings_ = false;

  connect(playlist_, SIGNAL(RestoreFinished()), SLOT(JumpToLastPlayedTrack()));

  connect(playlist_, SIGNAL(CurrentSongChanged(Song)), SLOT(MaybeAutoscroll()));
  connect(playlist_, SIGNAL(DynamicModeChanged(bool)), SLOT(DynamicModeChanged(bool)));
  connect(playlist_, SIGNAL(destroyed()), SLOT(PlaylistDestroyed()));
  connect(dynamic_controls_, SIGNAL(Repopulate()), playlist_, SLOT(RepopulateDynamicPlaylist()));
  connect(dynamic_controls_, SIGNAL(TurnOff()), playlist_, SLOT(TurnOffDynamicPlaylist()));
}

void PlaylistView::setModel(QAbstractItemModel *m) {
  if (model()) {
    disconnect(model(), SIGNAL(dataChanged(QModelIndex,QModelIndex)),
               this, SLOT(InvalidateCachedCurrentPixmap()));
    disconnect(model(), SIGNAL(layoutAboutToBeChanged()),
               this, SLOT(RatingHoverOut()));
  }

  QTreeView::setModel(m);

  connect(model(), SIGNAL(dataChanged(QModelIndex,QModelIndex)),
          this, SLOT(InvalidateCachedCurrentPixmap()));
  connect(model(), SIGNAL(layoutAboutToBeChanged()),
          this, SLOT(RatingHoverOut()));
}

void PlaylistView::LoadGeometry() {
  QSettings settings;
  settings.beginGroup(Playlist::kSettingsGroup);

  QByteArray state(settings.value("state").toByteArray());
  if (!header_->RestoreState(state)) {
    // Maybe we're upgrading from a version that persisted the state with
    // QHeaderView.
    if (!header_->restoreState(state)) {
      header_->HideSection(Playlist::Column_Disc);
      header_->HideSection(Playlist::Column_Year);
      header_->HideSection(Playlist::Column_Genre);
      header_->HideSection(Playlist::Column_BPM);
      header_->HideSection(Playlist::Column_Bitrate);
      header_->HideSection(Playlist::Column_Samplerate);
      header_->HideSection(Playlist::Column_Filename);
      header_->HideSection(Playlist::Column_Filesize);
      header_->HideSection(Playlist::Column_Filetype);
      header_->HideSection(Playlist::Column_DateCreated);
      header_->HideSection(Playlist::Column_DateModified);
      header_->HideSection(Playlist::Column_AlbumArtist);
      header_->HideSection(Playlist::Column_Composer);
      header_->HideSection(Playlist::Column_Rating);
      header_->HideSection(Playlist::Column_PlayCount);
      header_->HideSection(Playlist::Column_SkipCount);
      header_->HideSection(Playlist::Column_LastPlayed);

      header_->moveSection(header_->visualIndex(Playlist::Column_Track), 0);
      setting_initial_header_layout_ = true;
    } else {
      upgrading_from_qheaderview_ = true;
    }
  }

  // New columns that we add are visible by default if the user has upgraded
  // Clementine.  Hide them again here
  const int state_version = settings.value("state_version", 0).toInt();
  if (state_version < 1) {
    header_->HideSection(Playlist::Column_Rating);
    header_->HideSection(Playlist::Column_PlayCount);
    header_->HideSection(Playlist::Column_SkipCount);
    header_->HideSection(Playlist::Column_LastPlayed);
  }
  if (state_version < 2) {
    header_->HideSection(Playlist::Column_Score);
  }
  if (state_version < 3) {
    header_->HideSection(Playlist::Column_Comment);
  }

  // Make sure at least one column is visible
  bool all_hidden = true;
  for (int i=0 ; i<header_->count() ; ++i) {
    if (!header_->isSectionHidden(i) && header_->sectionSize(i) > 0) {
      all_hidden = false;
      break;
    }
  }
  if (all_hidden) {
    header_->ShowSection(Playlist::Column_Title);
  }
}

void PlaylistView::SaveGeometry() {
  if (read_only_settings_)
    return;

  QSettings settings;
  settings.beginGroup(Playlist::kSettingsGroup);
  settings.setValue("state", header_->SaveState());
  settings.setValue("state_version", kStateVersion);
}

void PlaylistView::ReloadBarPixmaps() {
  currenttrack_bar_left_ = LoadBarPixmap(":currenttrack_bar_left.png");
  currenttrack_bar_mid_ = LoadBarPixmap(":currenttrack_bar_mid.png");
  currenttrack_bar_right_ = LoadBarPixmap(":currenttrack_bar_right.png");
}

QList<QPixmap> PlaylistView::LoadBarPixmap(const QString& filename) {
  QImage image(filename);
  image = image.scaledToHeight(row_height_, Qt::SmoothTransformation);

  // Colour the bar with the palette colour
  QPainter p(&image);
  p.setCompositionMode(QPainter::CompositionMode_SourceAtop);
  p.setOpacity(0.7);
  p.fillRect(image.rect(), QApplication::palette().color(QPalette::Highlight));
  p.end();

  // Animation steps
  QList<QPixmap> ret;
  for(int i=0 ; i<kGlowIntensitySteps ; ++i) {
    QImage step(image.copy());
    p.begin(&step);
    p.setCompositionMode(QPainter::CompositionMode_SourceAtop);
    p.setOpacity(0.4 - 0.6 * sin(float(i)/kGlowIntensitySteps * (M_PI/2)));
    p.fillRect(step.rect(), Qt::white);
    p.end();
    ret << QPixmap::fromImage(step);
  }

  return ret;
}

void PlaylistView::drawTree(QPainter* painter, const QRegion& region) const {
  const_cast<PlaylistView*>(this)->current_paint_region_ = region;
  QTreeView::drawTree(painter, region);
  const_cast<PlaylistView*>(this)->current_paint_region_ = QRegion();
}

void PlaylistView::drawRow(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const {
  QStyleOptionViewItemV4 opt(option);

  bool is_current = index.data(Playlist::Role_IsCurrent).toBool();
  bool is_paused = index.data(Playlist::Role_IsPaused).toBool();

  if (is_current) {
    const_cast<PlaylistView*>(this)->last_current_item_ = index;
    const_cast<PlaylistView*>(this)->last_glow_rect_ = opt.rect;

    int step = glow_intensity_step_;
    if (step >= kGlowIntensitySteps)
      step = 2*(kGlowIntensitySteps-1) - step + 1;

    int row_height = opt.rect.height();
    if (row_height != row_height_) {
      // Recreate the pixmaps if the height changed since last time
      const_cast<PlaylistView*>(this)->row_height_ = row_height;
      const_cast<PlaylistView*>(this)->ReloadBarPixmaps();
    }

    QRect middle(opt.rect);
    middle.setLeft(middle.left() + currenttrack_bar_left_[0].width());
    middle.setRight(middle.right() - currenttrack_bar_right_[0].width());

    // Selection
    if (selectionModel()->isSelected(index))
      painter->fillRect(opt.rect, opt.palette.color(QPalette::Highlight));

    // Draw the bar
    painter->drawPixmap(opt.rect.topLeft(), currenttrack_bar_left_[step]);
    painter->drawPixmap(opt.rect.topRight() - currenttrack_bar_right_[0].rect().topRight(), currenttrack_bar_right_[step]);
    painter->drawPixmap(middle, currenttrack_bar_mid_[step]);

    // Draw the play icon
    QPoint play_pos(currenttrack_bar_left_[0].width() / 3 * 2,
                    (row_height - currenttrack_play_.height()) / 2);
    painter->drawPixmap(opt.rect.topLeft() + play_pos,
                        is_paused ? currenttrack_pause_ : currenttrack_play_);

    // Set the font
    opt.palette.setColor(QPalette::Text, Qt::white);
    opt.palette.setColor(QPalette::HighlightedText, Qt::white);
    opt.palette.setColor(QPalette::Highlight, Qt::transparent);
    opt.palette.setColor(QPalette::AlternateBase, Qt::transparent);
    opt.font.setItalic(true);
    opt.decorationSize = QSize(20,20);

    // Draw the actual row data on top.  We cache this, because it's fairly
    // expensive (1-2ms), and we do it many times per second.
    const bool cache_dirty = cached_current_row_rect_ != opt.rect ||
                             cached_current_row_row_ != index.row() ||
                             cached_current_row_.isNull();

    // We can't update the cache if we're not drawing the entire region,
    // QTreeView clips its drawing to only the columns in the region, so it
    // wouldn't update the whole pixmap properly.
    const bool whole_region =
        current_paint_region_.boundingRect().width() == viewport()->width();

    if (!cache_dirty) {
      painter->drawPixmap(opt.rect, cached_current_row_);
    } else {
      if (whole_region) {
        const_cast<PlaylistView*>(this)->UpdateCachedCurrentRowPixmap(opt, index);
        painter->drawPixmap(opt.rect, cached_current_row_);
      } else {
        QTreeView::drawRow(painter, opt, index);
      }
    }
  } else {
    QTreeView::drawRow(painter, opt, index);
  }
}

void PlaylistView::UpdateCachedCurrentRowPixmap(QStyleOptionViewItemV4 option,
                                                const QModelIndex& index) {
  cached_current_row_rect_ = option.rect;
  cached_current_row_row_ = index.row();

  option.rect.moveTo(0, 0);
  cached_current_row_ = QPixmap(option.rect.size());
  cached_current_row_.fill(Qt::transparent);

  QPainter p(&cached_current_row_);
  QTreeView::drawRow(&p, option, index);
}

void PlaylistView::InvalidateCachedCurrentPixmap() {
  cached_current_row_ = QPixmap();
}

void PlaylistView::timerEvent(QTimerEvent* event) {
  QTreeView::timerEvent(event);
  if (event->timerId() == glow_timer_.timerId())
    GlowIntensityChanged();
}

void PlaylistView::GlowIntensityChanged() {
  glow_intensity_step_ = (glow_intensity_step_ + 1) % (kGlowIntensitySteps * 2);

  viewport()->update(last_glow_rect_);
}

void PlaylistView::StopGlowing() {
  currently_glowing_ = false;
  glow_timer_.stop();
  glow_intensity_step_ = kGlowIntensitySteps;
}

void PlaylistView::StartGlowing() {
  currently_glowing_ = true;
  if (isVisible() && glow_enabled_)
    glow_timer_.start(1500 / kGlowIntensitySteps, this);
}

void PlaylistView::hideEvent(QHideEvent*) {
  glow_timer_.stop();
}

void PlaylistView::showEvent(QShowEvent*) {
  if (currently_glowing_ && glow_enabled_)
    glow_timer_.start(1500 / kGlowIntensitySteps, this);
  MaybeAutoscroll();
}

bool CompareSelectionRanges(const QItemSelectionRange& a, const QItemSelectionRange& b) {
  return b.bottom() < a.bottom();
}

void PlaylistView::keyPressEvent(QKeyEvent* event) {
  if (!model()) {
    QTreeView::keyPressEvent(event);
  } else if (state() == QAbstractItemView::EditingState) {
    QTreeView::keyPressEvent(event);
  } else if (event->matches(QKeySequence::Delete)) {
    RemoveSelected();
    event->accept();
#ifdef Q_OS_DARWIN
  } else if (event->key() == Qt::Key_Backspace) {
    RemoveSelected();
    event->accept();
#endif
  } else if (event->key() == Qt::Key_Enter ||
             event->key() == Qt::Key_Return) {
    if (currentIndex().isValid())
      emit PlayItem(currentIndex());
    event->accept();
  } else if(event->modifiers() != Qt::ControlModifier //Ctrl+Space selects the item
      && event->key() == Qt::Key_Space) {
    emit PlayPause();
    event->accept();
  } else if(event->key() == Qt::Key_Left) {
    emit SeekTrack(-1);
    event->accept();
  } else if(event->key() == Qt::Key_Right) {
    emit SeekTrack(1);
    event->accept();
  } else if(event->modifiers() == Qt::NoModifier // No modifier keys currently pressed...
      // ... and key pressed is something related to text
      && ( (event->key() >= Qt::Key_A && event->key() <= Qt::Key_Z)
        || event->key() == Qt::Key_Backspace
        || event->key() == Qt::Key_Escape)) {
    emit FocusOnFilterSignal(event);
    event->accept();
  } else {
    QTreeView::keyPressEvent(event);
  }
}

void PlaylistView::contextMenuEvent(QContextMenuEvent* e) {
  emit RightClicked(e->globalPos(), indexAt(e->pos()));
  e->accept();
}

void PlaylistView::RemoveSelected() {
  int rows_removed = 0;
  QItemSelection selection(selectionModel()->selection());

  if (selection.isEmpty()) {
    return;
  }

  // Sort the selection so we remove the items at the *bottom* first, ensuring
  // we don't have to mess around with changing row numbers
  qSort(selection.begin(), selection.end(), CompareSelectionRanges);

  // Store the last selected row, which is the first in the list
  int last_row = selection.first().bottom();

  foreach (const QItemSelectionRange& range, selection) {
    rows_removed += range.height();
    model()->removeRows(range.top(), range.height(), range.parent());
  }

  int new_row = last_row-rows_removed+1;
  // Index of the first column for the row to select
  QModelIndex new_index = model()->index(new_row, 0);

  // Select the new current item, we want always the item after the last selected
  if (new_index.isValid()) {
    // Update visual selection with the entire row
    selectionModel()->select(QItemSelection(new_index, model()->index(new_row, model()->columnCount()-1)),
                                            QItemSelectionModel::Select);
    // Update keyboard selected row, if it's not the first row
    if (new_row != 0)
      keyPressEvent(new QKeyEvent(QEvent::KeyPress, Qt::Key_Down, Qt::NoModifier));
  } else {
    // We're removing the last item, select the new last row
    selectionModel()->select(QItemSelection(model()->index(model()->rowCount()-1, 0),
                             model()->index(model()->rowCount()-1, model()->columnCount()-1)),
                             QItemSelectionModel::Select);
  }
}

QList<int> PlaylistView::GetEditableColumns() {
  QList<int> columns;
  QHeaderView* h = header();
  for (int col=0; col<h->count(); col++) {
    if (h->isSectionHidden(col))
      continue;
    QModelIndex index = model()->index(0, col);
    if (index.flags() & Qt::ItemIsEditable)
      columns << h->visualIndex(col);
  }
  qSort(columns);
  return columns;
}

QModelIndex PlaylistView::NextEditableIndex(const QModelIndex& current) {
  QList<int> columns = GetEditableColumns();
  QHeaderView* h = header();
  int index = columns.indexOf(h->visualIndex(current.column()));

  if(index+1 >= columns.size())
    return model()->index(current.row()+1, h->logicalIndex(columns.first()));

  return model()->index(current.row(), h->logicalIndex(columns[index+1]));
}

QModelIndex PlaylistView::PrevEditableIndex(const QModelIndex& current) {
  QList<int> columns = GetEditableColumns();
  QHeaderView* h = header();
  int index = columns.indexOf(h->visualIndex(current.column()));

  if(index-1 < 0)
    return model()->index(current.row()-1, h->logicalIndex(columns.last()));

  return model()->index(current.row(), h->logicalIndex(columns[index-1]));
}

void PlaylistView::closeEditor(QWidget* editor, QAbstractItemDelegate::EndEditHint hint) {
  if (hint == QAbstractItemDelegate::NoHint) {
    QTreeView::closeEditor(editor, QAbstractItemDelegate::SubmitModelCache);
  } else if (hint == QAbstractItemDelegate::EditNextItem ||
             hint == QAbstractItemDelegate::EditPreviousItem) {

    QModelIndex index;
    if (hint == QAbstractItemDelegate::EditNextItem)
      index = NextEditableIndex(currentIndex());
    else
      index = PrevEditableIndex(currentIndex());

    if (!index.isValid()) {
      QTreeView::closeEditor(editor, QAbstractItemDelegate::SubmitModelCache);
    } else {
      QTreeView::closeEditor(editor, QAbstractItemDelegate::NoHint);
      setCurrentIndex(index);
      edit(index);
    }
  } else {
    QTreeView::closeEditor(editor, hint);
  }
}

void PlaylistView::mouseMoveEvent(QMouseEvent* event) {
  QModelIndex index = indexAt(event->pos());
  if (index.isValid() && index.data(Playlist::Role_CanSetRating).toBool()) {
    RatingHoverIn(index, event->pos());
  } else if (rating_delegate_->is_mouse_over()) {
    RatingHoverOut();
  }
  if (!drag_over_) {
    QTreeView::mouseMoveEvent(event);
  }
}

void PlaylistView::leaveEvent(QEvent* e) {
  if (rating_delegate_->is_mouse_over()) {
    RatingHoverOut();
  }
  QTreeView::leaveEvent(e);
}

void PlaylistView::RatingHoverIn(const QModelIndex& index, const QPoint& pos) {
  if (!(editTriggers() & QAbstractItemView::SelectedClicked)) {
    return;
  }

  const QModelIndex old_index = rating_delegate_->mouse_over_index();
  rating_delegate_->set_mouse_over(index, selectedIndexes(), pos);
  setCursor(Qt::PointingHandCursor);

  update(index);
  update(old_index);
  foreach (const QModelIndex& index, selectedIndexes()) {
    if (index.column() == Playlist::Column_Rating) {
      update(index);
    }
  }

  if (index.data(Playlist::Role_IsCurrent).toBool() ||
      old_index.data(Playlist::Role_IsCurrent).toBool()) {
    InvalidateCachedCurrentPixmap();
  }
}

void PlaylistView::RatingHoverOut() {
  if (!(editTriggers() & QAbstractItemView::SelectedClicked)) {
    return;
  }

  const QModelIndex old_index = rating_delegate_->mouse_over_index();
  rating_delegate_->set_mouse_out();
  setCursor(QCursor());

  update(old_index);
  foreach (const QModelIndex& index, selectedIndexes()) {
    if (index.column() == Playlist::Column_Rating) {
      update(index);
    }
  }

  if (old_index.data(Playlist::Role_IsCurrent).toBool()) {
    InvalidateCachedCurrentPixmap();
  }
}

void PlaylistView::mousePressEvent(QMouseEvent* event) {
  if (!(editTriggers() & QAbstractItemView::SelectedClicked)) {
    QTreeView::mousePressEvent(event);
    return;
  }

  QModelIndex index = indexAt(event->pos());
  if (event->button() == Qt::LeftButton && index.isValid() &&
      index.data(Playlist::Role_CanSetRating).toBool()) {
    // Calculate which star was clicked
    double new_rating = RatingPainter::RatingForPos(
        event->pos(), visualRect(index));

    if (selectedIndexes().contains(index)) {
      // Update all the selected items
      foreach (const QModelIndex& index, selectedIndexes()) {
        if (index.data(Playlist::Role_CanSetRating).toBool()) {
          playlist_->RateSong(playlist_->proxy()->mapToSource(index), new_rating);
        }
      }
    } else {
      // Update only this item
      playlist_->RateSong(playlist_->proxy()->mapToSource(index), new_rating);
    }
  } else {
    QTreeView::mousePressEvent(event);
  }

  inhibit_autoscroll_ = true;
  inhibit_autoscroll_timer_->start();
}

void PlaylistView::scrollContentsBy(int dx, int dy) {
  if (dx) {
    InvalidateCachedCurrentPixmap();
  }
  cached_tree_ = QPixmap();

  QTreeView::scrollContentsBy(dx, dy);

  if (!currently_autoscrolling_) {
    // We only want to do this if the scroll was initiated by the user
    inhibit_autoscroll_ = true;
    inhibit_autoscroll_timer_->start();
  }
}

void PlaylistView::InhibitAutoscrollTimeout() {
  // For 1 minute after the user clicks on or scrolls the playlist we promise
  // not to automatically scroll the view to keep up with a track change.
  inhibit_autoscroll_ = false;
}

void PlaylistView::MaybeAutoscroll() {
  if (!inhibit_autoscroll_)
    JumpToCurrentlyPlayingTrack();
}

void PlaylistView::JumpToCurrentlyPlayingTrack() {
  Q_ASSERT(playlist_);

  if (playlist_->current_row() == -1)
    return;

  QModelIndex current = playlist_->proxy()->mapFromSource(
      playlist_->index(playlist_->current_row(), 0));
  if (!current.isValid())
    return;

  currently_autoscrolling_ = true;

  // Scroll to the item
  scrollTo(current, QAbstractItemView::PositionAtCenter);

  currently_autoscrolling_ = false;
}

void PlaylistView::JumpToLastPlayedTrack() {
  Q_ASSERT(playlist_);

  if (playlist_->last_played_row() == -1)
    return;

  QModelIndex last_played = playlist_->proxy()->mapFromSource(
      playlist_->index(playlist_->last_played_row(), 0));
  if (!last_played.isValid())
    return;

  // Select last played song
  last_current_item_ = last_played;
  setCurrentIndex(last_current_item_);

  currently_autoscrolling_ = true;

  // Scroll to the item
  scrollTo(last_played, QAbstractItemView::PositionAtCenter);

  currently_autoscrolling_ = false;
}

void PlaylistView::paintEvent(QPaintEvent* event) {
  // Reimplemented to draw the drop indicator
  // When the user is dragging some stuff over the playlist paintEvent gets
  // called for the entire viewport every time the user moves the mouse.
  // The drawTree is kinda expensive, so we cache the result and draw from the
  // cache while the user is dragging.  The cached pixmap gets invalidated in
  // dragLeaveEvent, dropEvent and scrollContentsBy.

  QPainter p(viewport());

  if (drop_indicator_row_ != -1) {
    if (cached_tree_.isNull()) {
      cached_tree_ = QPixmap(size());
      cached_tree_.fill(Qt::transparent);

      QPainter cache_painter(&cached_tree_);
      drawTree(&cache_painter, event->region());
    }

    p.drawPixmap(0, 0, cached_tree_);
  } else {
    drawTree(&p, event->region());
    return;
  }

  // Find the y position of the drop indicator
  QModelIndex drop_index = model()->index(drop_indicator_row_, 0);
  int drop_pos = -1;
  switch (dropIndicatorPosition()) {
    case QAbstractItemView::OnItem:
      return; // Don't draw anything

    case QAbstractItemView::AboveItem:
      drop_pos = visualRect(drop_index).top();
      break;

    case QAbstractItemView::BelowItem:
      drop_pos = visualRect(drop_index).bottom() + 1;
      break;

    case QAbstractItemView::OnViewport:
      if (model()->rowCount() == 0)
        drop_pos = 1;
      else
        drop_pos = visualRect(model()->index(model()->rowCount() - 1, 0)).bottom() + 1;
      break;
  }

  // Draw a nice gradient first
  QColor line_color(QApplication::palette().color(QPalette::Highlight));
  QColor shadow_color(line_color.lighter(140));
  QColor shadow_fadeout_color(shadow_color);
  shadow_color.setAlpha(255);
  shadow_fadeout_color.setAlpha(0);

  QLinearGradient gradient(QPoint(0, drop_pos - kDropIndicatorGradientWidth),
                           QPoint(0, drop_pos + kDropIndicatorGradientWidth));
  gradient.setColorAt(0.0, shadow_fadeout_color);
  gradient.setColorAt(0.5, shadow_color);
  gradient.setColorAt(1.0, shadow_fadeout_color);
  QPen gradient_pen(QBrush(gradient), kDropIndicatorGradientWidth * 2);
  p.setPen(gradient_pen);
  p.drawLine(QPoint(0, drop_pos),
             QPoint(width(), drop_pos));

  // Now draw the line on top
  QPen line_pen(line_color, kDropIndicatorWidth);
  p.setPen(line_pen);
  p.drawLine(QPoint(0, drop_pos),
             QPoint(width(), drop_pos));
}

void PlaylistView::dragMoveEvent(QDragMoveEvent *event) {
  QTreeView::dragMoveEvent(event);

  QModelIndex index(indexAt(event->pos()));
  drop_indicator_row_ = index.isValid() ? index.row() : 0;
}

void PlaylistView::dragEnterEvent(QDragEnterEvent *event) {
  QTreeView::dragEnterEvent(event);
  cached_tree_ = QPixmap();
  drag_over_ = true;
}

void PlaylistView::dragLeaveEvent(QDragLeaveEvent *event) {
  QTreeView::dragLeaveEvent(event);
  cached_tree_ = QPixmap();
  drag_over_ = false;
  drop_indicator_row_ = -1;
}

void PlaylistView::dropEvent(QDropEvent *event) {
  QTreeView::dropEvent(event);
  cached_tree_ = QPixmap();
  drop_indicator_row_ = -1;
  drag_over_ = false;
}

void PlaylistView::PlaylistDestroyed() {
  playlist_ = NULL;
  // We'll get a SetPlaylist() soon
}

void PlaylistView::ReloadSettings() {
  QSettings s;
  s.beginGroup(Playlist::kSettingsGroup);
  glow_enabled_ = s.value("glow_effect", true).toBool();
  background_enabled_ = s.value("bg_enabled", true).toBool();

  if (setting_initial_header_layout_ || upgrading_from_qheaderview_) {
    header_->SetStretchEnabled(s.value("stretch", true).toBool());
    upgrading_from_qheaderview_ = false;
  }

  if (currently_glowing_ && glow_enabled_ && isVisible())
    StartGlowing();
  if (!glow_enabled_)
    StopGlowing();

  setProperty("background_enabled", background_enabled_);
  
  if (setting_initial_header_layout_) {
    header_->SetColumnWidth(Playlist::Column_Length, 0.06);
    header_->SetColumnWidth(Playlist::Column_Track, 0.05);
    setting_initial_header_layout_ = false;
  }

  ColumnAlignmentMap column_alignments = s.value("column_alignments").value<ColumnAlignmentMap>();
  if (!column_alignments.isEmpty()) playlist_->set_column_alignments(column_alignments);
}

void PlaylistView::SaveSettings() {
  if (read_only_settings_)
    return;

  QSettings s;
  s.beginGroup(Playlist::kSettingsGroup);
  s.setValue("glow_effect", glow_enabled_);
  s.setValue("column_alignments", QVariant::fromValue(playlist_->column_alignments()));
  s.setValue("bg_enabled", background_enabled_);
}

void PlaylistView::StretchChanged(bool stretch) {
  setHorizontalScrollBarPolicy(stretch ? Qt::ScrollBarAlwaysOff : Qt::ScrollBarAsNeeded);
  SaveGeometry();
}

void PlaylistView::DynamicModeChanged(bool dynamic) {
  if (!dynamic) {
    dynamic_controls_->hide();
  } else {
    RepositionDynamicControls();
    dynamic_controls_->show();
  }
}

void PlaylistView::resizeEvent(QResizeEvent* e) {
  QTreeView::resizeEvent(e);
  if (dynamic_controls_->isVisible()) {
    RepositionDynamicControls();
  }
}

void PlaylistView::RepositionDynamicControls() {
  dynamic_controls_->resize(dynamic_controls_->sizeHint());
  dynamic_controls_->move((width() - dynamic_controls_->width()) / 2,
                          height() - dynamic_controls_->height() - 20);
}

bool PlaylistView::eventFilter(QObject* object, QEvent* event) {
  if (event->type() == QEvent::Enter &&
      (object == horizontalScrollBar() || object == verticalScrollBar())) {
    RatingHoverOut();
    return false;
  }
  return QObject::eventFilter(object, event);
}
