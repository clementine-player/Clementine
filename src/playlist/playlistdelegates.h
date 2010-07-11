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

#ifndef PLAYLISTDELEGATES_H
#define PLAYLISTDELEGATES_H

#include "playlist.h"
#include "library/library.h"

#include <QStyledItemDelegate>
#include <QTreeView>
#include <QStringListModel>
#include <QCompleter>

class QueuedItemDelegate : public QStyledItemDelegate {
public:
  QueuedItemDelegate(QObject* parent);
  void paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const;
};

class PlaylistDelegateBase : public QueuedItemDelegate {
  Q_OBJECT
 public:
  PlaylistDelegateBase(QTreeView* view, const QString& suffix = QString());
  void paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const;
  QString displayText(const QVariant& value, const QLocale& locale) const;
  QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const;

  QStyleOptionViewItemV4 Adjusted(const QStyleOptionViewItem& option, const QModelIndex& index) const;

  static const int kMinHeight;

 public slots:
  bool helpEvent(QHelpEvent *event, QAbstractItemView *view,
                 const QStyleOptionViewItem &option, const QModelIndex &index);

 protected:
  QTreeView* view_;
  QString suffix_;
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

class TagCompletionModel : public QStringListModel {
 public:
  TagCompletionModel(LibraryBackend* backend, Playlist::Column column);
};

class TagCompleter : public QCompleter {
 public:
  TagCompleter(LibraryBackend* backend, Playlist::Column column, QLineEdit* editor);
};

class TagCompletionItemDelegate : public PlaylistDelegateBase {
 public:
  TagCompletionItemDelegate(QTreeView* view,LibraryBackend* backend, Playlist::Column column) :
    PlaylistDelegateBase(view), backend_(backend), column_(column) {};

  QWidget* createEditor(QWidget* parent, const QStyleOptionViewItem& option,
      const QModelIndex& index) const;

 private:
  LibraryBackend* backend_;
  Playlist::Column column_;
};

#endif // PLAYLISTDELEGATES_H
