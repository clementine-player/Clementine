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

#ifndef QUERYWIZARDPLUGIN_H
#define QUERYWIZARDPLUGIN_H

#include "wizardplugin.h"

#include <memory>

#include <QWizard>

#include "search.h"

class Ui_SmartPlaylistQuerySearchPage;
class Ui_SmartPlaylistQuerySortPage;

class QVBoxLayout;

namespace smart_playlists {

class SearchPreview;
class SearchTermWidget;

class QueryWizardPlugin : public WizardPlugin {
  Q_OBJECT

 public:
  QueryWizardPlugin(Application* app, LibraryBackend* library, QObject* parent);
  ~QueryWizardPlugin();

  QString type() const { return "Query"; }
  QString name() const;
  QString description() const;
  bool is_dynamic() const { return true; }

  int CreatePages(QWizard* wizard, int finish_page_id);
  void SetGenerator(GeneratorPtr);
  GeneratorPtr CreateGenerator() const;

 private slots:
  void AddSearchTerm();
  void RemoveSearchTerm();

  void SearchTypeChanged();

  void UpdateTermPreview();
  void UpdateSortPreview();
  void UpdateSortOrder();

  void MoveTermListToBottom(int min, int max);

 private:
  class SearchPage;
  class SortPage;

  Search MakeSearch() const;

  SearchPage* search_page_;
  std::unique_ptr<Ui_SmartPlaylistQuerySortPage> sort_ui_;

  int previous_scrollarea_max_;
};

}  // namespace smart_playlists

#endif  // QUERYWIZARDPLUGIN_H
