/*
   Copyright 2010 Last.fm Ltd.
      - Primarily authored by Jono Cole, Michael Coffey, and William Viana

   This file is part of liblastfm.

   liblastfm is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   liblastfm is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with liblastfm.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef WNETWORK_CONNECTION_MONITOR_H
#define WNETWORK_CONNECTION_MONITOR_H

#include "../NetworkConnectionMonitor.h"

namespace lastfm {

class NdisEventsProxy;

class WNetworkConnectionMonitor : public NetworkConnectionMonitor
{
    Q_OBJECT
public:
    friend class lastfm::NdisEventsProxy;

    WNetworkConnectionMonitor( QObject* parent = 0 );
    ~WNetworkConnectionMonitor();

private:
    lastfm::NdisEventsProxy* m_ndisEventsProxy;
};

}

#endif // WNETWORK_CONNECTION_MONITOR_H

