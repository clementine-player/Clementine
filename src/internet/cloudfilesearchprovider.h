/* This file is part of Clementine.
   Copyright 2014, David Sansome <me@davidsansome.com>
   
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

#ifndef CLOUDFILESEARCHPROVIDER_H
#define CLOUDFILESEARCHPROVIDER_H

#include "globalsearch/librarysearchprovider.h"

class CloudFileService;

class CloudFileSearchProvider : public LibrarySearchProvider {
 public:
  CloudFileSearchProvider(LibraryBackendInterface* backend,
                          const QString& id,
                          const QIcon& icon,
                          CloudFileService* service);

  virtual bool IsLoggedIn();
  virtual void ShowConfig();
  virtual InternetService* internet_service();

 private:
  CloudFileService* service_;
};

#endif // CLOUDFILESEARCHPROVIDER_H
