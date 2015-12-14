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

#ifndef PLAYLISTVIEW_H
#define PLAYLISTVIEW_H

#include <memory>

#include <QBasicTimer>
#include <QProxyStyle>
#include <QTreeView>

#include "playlist.h"

class QCommonStyle;

class Application;
class DynamicPlaylistControls;
class LibraryBackend;
class PlaylistHeader;
class RadioLoadingIndicator;
class RatingItemDelegate;
class QTimeLine;

// This proxy style works around a bug/feature introduced in Qt 4.7's QGtkStyle
// that uses Gtk to paint row backgrounds, ignoring any custom brush or palette
// the caller set in the QStyleOption.  That breaks our currently playing track
// animation, which relies on the background painted by Qt to be transparent.
// This proxy style uses QCommonStyle to paint the affected elements.
// This class is used by the global search view as well.
class PlaylistProxyStyle : public QProxyStyle {
 public:
  PlaylistProxyStyle(QStyle* base);
  void drawControl(ControlElement element, const QStyleOption* option,
                   QPainter* painter, const QWidget* widget) const;
  void drawPrimitive(PrimitiveElement element, const QStyleOption* option,
                     QPainter* painter, const QWidget* widget) const;

 private:
  std::unique_ptr<QCommonStyle> common_style_;
};

class PlaylistView : public QTreeView {
  Q_OBJECT
 public:
  enum BackgroundImageType { Default, None, Custom, AlbumCover };

  PlaylistView(QWidget* parent = nullptr);

  static const int kStateVersion;
  // Constants for settings: are persistent, values should not be changed
  static const char* kSettingBackgroundImageType;
  static const char* kSettingBackgroundImageFilename;

  static const int kDefaultBlurRadius;
  static const int kDefaultOpacityLevel;

  static ColumnAlignmentMap DefaultColumnAlignment();

  void SetApplication(Application* app);
  void SetItemDelegates(LibraryBackend* backend);
  void SetPlaylist(Playlist* playlist);
  void RemoveSelected();

  void SetReadOnlySettings(bool read_only) { read_only_settings_ = read_only; }

  Playlist* playlist() const { return playlist_; }
  BackgroundImageType background_image_type() const {
    return background_image_type_;
  }
  Qt::Alignment column_alignment(int section) const;

  // QTreeView
  void drawTree(QPainter* painter, const QRegion& region) const;
  void drawRow(QPainter* painter, const QStyleOptionViewItem& option,
               const QModelIndex& index) const;
  void setModel(QAbstractItemModel* model);

 public slots:
  void ReloadSettings();
  void StopGlowing();
  void StartGlowing();
  void JumpToCurrentlyPlayingTrack();
  void JumpToLastPlayedTrack();
  void closeEditor(QWidget* editor, QAbstractItemDelegate::EndEditHint hint);
  void DynamicModeChanged(bool dynamic);
  void SetColumnAlignment(int section, Qt::Alignment alignment);

  void CopyCurrentSongToClipboard() const;
  void CurrentSongChanged(const Song& new_song, const QString& uri,
                          const QImage& cover_art);
  void PlayerStopped();

signals:
  void PlayItem(const QModelIndex& index);
  void PlayPause();
  void RightClicked(const QPoint& global_pos, const QModelIndex& index);
  void SeekForward();
  void SeekBackward();
  void FocusOnFilterSignal(QKeyEvent* event);
  void BackgroundPropertyChanged();
  void ColumnAlignmentChanged(const ColumnAlignmentMap& alignment);

 protected:
  // QWidget
  void keyPressEvent(QKeyEvent* event);
  void contextMenuEvent(QContextMenuEvent* e);
  void hideEvent(QHideEvent* event);
  void showEvent(QShowEvent* event);
  void timerEvent(QTimerEvent* event);
  void mouseMoveEvent(QMouseEvent* event);
  void mousePressEvent(QMouseEvent* event);
  void leaveEvent(QEvent*);
  void paintEvent(QPaintEvent* event);
  void dragMoveEvent(QDragMoveEvent* event);
  void dragEnterEvent(QDragEnterEvent* event);
  void dragLeaveEvent(QDragLeaveEvent* event);
  void dropEvent(QDropEvent* event);
  void resizeEvent(QResizeEvent* event);
  bool eventFilter(QObject* object, QEvent* event);
  void focusInEvent(QFocusEvent* event);

  // QAbstractScrollArea
  void scrollContentsBy(int dx, int dy);

  // QAbstractItemView
  void rowsInserted(const QModelIndex& parent, int start, int end);

 private slots:
  void LoadGeometry();
  void LoadRatingLockStatus();
  void SaveGeometry();
  void SetRatingLockStatus(bool state);
  void GlowIntensityChanged();
  void InhibitAutoscrollTimeout();
  void MaybeAutoscroll();
  void InvalidateCachedCurrentPixmap();
  void PlaylistDestroyed();

  void SaveSettings();
  void StretchChanged(bool stretch);

  void RatingHoverIn(const QModelIndex& index, const QPoint& pos);
  void RatingHoverOut();

  void FadePreviousBackgroundImage(qreal value);

 private:
  void ReloadBarPixmaps();
  QList<QPixmap> LoadBarPixmap(const QString& filename);
  void UpdateCachedCurrentRowPixmap(QStyleOptionViewItemV4 option,
                                    const QModelIndex& index);

  void set_background_image_type(BackgroundImageType bg) {
    background_image_type_ = bg;
    emit BackgroundPropertyChanged();
  }
  // Save image as the background_image_ after applying some modifications
  // (opacity, ...).
  // Should be used instead of modifying background_image_ directly
  void set_background_image(const QImage& image);

 private:
  static const int kGlowIntensitySteps;
  static const int kAutoscrollGraceTimeout;
  static const int kDropIndicatorWidth;
  static const int kDropIndicatorGradientWidth;

  QList<int> GetEditableColumns();
  QModelIndex NextEditableIndex(const QModelIndex& current);
  QModelIndex PrevEditableIndex(const QModelIndex& current);

  void RepositionDynamicControls();

  Application* app_;
  PlaylistProxyStyle* style_;
  Playlist* playlist_;
  PlaylistHeader* header_;
  bool setting_initial_header_layout_;
  bool upgrading_from_qheaderview_;
  bool read_only_settings_;
  int upgrading_from_version_;

  BackgroundImageType background_image_type_;
  // Stores the background image to be displayed. As we want this image to be
  // particular (in terms of format, opacity), you should probably use
  // set_background_image_type instead of modifying background_image_ directly
  QImage background_image_;
  int blur_radius_;
  int opacity_level_;
  // Used if background image is a filemane
  QString background_image_filename_;
  QImage current_song_cover_art_;
  QPixmap cached_scaled_background_image_;

  // For fading when image change
  QPixmap previous_background_image_;
  qreal previous_background_image_opacity_;
  QTimeLine* fade_animation_;

  // To know if we should redraw the background or not
  int last_height_;
  int last_width_;
  bool force_background_redraw_;

  bool glow_enabled_;
  bool currently_glowing_;
  QBasicTimer glow_timer_;
  int glow_intensity_step_;
  QModelIndex last_current_item_;
  QRect last_glow_rect_;

  RatingItemDelegate* rating_delegate_;

  QTimer* inhibit_autoscroll_timer_;
  bool inhibit_autoscroll_;
  bool currently_autoscrolling_;

  int row_height_;  // Used to invalidate the currenttrack_bar pixmaps
  QList<QPixmap> currenttrack_bar_left_;
  QList<QPixmap> currenttrack_bar_mid_;
  QList<QPixmap> currenttrack_bar_right_;
  QPixmap currenttrack_play_;
  QPixmap currenttrack_pause_;

  QRegion current_paint_region_;
  QPixmap cached_current_row_;
  QRect cached_current_row_rect_;
  int cached_current_row_row_;

  QPixmap cached_tree_;
  int drop_indicator_row_;
  bool drag_over_;

  bool ratings_locked_; // To store Ratings section lock status

  DynamicPlaylistControls* dynamic_controls_;

  ColumnAlignmentMap column_alignment_;
};

#endif  // PLAYLISTVIEW_H
