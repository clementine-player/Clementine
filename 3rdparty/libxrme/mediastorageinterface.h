/* This file is part of the XMPP Remote Media Extension.
   Copyright 2010, David Sansome <me@davidsansome.com>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public License
   as published by the Free Software Foundation; either version 2
   of the License, or (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef LIBXRME_MEDIASTORAGEINTERFACE_H
#define LIBXRME_MEDIASTORAGEINTERFACE_H

#include <xrme/common.h>

#include <QImage>
#include <QScopedPointer>

namespace xrme {

class MediaStorageHandler;

class MediaStorageInterface {
public:
  MediaStorageInterface();
  virtual ~MediaStorageInterface();

  virtual QStringList GetArtists() const = 0;

private:
  Q_DISABLE_COPY(MediaStorageInterface);
  friend class MediaStorageHandler;

  void Attach(MediaStorageHandler* handler);

  struct Private;
  QScopedPointer<Private> d;
};

} // namespace xrme

#endif // LIBXRME_MEDIASTORAGEINTERFACE_H
