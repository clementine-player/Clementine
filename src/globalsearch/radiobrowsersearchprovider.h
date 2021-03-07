/* This file is part of Clementine.
   Copyright 2021, Fabio Bas <ctrlaltca@gmail.com>

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

#ifndef RADIOBROWSERSEARCHPROVIDER_H
#define RADIOBROWSERSEARCHPROVIDER_H

#include "internet/radiobrowser/radiobrowserservice.h"
#include "searchprovider.h"

class RadioBrowserSearchProvider : public SearchProvider {
  Q_OBJECT

 public:
  RadioBrowserSearchProvider(Application* app, RadioBrowserService* service,
                             QObject* parent = nullptr);
  void SearchAsync(int id, const QString& query) override;
  //  void ShowConfig() override;
  InternetService* internet_service() override { return service_; }

 public slots:
  void SearchFinishedSlot(int search_id,
                          RadioBrowserService::StreamList streams);

 private:
  RadioBrowserService* service_;
};

#endif  // RADIOBROWSERSEARCHPROVIDER_H
