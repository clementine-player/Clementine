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

#ifndef NETWORKREMOTESETTINGSPAGE_H
#define NETWORKREMOTESETTINGSPAGE_H

#include <QHostAddress>
#include <QList>
#include <QStringList>

#include "settingspage.h"

class QListWidgetItem;
class Ui_NetworkRemoteSettingsPage;

class NetworkRemoteSettingsPage : public SettingsPage {
  Q_OBJECT

 public:
  NetworkRemoteSettingsPage(SettingsDialog* dialog);
  ~NetworkRemoteSettingsPage();

  void Load();
  void Save();

 protected:
  bool eventFilter(QObject* object, QEvent* event);

 private slots:
  void Options();
  void AddNetwork();
  void RemoveNetwork();

 private:
  // Fills the listen network list with the networks this computer is on,
  // checking the ones in |chosen|. Chosen networks that it isn't on now -
  // at home while it's at work, say - are kept, so saving doesn't quietly
  // drop them.
  void PopulateListenNetworks(const QStringList& chosen);
  // Adds a row for |network|, which was added by hand rather than being an
  // interface's, given this computer's |addresses| in it.
  QListWidgetItem* AddChosenNetwork(const QString& network,
                                    const QList<QHostAddress>& addresses);
  // Warns about chosen listen networks that only allowing local clients
  // leaves nobody able to connect through.
  void UpdateListenWarning();
  void UpdateRemoveButton();

  static const char* kPlayStoreUrl;

  Ui_NetworkRemoteSettingsPage* ui_;
};

#endif  // NETWORKREMOTESETTINGSPAGE_H
