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

#include <QStyledItemDelegate>
#include <QTreeView>

class RadioLoadingIndicator;

class PlaylistDelegateBase : public QStyledItemDelegate {
  Q_OBJECT
 public:
  PlaylistDelegateBase(QTreeView* view);
  void paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const;
  QString displayText(const QVariant& value, const QLocale& locale) const;

  QStyleOptionViewItemV4 Adjusted(const QStyleOptionViewItem& option, const QModelIndex& index) const;

 protected:
  QTreeView* view_;
};

class LengthItemDelegate : public PlaylistDelegateBase {
 public:
  LengthItemDelegate(QTreeView* view) : PlaylistDelegateBase(view) {}
  QString displayText(const QVariant& value, const QLocale& locale) const;
};

class SizeItemDelegate : public PlaylistDelegateBase {
 public:
  SizeItemDelegate(QTreeView* view) : PlaylistDelegateBase(view) {}
  QString displayText(const QVariant& value, const QLocale& locale) const;
};

class DateItemDelegate : public PlaylistDelegateBase {
 public:
  DateItemDelegate(QTreeView* view) : PlaylistDelegateBase(view) {}
  QString displayText(const QVariant& value, const QLocale& locale) const;
};

class FileTypeItemDelegate : public PlaylistDelegateBase {
 public:
  FileTypeItemDelegate(QTreeView* view) : PlaylistDelegateBase(view) {}
  QString displayText(const QVariant& value, const QLocale& locale) const;
};

class TextItemDelegate : public PlaylistDelegateBase {
 public:
  TextItemDelegate(QTreeView* view) : PlaylistDelegateBase(view) {};
  QWidget* createEditor(QWidget* parent, const QStyleOptionViewItem& option,
      const QModelIndex& index) const;
};

class TagCompletionItemDelegate : public PlaylistDelegateBase {
 public:
  TagCompletionItemDelegate(QTreeView* view, Library* library, Playlist::Column column) :
    PlaylistDelegateBase(view), library_(library), column_(column) {};

  QWidget* createEditor(QWidget* parent, const QStyleOptionViewItem& option,
      const QModelIndex& index) const;

 private:
  Library* library_;
  Playlist::Column column_;
};


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

 private slots:
  void LoadGeometry();
  void SaveGeometry();
  void GlowIntensityChanged();

 private:
  void ReloadBarPixmaps();
  QList<QPixmap> LoadBarPixmap(const QString& filename);

 private:
  static const char* kSettingsGroup;
  static const int kGlowIntensitySteps;

  QList<int> GetEditableColumns();
  QModelIndex NextEditableIndex(const QModelIndex& current);
  QModelIndex PrevEditableIndex(const QModelIndex& current);

  bool glow_enabled_;
  QTimer* glow_timer_;
  int glow_intensity_step_;
  QModelIndex last_current_item_;
  QRect last_glow_rect_;

  int row_height_; // Used to invalidate the currenttrack_bar pixmaps
  QList<QPixmap> currenttrack_bar_left_;
  QList<QPixmap> currenttrack_bar_mid_;
  QList<QPixmap> currenttrack_bar_right_;
  QPixmap currenttrack_play_;
  QPixmap currenttrack_pause_;
};

#endif // PLAYLISTVIEW_H
