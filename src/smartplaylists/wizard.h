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

#ifndef SMARTPLAYLISTWIZARD_H
#define SMARTPLAYLISTWIZARD_H

#include "search.h"

#include <QWizard>

class LibraryBackend;
class Ui_SmartPlaylistWizard;

class QComboBox;
class QVBoxLayout;

namespace smart_playlists {

class SearchPreview;
class SearchTermWidget;

class Wizard : public QWizard {
  Q_OBJECT

public:
  Wizard(LibraryBackend* library, QWidget* parent);
  ~Wizard();

  class SearchPage : public QWizardPage {
    friend class Wizard;
  public:
    SearchPage(QWidget* parent = 0);
    bool isComplete() const;

    QVBoxLayout* layout_;
    QComboBox* type_;
    QList<SearchTermWidget*> terms_;
    SearchTermWidget* new_term_;

    SearchPreview* preview_;
  };

private slots:
  void AddSearchTerm();
  void RemoveSearchTerm();

  void SearchTypeChanged();

  void UpdateTermPreview();
  void UpdateSortPreview();
  void UpdateSortOrder();

private:
  Search MakeSearch() const;

private:
  Ui_SmartPlaylistWizard* ui_;
  LibraryBackend* library_;
};

} // namespace

#endif // SMARTPLAYLISTWIZARD_H
