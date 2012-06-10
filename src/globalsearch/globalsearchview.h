/* This file is part of Clementine.
   Copyright 2012, David Sansome <me@davidsansome.com>
   
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

#ifndef GLOBALSEARCHVIEW_H
#define GLOBALSEARCHVIEW_H

#include "searchprovider.h"
#include "library/librarymodel.h"
#include "ui/settingsdialog.h"

#include <QWidget>

class Application;
class SearchProviderStatusWidget;
class Ui_GlobalSearchView;

class QMimeData;
class QSortFilterProxyModel;
class QStandardItem;
class QStandardItemModel;

class GlobalSearchView : public QWidget {
  Q_OBJECT
  
public:
  GlobalSearchView(Application* app, QWidget* parent = 0);
  ~GlobalSearchView();

  static const int kSwapModelsTimeoutMsec;

  enum Role {
    Role_Result = LibraryModel::LastRole,
    Role_LazyLoadingArt,
    Role_ProviderIndex,

    LastRole
  };

  struct ContainerKey {
    int provider_index_;
    QString group_[3];
  };

  // Called by the delegate
  void LazyLoadArt(const QModelIndex& index);

public slots:
  void StartSearch(const QString& query);

signals:
  void AddToPlaylist(QMimeData* data);
  void OpenSettingsAtPage(SettingsDialog::Page page);

private slots:
  void ReloadSettings();

  void SwapModels();
  void TextEdited(const QString& text);
  void AddResults(int id, const SearchProvider::ResultList& results);

  void ArtLoaded(int id, const QPixmap& pixmap);

private:
  MimeData* LoadSelectedTracks();
  QStandardItem* BuildContainers(const Song& metadata, QStandardItem* parent,
                                 ContainerKey* key, int level = 0);

  void GetChildResults(const QStandardItem* item,
                       SearchProvider::ResultList* results,
                       QSet<const QStandardItem*>* visited) const;
  
private:
  Application* app_;
  GlobalSearch* engine_;
  Ui_GlobalSearchView* ui_;

  int last_search_id_;

  // Like graphics APIs have a front buffer and a back buffer, there's a front
  // model and a back model - the front model is the one that's shown in the
  // UI and the back model is the one that lies in wait.  current_model_ will
  // point to either the front or the back model.
  QStandardItemModel* front_model_;
  QStandardItemModel* back_model_;
  QStandardItemModel* current_model_;

  QSortFilterProxyModel* front_proxy_;
  QSortFilterProxyModel* back_proxy_;
  QSortFilterProxyModel* current_proxy_;

  QTimer* swap_models_timer_;

  LibraryModel::Grouping group_by_;

  QMap<SearchProvider*, int> provider_sort_indices_;
  int next_provider_sort_index_;
  QMap<ContainerKey, QStandardItem*> containers_;

  QMap<int, QAction*> track_requests_;
  QMap<int, QModelIndex> art_requests_;

  QIcon artist_icon_;
  QIcon album_icon_;
  QPixmap no_cover_icon_;

  QList<SearchProviderStatusWidget*> provider_status_widgets_;
};

inline uint qHash(const GlobalSearchView::ContainerKey& key) {
  return qHash(key.provider_index_)
       ^ qHash(key.group_[0])
       ^ qHash(key.group_[1])
       ^ qHash(key.group_[2]);
}

inline bool operator <(const GlobalSearchView::ContainerKey& left,
                       const GlobalSearchView::ContainerKey& right) {
  #define CMP(field) \
    if (left.field < right.field) return true; \
    if (left.field > right.field) return false

  CMP(provider_index_);
  CMP(group_[0]);
  CMP(group_[1]);
  CMP(group_[2]);
  return false;

  #undef CMP
}

#endif // GLOBALSEARCHVIEW_H
