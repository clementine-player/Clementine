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

#include "utilities.h"

#include <QCoreApplication>
#include <QStringList>

namespace Utilities {

static QString tr(const char* str) {
  return QCoreApplication::translate("", str);
}

QString PrettyTime(int seconds) {
  // last.fm sometimes gets the track length wrong, so you end up with
  // negative times.
  seconds = qAbs(seconds);

  int hours = seconds / (60*60);
  int minutes = (seconds / 60) % 60;
  seconds %= 60;

  QString ret;
  if (hours)
    ret.sprintf("%d:%02d:%02d", hours, minutes, seconds);
  else
    ret.sprintf("%d:%02d", minutes, seconds);

  return ret;
}

QString WordyTime(quint64 seconds) {
  quint64 days = seconds / (60*60*24);

  // TODO: Make the plural rules translatable
  QStringList parts;

  if (days)
    parts << (days == 1 ? tr("1 day") : tr("%1 days").arg(days));
  parts << PrettyTime(seconds - days*60*60*24);

  return parts.join(" ");
}

QString PrettySize(quint64 bytes) {
  QString ret;

  if (bytes > 0) {
    if (bytes <= 1000)
      ret = QString::number(bytes) + " bytes";
    else if (bytes <= 1000*1000)
      ret.sprintf("%.1f KB", float(bytes) / 1000);
    else if (bytes <= 1000*1000*1000)
      ret.sprintf("%.1f MB", float(bytes) / (1000*1000));
    else
      ret.sprintf("%.1f GB", float(bytes) / (1000*1000*1000));
  }
  return ret;
}

} // namespace
