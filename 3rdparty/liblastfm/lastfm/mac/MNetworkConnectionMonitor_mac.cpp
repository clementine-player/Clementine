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

#include "MNetworkConnectionMonitor.h"
#include "moc_MNetworkConnectionMonitor.cpp"
#include "../ws.h"

#include <QPointer>
#include <SystemConfiguration/SCNetworkReachability.h>

lastfm::MNetworkConnectionMonitor* context = 0;

lastfm::MNetworkConnectionMonitor::MNetworkConnectionMonitor( QObject* parent ) :
    NetworkConnectionMonitor( parent )
{
    context = this;

    SCNetworkReachabilityRef ref = SCNetworkReachabilityCreateWithName( NULL, LASTFM_WS_HOSTNAME );
    SCNetworkReachabilityScheduleWithRunLoop( ref, CFRunLoopGetCurrent(), kCFRunLoopDefaultMode );
    SCNetworkReachabilitySetCallback( ref, callback, NULL );
    CFRelease( ref );
}

lastfm::MNetworkConnectionMonitor::~MNetworkConnectionMonitor()
{
}


void
lastfm::MNetworkConnectionMonitor::callback( SCNetworkReachabilityRef target,
                                     SCNetworkConnectionFlags flags,
                                     void * )
{
    static bool up = true;

    // I couldn't find any diffinitive usage examples for these flags
    // so I had to guess, since I can't test, eg. dial up :(

    bool b;
    if (flags & kSCNetworkFlagsConnectionRequired)
        b = false;
    else
        b = flags & (kSCNetworkFlagsReachable | kSCNetworkFlagsTransientConnection | kSCNetworkFlagsConnectionAutomatic);

    // basically, avoids telling everyone that we're up already on startup
    if (up == b)
        return;

    up = b;

    context->setConnected(b);
}

