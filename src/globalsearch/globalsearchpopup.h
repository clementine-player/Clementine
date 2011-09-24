/* This file is part of Clementine.
   Copyright 2011, David Sansome <me@davidsansome.com>

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

#ifndef GLOBALSEARCHPOPUP_H
#define GLOBALSEARCHPOPUP_H

#include <boost/scoped_ptr.hpp>

#include <QDialog>

#include "ui_globalsearchpopup.h"

class LibraryBackendInterface;
class Player;
class ProcessSerialNumber;

class GlobalSearchPopup : public QWidget {
  Q_OBJECT
 public:
  explicit GlobalSearchPopup(QWidget* parent = 0);

  void Init(GlobalSearch* engine, Player* player);

  // QWidget
  void setFocus(Qt::FocusReason reason = Qt::PopupFocusReason);

 protected:
  void showEvent(QShowEvent* e);

 protected slots:
  void hide();

 private:
  void StorePreviousProcess();
  void ActivatePreviousProcess();

 signals:
  void AddToPlaylist(QMimeData*);
  void Closed();

 private:
  boost::scoped_ptr<Ui_GlobalSearchPopup> ui_;
  ProcessSerialNumber* mac_psn_;
};

#endif  // GLOBALSEARCHPOPUP_H
