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

#ifndef GLOBALSHORTCUTS_H
#define GLOBALSHORTCUTS_H

#include <QObject>

class GlobalShortcuts : public QObject {
  Q_OBJECT

public:
  GlobalShortcuts(QObject* parent = 0);

  static const char* kGsdService;
  static const char* kGsdPath;
  static const char* kGsdInterface;

  void MacMediaKeyPressed(const QString& key);

  bool IsGsdAvailable() const;

signals:
  void PlayPause();
  void Stop();
  void Next();
  void Previous();

private:
  void Init();
  bool RegisterGnome();
  bool RegisterQxt();

private slots:
  void GnomeMediaKeyPressed(const QString& application, const QString& key);
};

#endif
