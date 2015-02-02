/*
   Copyright 2010 Last.fm Ltd.
      - Primarily authored by Max Howell, Jono Cole and Doug Mansell

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


#ifndef LNETWORK_CONNECTION_MONITOR_H
#define LNETWORK_CONNECTION_MONITOR_H

#include "../NetworkConnectionMonitor.h"
#include "global.h"

class QDBusConnection;
class QDBusInterface;

namespace lastfm
{

class LNetworkConnectionMonitor : public NetworkConnectionMonitor
{
    Q_OBJECT

    typedef enum {
        NM_STATE_UNKNOWN          = 0,
        NM_STATE_ASLEEP           = 10,
        NM_STATE_DISCONNECTED     = 20,
        NM_STATE_DISCONNECTING    = 30,
        NM_STATE_CONNECTING       = 40,
        NM_STATE_CONNECTED_LOCAL  = 50,
        NM_STATE_CONNECTED_SITE   = 60,
        NM_STATE_CONNECTED_GLOBAL = 70
    } NMState;

public:
    LNetworkConnectionMonitor( QObject* parent = 0 );
    ~LNetworkConnectionMonitor();
private slots:
    void onStateChange( uint newState );
private:
    QDBusInterface* m_nmInterface;
};

}

#endif // LNETWORK_CONNECTION_MONITOR_H

