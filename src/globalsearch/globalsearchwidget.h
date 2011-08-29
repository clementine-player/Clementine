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

#ifndef GLOBALSEARCHWIDGET_H
#define GLOBALSEARCHWIDGET_H

#include "searchprovider.h"

#include <QWidget>

class GlobalSearch;
class LibraryBackendInterface;
class Ui_GlobalSearchWidget;

class QListView;
class QMimeData;
class QModelIndex;
class QSortFilterProxyModel;
class QStandardItemModel;


class GlobalSearchWidget : public QWidget {
  Q_OBJECT

public:
  GlobalSearchWidget(QWidget *parent = 0);
  ~GlobalSearchWidget();

  static const int kMinVisibleItems;
  static const int kMaxVisibleItems;

  enum Role {
    Role_Result = Qt::UserRole + 1,
    Role_LazyLoadingArt
  };

  void Init(LibraryBackendInterface* library);

  // Called by the delegate
  void LazyLoadArt(const QModelIndex& index);

  // QWidget
  bool eventFilter(QObject* o, QEvent* e);

signals:
  void AddToPlaylist(QMimeData* data);

protected:
  void resizeEvent(QResizeEvent* e);
  void paintEvent(QPaintEvent* e);

private slots:
  void TextEdited(const QString& text);
  void SearchFinished(int id);
  void AddResults(int id, const SearchProvider::ResultList& results);

  void ArtLoaded(int id, const QPixmap& pixmap);

  void TracksLoaded(int id, MimeData* mime_data);

  void AddCurrent();

private:
  void Reset();
  void RepositionPopup();

private:
  Ui_GlobalSearchWidget* ui_;

  GlobalSearch* engine_;
  int last_id_;
  bool clear_model_on_next_result_;

  QMap<int, QModelIndex> art_requests_;

  QStandardItemModel* model_;
  QSortFilterProxyModel* proxy_;
  QListView* view_;
  bool eat_focus_out_;

  QPixmap background_;
  QPixmap background_scaled_;
};

#endif // GLOBALSEARCHWIDGET_H
