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
class GroupByDialog;
class SearchProviderStatusWidget;
class SuggestionWidget;
class Ui_GlobalSearchView;

class QActionGroup;
class QMimeData;
class QSortFilterProxyModel;
class QStandardItem;
class QStandardItemModel;

class GlobalSearchView : public QWidget {
  Q_OBJECT

 public:
  GlobalSearchView(Application* app, QWidget* parent = nullptr);
  ~GlobalSearchView();

  static const int kSwapModelsTimeoutMsec;
  static const int kMaxSuggestions;
  static const int kUpdateSuggestionsTimeoutMsec;

  // Called by the delegate
  void LazyLoadArt(const QModelIndex& index);

  // QWidget
  void showEvent(QShowEvent* e);
  void hideEvent(QHideEvent* e);

  // QObject
  bool eventFilter(QObject* object, QEvent* event);

 public slots:
  void ReloadSettings();
  void StartSearch(const QString& query);
  void FocusSearchField();
  void OpenSettingsDialog();

signals:
  void AddToPlaylist(QMimeData* data);

 private slots:
  void UpdateSuggestions();

  void SwapModels();
  void TextEdited(const QString& text);
  void AddResults(int id, const SearchProvider::ResultList& results);
  void ArtLoaded(int id, const QPixmap& pixmap);

  void FocusOnFilter(QKeyEvent* event);

  void AddSelectedToPlaylist();
  void LoadSelected();
  void OpenSelectedInNewPlaylist();
  void AddSelectedToPlaylistEnqueue();

  void GroupByClicked(QAction* action);
  void SetGroupBy(const LibraryModel::Grouping& grouping);

 private:
  MimeData* SelectedMimeData();

  bool SearchKeyEvent(QKeyEvent* event);
  bool ResultsContextMenuEvent(QContextMenuEvent* event);

 private:
  Application* app_;
  GlobalSearch* engine_;
  Ui_GlobalSearchView* ui_;
  QScopedPointer<GroupByDialog> group_by_dialog_;

  QMenu* context_menu_;
  QList<QAction*> context_actions_;
  QActionGroup* group_by_actions_;

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
  QTimer* update_suggestions_timer_;

  QList<SearchProviderStatusWidget*> provider_status_widgets_;
  QList<SuggestionWidget*> suggestion_widgets_;

  QIcon search_icon_;
  QIcon warning_icon_;

  bool show_providers_;
  bool show_suggestions_;

  bool show_album_year_;
};

#endif  // GLOBALSEARCHVIEW_H
