/*
   Copyright 2009 Last.fm Ltd.
      - Primarily authored by Jono Cole and Michael Coffey

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

#ifndef MNETWORK_CONNECTION_MONITOR_H
#define MNETWORK_CONNECTION_MONITOR_H

#include "../NetworkConnectionMonitor.h"

#ifdef Q_OS_MAC
#include <SystemConfiguration/SCNetwork.h> //TODO remove
#include <SystemConfiguration/SCNetworkReachability.h>
#endif

class __SCNetworkReachability;

namespace lastfm
{

class MNetworkConnectionMonitor : public NetworkConnectionMonitor
{
    Q_OBJECT
public:
    MNetworkConnectionMonitor( QObject* parent = 0 );
    ~MNetworkConnectionMonitor();
private slots:

private:
#ifdef Q_OS_MAC
    static void callback( SCNetworkReachabilityRef target,
                          SCNetworkConnectionFlags flags,
                          void *info );
#endif
};

}

#endif // MNETWORK_CONNECTION_MONITOR_H

