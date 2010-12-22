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

#include "playlist.h"

#include <QBasicTimer>
#include <QProxyStyle>
#include <QTreeView>

#include <boost/scoped_ptr.hpp>

class QCleanlooksStyle;

class DynamicPlaylistControls;
class LibraryBackend;
class PlaylistHeader;
class RadioLoadingIndicator;
class RatingItemDelegate;


// This proxy style works around a bug/feature introduced in Qt 4.7's QGtkStyle
// that uses Gtk to paint row backgrounds, ignoring any custom brush or palette
// the caller set in the QStyleOption.  That breaks our currently playing track
// animation, which relies on the background painted by Qt to be transparent.
// This proxy style uses QCleanlooksStyle to paint the affected elements.
class PlaylistProxyStyle : public QProxyStyle {
public:
  PlaylistProxyStyle(QStyle* base);
  void drawControl(ControlElement element, const QStyleOption* option,
                   QPainter* painter, const QWidget* widget) const;
  void drawPrimitive(PrimitiveElement element, const QStyleOption* option,
                     QPainter* painter, const QWidget* widget) const;

private:
  boost::scoped_ptr<QCleanlooksStyle> cleanlooks_;
};


class PlaylistView : public QTreeView {
  Q_OBJECT

 public:
  PlaylistView(QWidget* parent = 0);

  static const char* kSettingsGroup;
  static const int kStateVersion;

  void SetItemDelegates(LibraryBackend* backend);
  void SetPlaylist(Playlist* playlist);
  void RemoveSelected();

  void SetReadOnlySettings(bool read_only) { read_only_settings_ = read_only; }

  Playlist* playlist() const { return playlist_; }

  // QTreeView
  void drawTree(QPainter* painter, const QRegion& region) const;
  void drawRow(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const;
  void keyPressEvent(QKeyEvent* event);
  void setModel(QAbstractItemModel *model);

 public slots:
  void ReloadSettings();
  void StopGlowing();
  void StartGlowing();
  void JumpToCurrentlyPlayingTrack();
  void closeEditor(QWidget* editor, QAbstractItemDelegate::EndEditHint hint);
  void DynamicModeChanged(bool dynamic);

 signals:
  void PlayPauseItem(const QModelIndex& index);
  void RightClicked(const QPoint& global_pos, const QModelIndex& index);

 protected:
  void contextMenuEvent(QContextMenuEvent* e);
  void hideEvent(QHideEvent* event);
  void showEvent(QShowEvent* event);
  void timerEvent(QTimerEvent* event);
  void mouseMoveEvent(QMouseEvent* event);
  void mousePressEvent(QMouseEvent* event);
  void leaveEvent(QEvent*);
  void scrollContentsBy(int dx, int dy);
  void paintEvent(QPaintEvent *event);
  void dragMoveEvent(QDragMoveEvent *event);
  void dragEnterEvent(QDragEnterEvent *event);
  void dragLeaveEvent(QDragLeaveEvent *event);
  void dropEvent(QDropEvent *event);
  void resizeEvent(QResizeEvent* event);
  bool eventFilter(QObject* object, QEvent* event);

 private slots:
  void LoadGeometry();
  void SaveGeometry();
  void GlowIntensityChanged();
  void InhibitAutoscrollTimeout();
  void MaybeAutoscroll();
  void InvalidateCachedCurrentPixmap();
  void PlaylistDestroyed();

  void SaveSettings();
  void StretchChanged(bool stretch);

  void RatingHoverIn(const QModelIndex& index, const QPoint& pos);
  void RatingHoverOut();

 private:
  void ReloadBarPixmaps();
  QList<QPixmap> LoadBarPixmap(const QString& filename);
  void UpdateCachedCurrentRowPixmap(QStyleOptionViewItemV4 option,
                                    const QModelIndex& index);

 private:
  static const int kGlowIntensitySteps;
  static const int kAutoscrollGraceTimeout;
  static const int kDropIndicatorWidth;
  static const int kDropIndicatorGradientWidth;

  QList<int> GetEditableColumns();
  QModelIndex NextEditableIndex(const QModelIndex& current);
  QModelIndex PrevEditableIndex(const QModelIndex& current);

  void RepositionDynamicControls();

  PlaylistProxyStyle* style_;
  Playlist* playlist_;
  PlaylistHeader* header_;
  bool setting_initial_header_layout_;
  bool read_only_settings_;

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

  int row_height_; // Used to invalidate the currenttrack_bar pixmaps
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

  DynamicPlaylistControls* dynamic_controls_;
};

#endif // PLAYLISTVIEW_H
