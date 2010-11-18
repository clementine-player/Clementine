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

#ifndef QUERYWIZARDPLUGIN_H
#define QUERYWIZARDPLUGIN_H

#include "search.h"
#include "wizardplugin.h"

#include <QWizard>

#include <boost/scoped_ptr.hpp>

class Ui_SmartPlaylistQuerySearchPage;
class Ui_SmartPlaylistQuerySortPage;

class QVBoxLayout;

namespace smart_playlists {

class SearchPreview;
class SearchTermWidget;

class QueryWizardPlugin : public WizardPlugin {
  Q_OBJECT

public:
  QueryWizardPlugin(LibraryBackend* library, QObject* parent);
  ~QueryWizardPlugin();

  QString name() const;
  QString description() const;

  int CreatePages(QWizard* wizard);
  GeneratorPtr CreateGenerator() const;

private slots:
  void AddSearchTerm();
  void RemoveSearchTerm();

  void SearchTypeChanged();

  void UpdateTermPreview();
  void UpdateSortPreview();
  void UpdateSortOrder();

private:
  class SearchPage : public QWizardPage {
    friend class QueryWizardPlugin;

  public:
    SearchPage(QWidget* parent = 0);
    bool isComplete() const;

    QVBoxLayout* layout_;
    QList<SearchTermWidget*> terms_;
    SearchTermWidget* new_term_;

    SearchPreview* preview_;

    boost::scoped_ptr<Ui_SmartPlaylistQuerySearchPage> ui_;
  };

  Search MakeSearch() const;

  SearchPage* search_page_;
  boost::scoped_ptr<Ui_SmartPlaylistQuerySortPage> sort_ui_;
};

} // namespace smart_playlists

#endif // QUERYWIZARDPLUGIN_H
