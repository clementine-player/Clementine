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

#include <QWizard>

class LibraryBackend;
class SmartPlaylistSearchTermWidget;
class Ui_SmartPlaylistWizard;

class QVBoxLayout;

class SmartPlaylistWizard : public QWizard {
  Q_OBJECT

public:
  SmartPlaylistWizard(LibraryBackend* library, QWidget* parent);
  ~SmartPlaylistWizard();

private slots:
  void AddSearchTerm();

private:
  Ui_SmartPlaylistWizard* ui_;
  LibraryBackend* library_;

  QVBoxLayout* search_term_layout_;
  QList<SmartPlaylistSearchTermWidget*> search_terms_;
  SmartPlaylistSearchTermWidget* new_search_term_;
};

#endif // SMARTPLAYLISTWIZARD_H
