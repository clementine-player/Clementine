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

#ifndef DEVICEVIEW_H
#define DEVICEVIEW_H

#include "core/song.h"
#include "library/libraryview.h"
#include "widgets/autoexpandingtreeview.h"

class QAction;
class QMenu;
class QSortFilterProxyModel;

class DeviceManager;
class DeviceProperties;
class LibraryModel;
class MergedProxyModel;

class DeviceItemDelegate : public LibraryItemDelegate {
public:
  DeviceItemDelegate(QObject* parent);

  static const int kIconPadding;

  void paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const;
};


class DeviceView : public AutoExpandingTreeView {
  Q_OBJECT

public:
  DeviceView(QWidget* parent = 0);
  ~DeviceView();

  void SetDeviceManager(DeviceManager* manager);
  void SetLibrary(LibraryModel* library);

signals:
  void Load(const SongList& songs);
  void AddToPlaylist(const SongList& songs);
  void DoubleClicked(const SongList& songs);

protected:
  void contextMenuEvent(QContextMenuEvent *);
  void mouseDoubleClickEvent(QMouseEvent *event);

private slots:
  // Device menu actions
  void Connect();
  void Disconnect();
  void Forget();
  void Properties();

  // Library menu actions
  void Load();
  void AddToPlaylist();
  void Organise();
  void Delete();

  void DeviceDisconnected(int row);

private:
  QModelIndex MapToDevice(const QModelIndex& merged_model_index) const;
  QModelIndex MapToLibrary(const QModelIndex& merged_model_index) const;
  SongList GetSelectedSongs() const;

private:
  DeviceManager* manager_;
  LibraryModel* library_;
  MergedProxyModel* merged_model_;
  QSortFilterProxyModel* sort_model_;

  boost::scoped_ptr<DeviceProperties> properties_dialog_;
  boost::scoped_ptr<OrganiseDialog> organise_dialog_;

  QMenu* device_menu_;
  QAction* connect_action_;
  QAction* disconnect_action_;
  QAction* forget_action_;
  QAction* properties_action_;

  QMenu* library_menu_;
  QAction* load_action_;
  QAction* add_to_playlist_action_;
  QAction* organise_action_;
  QAction* delete_action_;

  QModelIndex menu_index_;
};

#endif // DEVICEVIEW_H
