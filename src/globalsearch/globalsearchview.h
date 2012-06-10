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
class GlobalSearchModel;
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
  GlobalSearchModel* front_model_;
  GlobalSearchModel* back_model_;
  GlobalSearchModel* current_model_;

  QSortFilterProxyModel* front_proxy_;
  QSortFilterProxyModel* back_proxy_;
  QSortFilterProxyModel* current_proxy_;

  QMap<int, QModelIndex> art_requests_;

  QTimer* swap_models_timer_;

  QList<SearchProviderStatusWidget*> provider_status_widgets_;
};

#endif // GLOBALSEARCHVIEW_H
