/* This file is part of Clementine.
   Copyright 2014, Krzysztof Sobiecki <sobkas@gmail.com>
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

#include "internet/cloudfilesearchprovider.h"
#include "internet/cloudfileservice.h"
#include "internet/internetmodel.h"

CloudFileSearchProvider::CloudFileSearchProvider(
    LibraryBackendInterface* backend, const QString& id, const QIcon& icon,
    CloudFileService* service)
    : LibrarySearchProvider(backend, service->name(), id, icon, true,
                            service->model()->app(), service),
      service_(service) {
  SetHint(CanShowConfig);
}

bool CloudFileSearchProvider::IsLoggedIn() {
  return service_->has_credentials();
}

void CloudFileSearchProvider::ShowConfig() { service_->ShowSettingsDialog(); }

InternetService* CloudFileSearchProvider::internet_service() {
  return service_;
}
