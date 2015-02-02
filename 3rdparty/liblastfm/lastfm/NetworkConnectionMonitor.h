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

#ifndef NETWORK_CONNECTION_MONITOR_H
#define NETWORK_CONNECTION_MONITOR_H

#include "global.h"
#include <QObject>

namespace lastfm
{

class LASTFM_DLLEXPORT NetworkConnectionMonitor : public QObject
{
    Q_OBJECT
public:
    NetworkConnectionMonitor( QObject *parent = 0 );
    ~NetworkConnectionMonitor();
    bool isConnected() const;

signals:
    void networkUp();
    void networkDown();

protected:
    void setConnected( bool connected );

private:
    class NetworkConnectionMonitorPrivate * const d;
};

}

#endif // NETWORK_CONNECTION_MONITOR_H
