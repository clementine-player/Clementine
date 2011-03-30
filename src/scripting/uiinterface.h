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

#ifndef UIINTERFACE_H
#define UIINTERFACE_H

#include <QMap>
#include <QObject>
#include <QPair>

class QAction;
class QMenu;

class UIInterface : public QObject {
  Q_OBJECT

public:
  UIInterface(QObject* parent = 0);

  // Called from C++
  void RegisterActionLocation(const QString& id, QMenu* menu, QAction* before);

  // Called from scripts
  void AddAction(const QString& id, QAction* action);

private slots:
  void MenuDestroyed();
  void MenuActionDestroyed();

  void ActionDestroyed();

private:
  struct Location {
    Location() : menu_(NULL), before_(NULL) {}
    Location(QMenu* menu, QAction* before) : menu_(menu), before_(before) {}

    QMenu* menu_;
    QAction* before_;
  };

  typedef QPair<QString, QAction*> IdAndAction;

  void DoAddAction(const QString& id, QAction* action);

private:
  QList<IdAndAction> pending_actions_;
  QMap<QString, Location> locations_;
};

#endif // UIINTERFACE_H
