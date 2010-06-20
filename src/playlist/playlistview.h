/* This file is part of Clementine.

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

#include <QTreeView>
#include <QBasicTimer>

class RadioLoadingIndicator;
class LibraryBackend;

class PlaylistView : public QTreeView {
  Q_OBJECT

 public:
  PlaylistView(QWidget* parent = 0);

  void SetItemDelegates(LibraryBackend* backend);
  void SetPlaylist(Playlist* playlist);
  void RemoveSelected();

  // QTreeView
  void drawRow(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const;
  void keyPressEvent(QKeyEvent* event);
  void setModel(QAbstractItemModel *model);

  // QAbstractScrollArea
  void contextMenuEvent(QContextMenuEvent* e);

 public slots:
  void StopGlowing();
  void StartGlowing();
  void JumpToCurrentlyPlayingTrack();
  void closeEditor(QWidget* editor, QAbstractItemDelegate::EndEditHint hint);

 signals:
  void PlayPauseItem(const QModelIndex& index);
  void RightClicked(const QPoint& global_pos, const QModelIndex& index);

 protected:
  void hideEvent(QHideEvent* event);
  void showEvent(QShowEvent* event);
  void timerEvent(QTimerEvent *event);
  void mousePressEvent(QMouseEvent *event);
  void scrollContentsBy(int dx, int dy);
  void paintEvent(QPaintEvent *event);
  void dragMoveEvent(QDragMoveEvent *event);
  void dragEnterEvent(QDragEnterEvent *event);
  void dragLeaveEvent(QDragLeaveEvent *event);
  void dropEvent(QDropEvent *event);

 private slots:
  void LoadGeometry();
  void SaveGeometry();
  void GlowIntensityChanged();
  void InhibitAutoscrollTimeout();
  void MaybeAutoscroll();
  void InvalidateCachedCurrentPixmap();

 private:
  void ReloadBarPixmaps();
  QList<QPixmap> LoadBarPixmap(const QString& filename);
  void UpdateCachedCurrentRowPixmap(QStyleOptionViewItem option,
                                    const QModelIndex& index);

 private:
  static const char* kSettingsGroup;
  static const int kGlowIntensitySteps;
  static const int kAutoscrollGraceTimeout;
  static const int kDropIndicatorWidth;
  static const int kDropIndicatorGradientWidth;

  QList<int> GetEditableColumns();
  QModelIndex NextEditableIndex(const QModelIndex& current);
  QModelIndex PrevEditableIndex(const QModelIndex& current);

  Playlist* playlist_;

  bool glow_enabled_;
  QBasicTimer glow_timer_;
  int glow_intensity_step_;
  QModelIndex last_current_item_;
  QRect last_glow_rect_;

  QTimer* inhibit_autoscroll_timer_;
  bool inhibit_autoscroll_;
  bool currently_autoscrolling_;

  int row_height_; // Used to invalidate the currenttrack_bar pixmaps
  QList<QPixmap> currenttrack_bar_left_;
  QList<QPixmap> currenttrack_bar_mid_;
  QList<QPixmap> currenttrack_bar_right_;
  QPixmap currenttrack_play_;
  QPixmap currenttrack_pause_;

  QPixmap cached_current_row_;
  QRect cached_current_row_rect_;
  int cached_current_row_row_;

  QPixmap cached_tree_;
  int drop_indicator_row_;
};

#endif // PLAYLISTVIEW_H
