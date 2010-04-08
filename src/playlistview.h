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
#include "library.h"

#include <QTreeView>

class RadioLoadingIndicator;

class PlaylistView : public QTreeView {
  Q_OBJECT

 public:
  PlaylistView(QWidget* parent = 0);

  void setItemDelegates(Library* library);
  void RemoveSelected();

  // QTreeView
  void setModel(QAbstractItemModel *model);
  void drawRow(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const;
  void keyPressEvent(QKeyEvent* event);

  // QAbstractScrollArea
  void contextMenuEvent(QContextMenuEvent* e);

 public slots:
  void StopGlowing();
  void StartGlowing();
  void closeEditor(QWidget* editor, QAbstractItemDelegate::EndEditHint hint);

 signals:
  void PlayPauseItem(const QModelIndex& index);
  void RightClicked(const QPoint& global_pos, const QModelIndex& index);

 protected:
  void hideEvent(QHideEvent* event);
  void showEvent(QShowEvent* event);
  void mousePressEvent(QMouseEvent *event);
  void scrollContentsBy(int dx, int dy);

 protected slots:
  void dataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight);

 private slots:
  void LoadGeometry();
  void SaveGeometry();
  void GlowIntensityChanged();
  void InhibitAutoscrollTimeout();

 private:
  void ReloadBarPixmaps();
  QList<QPixmap> LoadBarPixmap(const QString& filename);
  void MaybeAutoscroll();

 private:
  static const char* kSettingsGroup;
  static const int kGlowIntensitySteps;
  static const int kAutoscrollGraceTimeout;

  QList<int> GetEditableColumns();
  QModelIndex NextEditableIndex(const QModelIndex& current);
  QModelIndex PrevEditableIndex(const QModelIndex& current);

  bool glow_enabled_;
  QTimer* glow_timer_;
  int glow_intensity_step_;
  QModelIndex last_current_item_;
  QRect last_glow_rect_;

  QTimer* inhibit_autoscroll_timer_;
  bool inhibit_autoscroll_;

  int row_height_; // Used to invalidate the currenttrack_bar pixmaps
  QList<QPixmap> currenttrack_bar_left_;
  QList<QPixmap> currenttrack_bar_mid_;
  QList<QPixmap> currenttrack_bar_right_;
  QPixmap currenttrack_play_;
  QPixmap currenttrack_pause_;
};

#endif // PLAYLISTVIEW_H
