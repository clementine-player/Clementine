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

#include "config.h"
#include "dynamicplaylistcontrols.h"
#include "playlist.h"
#include "playlistdelegates.h"
#include "playlistheader.h"
#include "playlistview.h"
#include "core/application.h"
#include "core/logging.h"
#include "core/player.h"
#include "covers/currentartloader.h"
#include "ui/qt_blurimage.h"
#include "ui/iconloader.h"

#include <QCleanlooksStyle>
#include <QClipboard>
#include <QPainter>
#include <QHeaderView>
#include <QSettings>
#include <QtDebug>
#include <QTimer>
#include <QKeyEvent>
#include <QApplication>
#include <QSortFilterProxyModel>
#include <QScrollBar>
#include <QTimeLine>

#include <math.h>
#include <algorithm>

#ifdef HAVE_MOODBAR
#include "moodbar/moodbaritemdelegate.h"
#endif

const int PlaylistView::kStateVersion = 6;
const int PlaylistView::kGlowIntensitySteps = 24;
const int PlaylistView::kAutoscrollGraceTimeout = 30;  // seconds
const int PlaylistView::kDropIndicatorWidth = 2;
const int PlaylistView::kDropIndicatorGradientWidth = 5;
const char* PlaylistView::kSettingBackgroundImageType =
    "playlistview_background_type";
const char* PlaylistView::kSettingBackgroundImageFilename =
    "playlistview_background_image_file";

const int PlaylistView::kDefaultBlurRadius = 0;
const int PlaylistView::kDefaultOpacityLevel = 40;

PlaylistProxyStyle::PlaylistProxyStyle(QStyle* base)
    : QProxyStyle(base), cleanlooks_(new QCleanlooksStyle) {}

void PlaylistProxyStyle::drawControl(ControlElement element,
                                     const QStyleOption* option,
                                     QPainter* painter,
                                     const QWidget* widget) const {
  if (element == CE_Header) {
    const QStyleOptionHeader* header_option =
        qstyleoption_cast<const QStyleOptionHeader*>(option);
    const QRect& rect = header_option->rect;
    const QString& text = header_option->text;
    const QFontMetrics& font_metrics = header_option->fontMetrics;

    // spaces added to make transition less abrupt
    if (rect.width() < font_metrics.width(text + "  ")) {
      const Playlist::Column column =
          static_cast<Playlist::Column>(header_option->section);
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

void PlaylistProxyStyle::drawPrimitive(PrimitiveElement element,
                                       const QStyleOption* option,
                                       QPainter* painter,
                                       const QWidget* widget) const {
  if (element == QStyle::PE_PanelItemViewRow ||
      element == QStyle::PE_PanelItemViewItem)
    cleanlooks_->drawPrimitive(element, option, painter, widget);
  else
    QProxyStyle::drawPrimitive(element, option, painter, widget);
}

PlaylistView::PlaylistView(QWidget* parent)
    : QTreeView(parent),
      app_(nullptr),
      style_(new PlaylistProxyStyle(style())),
      playlist_(nullptr),
      header_(new PlaylistHeader(Qt::Horizontal, this, this)),
      setting_initial_header_layout_(false),
      upgrading_from_qheaderview_(false),
      read_only_settings_(true),
      upgrading_from_version_(-1),
      background_initialized_(false),
      background_image_type_(Default),
      blur_radius_(kDefaultBlurRadius),
      opacity_level_(kDefaultOpacityLevel),
      previous_background_image_opacity_(0.0),
      fade_animation_(new QTimeLine(1000, this)),
      last_height_(-1),
      last_width_(-1),
      force_background_redraw_(false),
      glow_enabled_(false),
      currently_glowing_(false),
      glow_intensity_step_(0),
      rating_delegate_(nullptr),
      inhibit_autoscroll_timer_(new QTimer(this)),
      inhibit_autoscroll_(false),
      currently_autoscrolling_(false),
      row_height_(-1),
      cached_current_row_row_(-1),
      drop_indicator_row_(-1),
      drag_over_(false),
      dynamic_controls_(new DynamicPlaylistControls(this)) {
  setHeader(header_);
  header_->setMovable(true);
  setStyle(style_);
  setMouseTracking(true);

  QIcon currenttrack_play =
      IconLoader::Load("currenttrack_play", IconLoader::Other);
  currenttrack_play_ =
      currenttrack_play.pixmap(currenttrack_play.availableSizes().last());
  QIcon currenttrack_pause =
      IconLoader::Load("currenttrack_pause", IconLoader::Other);
  currenttrack_pause_ =
      currenttrack_pause.pixmap(currenttrack_pause.availableSizes().last());

  connect(header_, SIGNAL(sectionResized(int, int, int)), SLOT(SaveGeometry()));
  connect(header_, SIGNAL(sectionMoved(int, int, int)), SLOT(SaveGeometry()));
  connect(header_, SIGNAL(sortIndicatorChanged(int, Qt::SortOrder)),
          SLOT(SaveGeometry()));
  connect(header_, SIGNAL(SectionVisibilityChanged(int, bool)),
          SLOT(SaveGeometry()));
  connect(header_, SIGNAL(SectionRatingLockStatusChanged(bool)),
          SLOT(SetRatingLockStatus(bool)));
  connect(header_, SIGNAL(sectionResized(int, int, int)),
          SLOT(InvalidateCachedCurrentPixmap()));
  connect(header_, SIGNAL(sectionMoved(int, int, int)),
          SLOT(InvalidateCachedCurrentPixmap()));
  connect(header_, SIGNAL(SectionVisibilityChanged(int, bool)),
          SLOT(InvalidateCachedCurrentPixmap()));
  connect(header_, SIGNAL(StretchEnabledChanged(bool)), SLOT(SaveSettings()));
  connect(header_, SIGNAL(StretchEnabledChanged(bool)),
          SLOT(StretchChanged(bool)));
  connect(header_, SIGNAL(MouseEntered()), SLOT(RatingHoverOut()));

  inhibit_autoscroll_timer_->setInterval(kAutoscrollGraceTimeout * 1000);
  inhibit_autoscroll_timer_->setSingleShot(true);
  connect(inhibit_autoscroll_timer_, SIGNAL(timeout()),
          SLOT(InhibitAutoscrollTimeout()));

  horizontalScrollBar()->installEventFilter(this);
  verticalScrollBar()->installEventFilter(this);

  setAlternatingRowColors(true);

  setAttribute(Qt::WA_MacShowFocusRect, false);

  dynamic_controls_->hide();

#ifdef Q_OS_DARWIN
  setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
#endif
  // For fading
  connect(fade_animation_, SIGNAL(valueChanged(qreal)),
          SLOT(FadePreviousBackgroundImage(qreal)));
  fade_animation_->setDirection(QTimeLine::Backward);  // 1.0 -> 0.0
}

void PlaylistView::SetApplication(Application* app) {
  Q_ASSERT(app);
  app_ = app;
  connect(app_->current_art_loader(),
          SIGNAL(ArtLoaded(const Song&, const QString&, const QImage&)),
          SLOT(CurrentSongChanged(const Song&, const QString&, const QImage&)));
  connect(app_->player(), SIGNAL(Paused()), SLOT(StopGlowing()));
  connect(app_->player(), SIGNAL(Playing()), SLOT(StartGlowing()));
  connect(app_->player(), SIGNAL(Stopped()), SLOT(StopGlowing()));
  connect(app_->player(), SIGNAL(Stopped()), SLOT(PlayerStopped()));
}

void PlaylistView::SetItemDelegates(LibraryBackend* backend) {
  rating_delegate_ = new RatingItemDelegate(this);

  setItemDelegate(new PlaylistDelegateBase(this));
  setItemDelegateForColumn(Playlist::Column_Title, new TextItemDelegate(this));
  setItemDelegateForColumn(
      Playlist::Column_Album,
      new TagCompletionItemDelegate(this, backend, Playlist::Column_Album));
  setItemDelegateForColumn(
      Playlist::Column_Artist,
      new TagCompletionItemDelegate(this, backend, Playlist::Column_Artist));
  setItemDelegateForColumn(Playlist::Column_AlbumArtist,
                           new TagCompletionItemDelegate(
                               this, backend, Playlist::Column_AlbumArtist));
  setItemDelegateForColumn(
      Playlist::Column_Genre,
      new TagCompletionItemDelegate(this, backend, Playlist::Column_Genre));
  setItemDelegateForColumn(
      Playlist::Column_Composer,
      new TagCompletionItemDelegate(this, backend, Playlist::Column_Composer));
  setItemDelegateForColumn(
      Playlist::Column_Performer,
      new TagCompletionItemDelegate(this, backend, Playlist::Column_Performer));
  setItemDelegateForColumn(
      Playlist::Column_Grouping,
      new TagCompletionItemDelegate(this, backend, Playlist::Column_Grouping));
  setItemDelegateForColumn(Playlist::Column_Length,
                           new LengthItemDelegate(this));
  setItemDelegateForColumn(Playlist::Column_Filesize,
                           new SizeItemDelegate(this));
  setItemDelegateForColumn(Playlist::Column_Filetype,
                           new FileTypeItemDelegate(this));
  setItemDelegateForColumn(Playlist::Column_DateCreated,
                           new DateItemDelegate(this));
  setItemDelegateForColumn(Playlist::Column_DateModified,
                           new DateItemDelegate(this));
  setItemDelegateForColumn(Playlist::Column_BPM,
                           new PlaylistDelegateBase(this, tr("bpm")));
  setItemDelegateForColumn(Playlist::Column_Samplerate,
                           new PlaylistDelegateBase(this, ("Hz")));
  setItemDelegateForColumn(Playlist::Column_Bitrate,
                           new PlaylistDelegateBase(this, tr("kbps")));
  setItemDelegateForColumn(Playlist::Column_Filename,
                           new NativeSeparatorsDelegate(this));
  setItemDelegateForColumn(Playlist::Column_Rating, rating_delegate_);
  setItemDelegateForColumn(Playlist::Column_LastPlayed,
                           new LastPlayedItemDelegate(this));

#ifdef HAVE_MOODBAR
  setItemDelegateForColumn(Playlist::Column_Mood,
                           new MoodbarItemDelegate(app_, this, this));
#endif

  if (app_ && app_->player()) {
    setItemDelegateForColumn(Playlist::Column_Source,
                             new SongSourceDelegate(this, app_->player()));
  } else {
    header_->HideSection(Playlist::Column_Source);
  }
}

void PlaylistView::SetPlaylist(Playlist* playlist) {
  if (playlist_) {
    disconnect(playlist_, SIGNAL(CurrentSongChanged(Song)), this,
               SLOT(MaybeAutoscroll()));
    disconnect(playlist_, SIGNAL(DynamicModeChanged(bool)), this,
               SLOT(DynamicModeChanged(bool)));
    disconnect(playlist_, SIGNAL(destroyed()), this, SLOT(PlaylistDestroyed()));
    disconnect(playlist_, SIGNAL(QueueChanged()), this, SLOT(update()));

    disconnect(dynamic_controls_, SIGNAL(Expand()), playlist_,
               SLOT(ExpandDynamicPlaylist()));
    disconnect(dynamic_controls_, SIGNAL(Repopulate()), playlist_,
               SLOT(RepopulateDynamicPlaylist()));
    disconnect(dynamic_controls_, SIGNAL(TurnOff()), playlist_,
               SLOT(TurnOffDynamicPlaylist()));
  }

  playlist_ = playlist;
  LoadGeometry();
  LoadRatingLockStatus();
  ReloadSettings();
  DynamicModeChanged(playlist->is_dynamic());
  setFocus();
  read_only_settings_ = false;
  JumpToLastPlayedTrack();

  connect(playlist_, SIGNAL(RestoreFinished()), SLOT(JumpToLastPlayedTrack()));
  connect(playlist_, SIGNAL(CurrentSongChanged(Song)), SLOT(MaybeAutoscroll()));
  connect(playlist_, SIGNAL(DynamicModeChanged(bool)),
          SLOT(DynamicModeChanged(bool)));
  connect(playlist_, SIGNAL(destroyed()), SLOT(PlaylistDestroyed()));
  connect(playlist_, SIGNAL(QueueChanged()), SLOT(update()));

  connect(dynamic_controls_, SIGNAL(Expand()), playlist_,
          SLOT(ExpandDynamicPlaylist()));
  connect(dynamic_controls_, SIGNAL(Repopulate()), playlist_,
          SLOT(RepopulateDynamicPlaylist()));
  connect(dynamic_controls_, SIGNAL(TurnOff()), playlist_,
          SLOT(TurnOffDynamicPlaylist()));
}

void PlaylistView::setModel(QAbstractItemModel* m) {
  if (model()) {
    disconnect(model(), SIGNAL(dataChanged(QModelIndex, QModelIndex)), this,
               SLOT(InvalidateCachedCurrentPixmap()));
    disconnect(model(), SIGNAL(layoutAboutToBeChanged()), this,
               SLOT(RatingHoverOut()));
    // When changing the model, always invalidate the current pixmap.
    // If a remote client uses "stop after", without invaliding the stop
    // mark would not appear.
    InvalidateCachedCurrentPixmap();
  }

  QTreeView::setModel(m);

  connect(model(), SIGNAL(dataChanged(QModelIndex, QModelIndex)), this,
          SLOT(InvalidateCachedCurrentPixmap()));
  connect(model(), SIGNAL(layoutAboutToBeChanged()), this,
          SLOT(RatingHoverOut()));
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
      header_->HideSection(Playlist::Column_OriginalYear);
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
      header_->HideSection(Playlist::Column_Performer);
      header_->HideSection(Playlist::Column_Grouping);
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
  upgrading_from_version_ = state_version;

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
  if (state_version < 5) {
    header_->HideSection(Playlist::Column_Mood);
  }
  if (state_version < 6) {
    header_->HideSection(Playlist::Column_Performer);
    header_->HideSection(Playlist::Column_Grouping);
  }

  // Make sure at least one column is visible
  bool all_hidden = true;
  for (int i = 0; i < header_->count(); ++i) {
    if (!header_->isSectionHidden(i) && header_->sectionSize(i) > 0) {
      all_hidden = false;
      break;
    }
  }
  if (all_hidden) {
    header_->ShowSection(Playlist::Column_Title);
  }
}

void PlaylistView::LoadRatingLockStatus() {
  QSettings s;
  s.beginGroup(Playlist::kSettingsGroup);
  ratings_locked_ = s.value("RatingLocked", false).toBool();
}

void PlaylistView::SaveGeometry() {
  if (read_only_settings_) return;

  QSettings settings;
  settings.beginGroup(Playlist::kSettingsGroup);
  settings.setValue("state", header_->SaveState());
  settings.setValue("state_version", kStateVersion);
}

void PlaylistView::SetRatingLockStatus(bool state) {
  if (read_only_settings_) return;

  ratings_locked_ = state;
  QSettings s;
  s.beginGroup(Playlist::kSettingsGroup);
  s.setValue("RatingLocked", state);
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
  for (int i = 0; i < kGlowIntensitySteps; ++i) {
    QImage step(image.copy());
    p.begin(&step);
    p.setCompositionMode(QPainter::CompositionMode_SourceAtop);
    p.setOpacity(0.4 - 0.6 * sin(float(i) / kGlowIntensitySteps * (M_PI / 2)));
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

void PlaylistView::drawRow(QPainter* painter,
                           const QStyleOptionViewItem& option,
                           const QModelIndex& index) const {
  QStyleOptionViewItemV4 opt(option);

  bool is_current = index.data(Playlist::Role_IsCurrent).toBool();
  bool is_paused = index.data(Playlist::Role_IsPaused).toBool();

  if (is_current) {
    const_cast<PlaylistView*>(this)->last_current_item_ = index;
    const_cast<PlaylistView*>(this)->last_glow_rect_ = opt.rect;

    int step = glow_intensity_step_;
    if (step >= kGlowIntensitySteps)
      step = 2 * (kGlowIntensitySteps - 1) - step + 1;

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
    painter->drawPixmap(
        opt.rect.topRight() - currenttrack_bar_right_[0].rect().topRight(),
        currenttrack_bar_right_[step]);
    painter->drawPixmap(middle, currenttrack_bar_mid_[step]);

    // Draw the play icon
    QPoint play_pos(currenttrack_bar_left_[0].width() / 3 * 2,
                    (row_height - currenttrack_play_.height()) / 2);
    painter->drawPixmap(opt.rect.topLeft() + play_pos,
                        is_paused ? currenttrack_pause_ : currenttrack_play_);

    // Set the font
    opt.palette.setColor(QPalette::Inactive, QPalette::HighlightedText,
                         QApplication::palette().color(
                             QPalette::Active, QPalette::HighlightedText));
    opt.palette.setColor(QPalette::Text, QApplication::palette().color(
                                             QPalette::HighlightedText));
    opt.palette.setColor(QPalette::Highlight, Qt::transparent);
    opt.palette.setColor(QPalette::AlternateBase, Qt::transparent);
    opt.decorationSize = QSize(20, 20);

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
        const_cast<PlaylistView*>(this)
            ->UpdateCachedCurrentRowPixmap(opt, index);
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
  if (event->timerId() == glow_timer_.timerId()) GlowIntensityChanged();
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

void PlaylistView::hideEvent(QHideEvent*) { glow_timer_.stop(); }

void PlaylistView::showEvent(QShowEvent*) {
  if (currently_glowing_ && glow_enabled_)
    glow_timer_.start(1500 / kGlowIntensitySteps, this);
  MaybeAutoscroll();
}

bool CompareSelectionRanges(const QItemSelectionRange& a,
                            const QItemSelectionRange& b) {
  return b.bottom() < a.bottom();
}

void PlaylistView::keyPressEvent(QKeyEvent* event) {
  if (!model() || state() == QAbstractItemView::EditingState) {
    QTreeView::keyPressEvent(event);
  } else if (event == QKeySequence::Delete) {
    RemoveSelected(false);
    event->accept();
#ifdef Q_OS_DARWIN
  } else if (event->key() == Qt::Key_Backspace) {
    RemoveSelected(false);
    event->accept();
#endif
  } else if (event == QKeySequence::Copy) {
    CopyCurrentSongToClipboard();
  } else if (event->key() == Qt::Key_Enter || event->key() == Qt::Key_Return) {
    if (currentIndex().isValid()) emit PlayItem(currentIndex());
    event->accept();
  } else if (event->modifiers() != Qt::ControlModifier  // Ctrl+Space selects
                                                        // the item
             &&
             event->key() == Qt::Key_Space) {
    emit PlayPause();
    event->accept();
  } else if (event->key() == Qt::Key_Left) {
    emit SeekBackward();
    event->accept();
  } else if (event->key() == Qt::Key_Right) {
    emit SeekForward();
    event->accept();
  } else if (event->modifiers() ==
                 Qt::NoModifier  // No modifier keys currently pressed...
                 // ... and key pressed is something related to text
             &&
             ((event->key() >= Qt::Key_Exclam && event->key() <= Qt::Key_Z) ||
              event->key() == Qt::Key_Backspace ||
              event->key() == Qt::Key_Escape)) {
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

void PlaylistView::RemoveSelected(bool deleting_from_disk) {
  int rows_removed = 0;
  QItemSelection selection(selectionModel()->selection());

  if (selection.isEmpty()) {
    return;
  }

  // Store the last selected row, which is the last in the list
  int last_row = selection.last().top();

  // Sort the selection so we remove the items at the *bottom* first, ensuring
  // we don't have to mess around with changing row numbers
  std::sort(selection.begin(), selection.end(), CompareSelectionRanges);

  for (const QItemSelectionRange& range : selection) {
    if (range.top() < last_row) rows_removed += range.height();

    if (!deleting_from_disk) {
      model()->removeRows(range.top(), range.height(), range.topLeft());
    } else {
      model()->removeRows(range.top(), range.height(), QModelIndex());
    }
  }

  int new_row = last_row - rows_removed;
  // Index of the first column for the row to select
  QModelIndex new_index = model()->index(new_row, 0);

  // Select the new current item, we want always the item after the last
  // selected
  if (new_index.isValid()) {
    // Workaround to update keyboard selected row, if it's not the first row
    // (this also triggers selection)
    if (new_row != 0)
      keyPressEvent(
          new QKeyEvent(QEvent::KeyPress, Qt::Key_Down, Qt::NoModifier));
    // Update visual selection with the entire row
    selectionModel()->select(new_index, QItemSelectionModel::ClearAndSelect |
                                            QItemSelectionModel::Rows);
  } else {
    // We're removing the last item, select the new last row
    selectionModel()->select(
        model()->index(model()->rowCount() - 1, 0),
        QItemSelectionModel::ClearAndSelect | QItemSelectionModel::Rows);
  }
}

QList<int> PlaylistView::GetEditableColumns() {
  QList<int> columns;
  QHeaderView* h = header();
  for (int col = 0; col < h->count(); col++) {
    if (h->isSectionHidden(col)) continue;
    QModelIndex index = model()->index(0, col);
    if (index.flags() & Qt::ItemIsEditable) columns << h->visualIndex(col);
  }
  std::sort(columns.begin(), columns.end());
  return columns;
}

QModelIndex PlaylistView::NextEditableIndex(const QModelIndex& current) {
  QList<int> columns = GetEditableColumns();
  QHeaderView* h = header();
  int index = columns.indexOf(h->visualIndex(current.column()));

  if (index + 1 >= columns.size())
    return model()->index(current.row() + 1, h->logicalIndex(columns.first()));

  return model()->index(current.row(), h->logicalIndex(columns[index + 1]));
}

QModelIndex PlaylistView::PrevEditableIndex(const QModelIndex& current) {
  QList<int> columns = GetEditableColumns();
  QHeaderView* h = header();
  int index = columns.indexOf(h->visualIndex(current.column()));

  if (index - 1 < 0)
    return model()->index(current.row() - 1, h->logicalIndex(columns.last()));

  return model()->index(current.row(), h->logicalIndex(columns[index - 1]));
}

void PlaylistView::closeEditor(QWidget* editor,
                               QAbstractItemDelegate::EndEditHint hint) {
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
  // Check wheather rating section is locked by user or not
  if (!ratings_locked_) {
    QModelIndex index = indexAt(event->pos());
    if (index.isValid() && index.data(Playlist::Role_CanSetRating).toBool()) {
      RatingHoverIn(index, event->pos());
    } else if (rating_delegate_->is_mouse_over()) {
      RatingHoverOut();
    }
  }
  if (!drag_over_) {
    QTreeView::mouseMoveEvent(event);
  }
}

void PlaylistView::leaveEvent(QEvent* e) {
  if (rating_delegate_->is_mouse_over() && !ratings_locked_) {
    RatingHoverOut();
  }
  QTreeView::leaveEvent(e);
}

void PlaylistView::RatingHoverIn(const QModelIndex& index, const QPoint& pos) {
  if (editTriggers() & QAbstractItemView::NoEditTriggers) {
    return;
  }

  const QModelIndex old_index = rating_delegate_->mouse_over_index();
  rating_delegate_->set_mouse_over(index, selectedIndexes(), pos);
  setCursor(Qt::PointingHandCursor);

  update(index);
  update(old_index);
  for (const QModelIndex& index : selectedIndexes()) {
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
  if (editTriggers() & QAbstractItemView::NoEditTriggers) {
    return;
  }

  const QModelIndex old_index = rating_delegate_->mouse_over_index();
  rating_delegate_->set_mouse_out();
  setCursor(QCursor());

  update(old_index);
  for (const QModelIndex& index : selectedIndexes()) {
    if (index.column() == Playlist::Column_Rating) {
      update(index);
    }
  }

  if (old_index.data(Playlist::Role_IsCurrent).toBool()) {
    InvalidateCachedCurrentPixmap();
  }
}

void PlaylistView::mousePressEvent(QMouseEvent* event) {
  if (editTriggers() & QAbstractItemView::NoEditTriggers) {
    QTreeView::mousePressEvent(event);
    return;
  }

  QModelIndex index = indexAt(event->pos());
  if (event->button() == Qt::LeftButton && index.isValid() &&
      index.data(Playlist::Role_CanSetRating).toBool() && !ratings_locked_) {
    // Calculate which star was clicked
    double new_rating =
        RatingPainter::RatingForPos(event->pos(), visualRect(index));

    if (selectedIndexes().contains(index)) {
      // Update all the selected item ratings
      QModelIndexList src_index_list;
      for (const QModelIndex& index : selectedIndexes()) {
        if (index.data(Playlist::Role_CanSetRating).toBool()) {
          QModelIndex src_index = playlist_->proxy()->mapToSource(index);
          src_index_list << src_index;
        }
      }
      playlist_->RateSongs(src_index_list, new_rating);
    } else {
      // Update only this item rating
      playlist_->RateSong(playlist_->proxy()->mapToSource(index), new_rating);
    }
  } else if (event->button() == Qt::XButton1 && index.isValid()) {
    app_->player()->Previous();
  } else if (event->button() == Qt::XButton2 && index.isValid()) {
    app_->player()->Next();
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
  // For 30 seconds after the user clicks on or scrolls the playlist we promise
  // not to automatically scroll the view to keep up with a track change.
  inhibit_autoscroll_ = false;
}

void PlaylistView::MaybeAutoscroll() {
  if (!inhibit_autoscroll_) JumpToCurrentlyPlayingTrack();
}

void PlaylistView::JumpToCurrentlyPlayingTrack() {
  Q_ASSERT(playlist_);

  // Usage of the "Jump to the currently playing track" action shall enable
  // autoscroll
  inhibit_autoscroll_ = false;

  if (playlist_->current_row() == -1) return;

  QModelIndex current = playlist_->proxy()->mapFromSource(
      playlist_->index(playlist_->current_row(), 0));
  if (!current.isValid()) return;

  currently_autoscrolling_ = true;

  // Scroll to the item
  scrollTo(current, QAbstractItemView::PositionAtCenter);

  currently_autoscrolling_ = false;
}

void PlaylistView::JumpToLastPlayedTrack() {
  Q_ASSERT(playlist_);

  if (playlist_->last_played_row() == -1) return;

  QModelIndex last_played = playlist_->proxy()->mapFromSource(
      playlist_->index(playlist_->last_played_row(), 0));
  if (!last_played.isValid()) return;

  // Select last played song
  last_current_item_ = last_played;
  setCurrentIndex(last_current_item_);

  currently_autoscrolling_ = true;

  // Scroll to the item
  scrollTo(last_played, QAbstractItemView::PositionAtCenter);

  currently_autoscrolling_ = false;
}

void PlaylistView::paintEvent(QPaintEvent* event) {
  // Reimplemented to draw the background image.
  // Reimplemented also to draw the drop indicator
  // When the user is dragging some stuff over the playlist paintEvent gets
  // called for the entire viewport every time the user moves the mouse.
  // The drawTree is kinda expensive, so we cache the result and draw from the
  // cache while the user is dragging.  The cached pixmap gets invalidated in
  // dragLeaveEvent, dropEvent and scrollContentsBy.

  // Draw background
  if (background_image_type_ == Custom ||
      background_image_type_ == AlbumCover) {
    if (!background_image_.isNull() || !previous_background_image_.isNull()) {
      QPainter background_painter(viewport());

      // Check if we should recompute the background image
      if (height() != last_height_ || width() != last_width_ ||
          force_background_redraw_) {
        if (background_image_.isNull()) {
          cached_scaled_background_image_ = QPixmap();
        } else {
          cached_scaled_background_image_ =
              QPixmap::fromImage(background_image_.scaled(
                  width(), height(), Qt::KeepAspectRatioByExpanding,
                  Qt::SmoothTransformation));
        }

        last_height_ = height();
        last_width_ = width();
        force_background_redraw_ = false;
      }

      // Actually draw the background image
      if (!cached_scaled_background_image_.isNull()) {
        // Set opactiy only if needed, as this deactivate hardware acceleration
        if (!qFuzzyCompare(previous_background_image_opacity_, qreal(0.0))) {
          background_painter.setOpacity(1.0 -
                                        previous_background_image_opacity_);
        }
        background_painter.drawPixmap(
            (width() - cached_scaled_background_image_.width()) / 2,
            (height() - cached_scaled_background_image_.height()) / 2,
            cached_scaled_background_image_);
      }
      // Draw the previous background image if we're fading
      if (!previous_background_image_.isNull()) {
        background_painter.setOpacity(previous_background_image_opacity_);
        background_painter.drawPixmap(
            (width() - previous_background_image_.width()) / 2,
            (height() - previous_background_image_.height()) / 2,
            previous_background_image_);
      }
    }
  }

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

  const int first_column = header_->logicalIndex(0);

  // Find the y position of the drop indicator
  QModelIndex drop_index = model()->index(drop_indicator_row_, first_column);
  int drop_pos = -1;
  switch (dropIndicatorPosition()) {
    case QAbstractItemView::OnItem:
      return;  // Don't draw anything

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
        drop_pos = 1 +
                   visualRect(model()->index(model()->rowCount() - 1,
                                             first_column)).bottom();
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
  p.drawLine(QPoint(0, drop_pos), QPoint(width(), drop_pos));

  // Now draw the line on top
  QPen line_pen(line_color, kDropIndicatorWidth);
  p.setPen(line_pen);
  p.drawLine(QPoint(0, drop_pos), QPoint(width(), drop_pos));
}

void PlaylistView::dragMoveEvent(QDragMoveEvent* event) {
  QTreeView::dragMoveEvent(event);

  QModelIndex index(indexAt(event->pos()));
  drop_indicator_row_ = index.isValid() ? index.row() : 0;
}

void PlaylistView::dragEnterEvent(QDragEnterEvent* event) {
  QTreeView::dragEnterEvent(event);
  cached_tree_ = QPixmap();
  drag_over_ = true;
}

void PlaylistView::dragLeaveEvent(QDragLeaveEvent* event) {
  QTreeView::dragLeaveEvent(event);
  cached_tree_ = QPixmap();
  drag_over_ = false;
  drop_indicator_row_ = -1;
}

void PlaylistView::dropEvent(QDropEvent* event) {
  QTreeView::dropEvent(event);
  cached_tree_ = QPixmap();
  drop_indicator_row_ = -1;
  drag_over_ = false;
}

void PlaylistView::PlaylistDestroyed() {
  playlist_ = nullptr;
  // We'll get a SetPlaylist() soon
}

void PlaylistView::ReloadSettings() {
  QSettings s;
  s.beginGroup(Playlist::kSettingsGroup);
  glow_enabled_ = s.value("glow_effect", false).toBool();

  if (setting_initial_header_layout_ || upgrading_from_qheaderview_) {
    header_->SetStretchEnabled(s.value("stretch", true).toBool());
    upgrading_from_qheaderview_ = false;
  }

  if (currently_glowing_ && glow_enabled_ && isVisible()) StartGlowing();
  if (!glow_enabled_) StopGlowing();

  if (setting_initial_header_layout_) {
    header_->SetColumnWidth(Playlist::Column_Length, 0.06);
    header_->SetColumnWidth(Playlist::Column_Track, 0.05);
    setting_initial_header_layout_ = false;
  }

  if (upgrading_from_version_ != -1) {
    if (upgrading_from_version_ < 4) {
      header_->SetColumnWidth(Playlist::Column_Source, 0.05);
    }
    upgrading_from_version_ = -1;
  }

  column_alignment_ = s.value("column_alignments").value<ColumnAlignmentMap>();
  if (column_alignment_.isEmpty()) {
    column_alignment_ = DefaultColumnAlignment();
  }

  emit ColumnAlignmentChanged(column_alignment_);

  // Background:
  QVariant q_playlistview_background_type =
      s.value(kSettingBackgroundImageType);
  BackgroundImageType background_type(Default);
  // bg_enabled should also be checked for backward compatibility (in releases
  // <= 1.0, there was just a boolean to activate/deactivate the background)
  QVariant bg_enabled = s.value("bg_enabled");
  if (q_playlistview_background_type.isValid()) {
    background_type = static_cast<BackgroundImageType>(
        q_playlistview_background_type.toInt());
  } else if (bg_enabled.isValid()) {
    if (bg_enabled.toBool()) {
      background_type = Default;
    } else {
      background_type = None;
    }
  }
  QString background_image_filename =
      s.value(kSettingBackgroundImageFilename).toString();
  int blur_radius = s.value("blur_radius", kDefaultBlurRadius).toInt();
  int opacity_level = s.value("opacity_level", kDefaultOpacityLevel).toInt();
  // Check if background properties have changed.
  // We change properties only if they have actually changed, to avoid to call
  // set_background_image when it is not needed, as this will cause the fading
  // animation to start again. This also avoid to do useless
  // "force_background_redraw".
  if (background_initialized_ == false ||
      background_image_filename != background_image_filename_ ||
      background_type != background_image_type_ ||
      blur_radius_ != blur_radius || opacity_level_ != opacity_level) {
    // Store background properties
    background_initialized_ = true;
    background_image_type_ = background_type;
    background_image_filename_ = background_image_filename;
    blur_radius_ = blur_radius;
    opacity_level_ = opacity_level;
    if (background_image_type_ == Custom) {
      set_background_image(QImage(background_image_filename));
    } else if (background_image_type_ == AlbumCover) {
      set_background_image(current_song_cover_art_);
    } else {
      // User changed background image type to something that will not be
      // painted through paintEvent: reset all background images.
      // This avoid to use old (deprecated) images for fading when selecting
      // AlbumCover or Custom background image type later.
      set_background_image(QImage());
      cached_scaled_background_image_ = QPixmap();
      previous_background_image_ = QPixmap();
    }
    setProperty("default_background_enabled",
                background_image_type_ == Default);
    emit BackgroundPropertyChanged();
    force_background_redraw_ = true;
  }

  if (!s.value("click_edit_inline", true).toBool())
    setEditTriggers(editTriggers() & ~QAbstractItemView::SelectedClicked);
  else
    setEditTriggers(editTriggers() | QAbstractItemView::SelectedClicked);
}

void PlaylistView::SaveSettings() {
  if (read_only_settings_) return;

  QSettings s;
  s.beginGroup(Playlist::kSettingsGroup);
  s.setValue("glow_effect", glow_enabled_);
  s.setValue("column_alignments", QVariant::fromValue(column_alignment_));
  s.setValue(kSettingBackgroundImageType, background_image_type_);
}

void PlaylistView::StretchChanged(bool stretch) {
  setHorizontalScrollBarPolicy(stretch ? Qt::ScrollBarAlwaysOff
                                       : Qt::ScrollBarAsNeeded);
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

void PlaylistView::rowsInserted(const QModelIndex& parent, int start, int end) {
  const bool at_end = end == model()->rowCount(parent) - 1;

  QTreeView::rowsInserted(parent, start, end);

  if (at_end) {
    // If the rows were inserted at the end of the playlist then let's scroll
    // the view so the user can see.
    scrollTo(model()->index(start, 0, parent),
             QAbstractItemView::PositionAtTop);
  }
}

ColumnAlignmentMap PlaylistView::DefaultColumnAlignment() {
  ColumnAlignmentMap ret;

  ret[Playlist::Column_Length] = ret[Playlist::Column_Track] =
      ret[Playlist::Column_Disc] = ret[Playlist::Column_Year] =
          ret[Playlist::Column_BPM] = ret[Playlist::Column_Bitrate] =
              ret[Playlist::Column_Samplerate] =
                  ret[Playlist::Column_Filesize] =
                      ret[Playlist::Column_PlayCount] =
                          ret[Playlist::Column_SkipCount] =
                              ret[Playlist::Column_OriginalYear] =
                                  (Qt::AlignRight | Qt::AlignVCenter);
  ret[Playlist::Column_Score] = (Qt::AlignCenter);

  return ret;
}

void PlaylistView::SetColumnAlignment(int section, Qt::Alignment alignment) {
  if (section < 0) return;

  column_alignment_[section] = alignment;
  emit ColumnAlignmentChanged(column_alignment_);
  SaveSettings();
}

Qt::Alignment PlaylistView::column_alignment(int section) const {
  return column_alignment_.value(section, Qt::AlignLeft | Qt::AlignVCenter);
}

void PlaylistView::CopyCurrentSongToClipboard() const {
  // Get the display text of all visible columns.
  QStringList columns;

  for (int i = 0; i < header()->count(); ++i) {
    if (header()->isSectionHidden(i)) {
      continue;
    }

    const QVariant data =
        model()->data(currentIndex().sibling(currentIndex().row(), i));
    if (data.type() == QVariant::String) {
      columns << data.toString();
    }
  }

  // Get the song's URL
  const QUrl url = model()
                       ->data(currentIndex().sibling(currentIndex().row(),
                                                     Playlist::Column_Filename))
                       .toUrl();

  QMimeData* mime_data = new QMimeData;
  mime_data->setUrls(QList<QUrl>() << url);
  mime_data->setText(columns.join(" - "));

  QApplication::clipboard()->setMimeData(mime_data);
}

void PlaylistView::CurrentSongChanged(const Song& song, const QString& uri,
                                      const QImage& song_art) {
  if (current_song_cover_art_ == song_art) return;

  current_song_cover_art_ = song_art;
  if (background_image_type_ == AlbumCover) {
    if (song.art_automatic().isEmpty() && song.art_manual().isEmpty()) {
      set_background_image(QImage());
    } else {
      set_background_image(current_song_cover_art_);
    }
    force_background_redraw_ = true;
    update();
  }
}

void PlaylistView::set_background_image(const QImage& image) {
  // Save previous image, for fading
  previous_background_image_ = cached_scaled_background_image_;

  if (image.isNull() || image.format() == QImage::Format_ARGB32) {
    background_image_ = image;
  } else {
    background_image_ = image.convertToFormat(QImage::Format_ARGB32);
  }

  if (!background_image_.isNull()) {
    // Apply opacity filter
    uchar* bits = background_image_.bits();
    for (int i = 0;
         i < background_image_.height() * background_image_.bytesPerLine();
         i += 4) {
      bits[i + 3] = (opacity_level_ / 100.0) * 255;
    }

    if (blur_radius_ != 0) {
      QImage blurred(background_image_.size(),
                     QImage::Format_ARGB32_Premultiplied);
      blurred.fill(Qt::transparent);
      QPainter blur_painter(&blurred);
      qt_blurImage(&blur_painter, background_image_, blur_radius_, true, false);
      blur_painter.end();

      background_image_ = blurred;
    }
  }

  if (isVisible()) {
    previous_background_image_opacity_ = 1.0;
    fade_animation_->start();
  }
}

void PlaylistView::FadePreviousBackgroundImage(qreal value) {
  previous_background_image_opacity_ = value;
  if (qFuzzyCompare(previous_background_image_opacity_, qreal(0.0))) {
    previous_background_image_ = QPixmap();
    previous_background_image_opacity_ = 0.0;
  }

  update();
}

void PlaylistView::PlayerStopped() {
  CurrentSongChanged(Song(), QString(), QImage());
}

void PlaylistView::focusInEvent(QFocusEvent* event) {
  QTreeView::focusInEvent(event);

  if (event->reason() == Qt::TabFocusReason ||
      event->reason() == Qt::BacktabFocusReason) {
    // If there's a current item but no selection it probably means the list was
    // filtered, and the selected item does not match the filter.  If there's
    // only 1 item in the view it is now impossible to select that item without
    // using the mouse.
    const QModelIndex& current = selectionModel()->currentIndex();
    if (current.isValid() && selectionModel()->selectedIndexes().isEmpty()) {
      QItemSelection new_selection(
          current.sibling(current.row(), 0),
          current.sibling(current.row(),
                          current.model()->columnCount(current.parent()) - 1));
      selectionModel()->select(new_selection, QItemSelectionModel::Select);
    }
  }
}
