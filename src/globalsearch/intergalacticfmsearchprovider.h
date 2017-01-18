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

#ifndef INTERGALACTICFMSEARCHPROVIDER_H
#define INTERGALACTICFMSEARCHPROVIDER_H

#include "internet/intergalacticfm/intergalacticfmservice.h"
#include "simplesearchprovider.h"

class IntergalacticFMSearchProvider : public SimpleSearchProvider {
 public:
  IntergalacticFMSearchProvider(IntergalacticFMServiceBase* service,
                                Application* app, QObject* parent);
  // SearchProvider
  InternetService* internet_service() override { return service_; }

  void LoadArtAsync(int id, const Result& result) override;

 protected:
  void RecreateItems() override;

 private:
  IntergalacticFMServiceBase* service_;
  QImage icon_;
};

#endif  // INTERGALACTICFMSEARCHPROVIDER_H
