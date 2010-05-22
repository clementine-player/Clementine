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

#include "plsparser.h"

#include <QTemporaryFile>
#include <QSettings>
#include <QtDebug>

PLSParser::PLSParser(QObject* parent)
  : ParserBase(parent)
{
}

SongList PLSParser::Load(QIODevice *device, const QDir &dir) const {
  QTemporaryFile temp_file;
  temp_file.open();
  temp_file.write(device->readAll());
  temp_file.flush();

  QSettings s(temp_file.fileName(), QSettings::IniFormat);

  SongList ret;
  // Use the first group, probably "playlist" but it doesn't matter
  if (s.childGroups().isEmpty())
    return ret;
  s.beginGroup(s.childGroups()[0]);

  // We try not to rely on NumberOfEntries (it might not be present), so go
  // through each key in the file and look at ones that start with "File"
  foreach (const QString& key, s.childKeys()) {
    if (!key.toLower().startsWith("file"))
      continue;

    bool ok = false;
    int n = key.mid(4).toInt(&ok); // 4 == "file".length

    if (!ok)
      continue;

    QString filename = s.value(key).toString();
    QString title = s.value("Title" + QString::number(n)).toString();
    int length = s.value("Length" + QString::number(n)).toInt();

    Song song;
    song.set_title(title);
    song.set_length(length);

    if (!ParseTrackLocation(filename, dir, &song)) {
      qWarning() << "Failed to parse location: " << filename;
    } else {
      ret << song;
    }
  }

  return ret;
}

void PLSParser::Save(const SongList &songs, QIODevice *device, const QDir &dir) const {

}
