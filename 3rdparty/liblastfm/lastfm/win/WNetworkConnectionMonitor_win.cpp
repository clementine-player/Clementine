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

#include "WNetworkConnectionMonitor.h"
#include "moc_WNetworkConnectionMonitor.cpp"

// WsAccessManager needs special init (on Windows), and it needs to be done
// early, so be careful about moving this
#include "../win/ComSetup.h" //must be first header or compile fail results!
#include "../win/NdisEvents.h"
static ComSetup com_setup;

namespace lastfm {

// bounce NdisEvents signals through here so we don't have to expose the
// NdisEvents interface in InternetConnectionMonitor  :)
class NdisEventsProxy : public NdisEvents
{
public:
    NdisEventsProxy(WNetworkConnectionMonitor* icm)
        :m_icm(icm)
    {
    }

    // WmiSink callbacks:
    void onConnectionUp( BSTR /*name*/ )
    {
        m_icm->setConnected( true );
    }

    void onConnectionDown( BSTR /*name*/ )
    {
        m_icm->setConnected( false );
    }

    WNetworkConnectionMonitor* m_icm;
};


WNetworkConnectionMonitor::WNetworkConnectionMonitor( QObject* parent ) :
    NetworkConnectionMonitor( parent )
{
    m_ndisEventsProxy = new lastfm::NdisEventsProxy( this );
    m_ndisEventsProxy->registerForNdisEvents();
}

WNetworkConnectionMonitor::~WNetworkConnectionMonitor()
{
    delete m_ndisEventsProxy;
}

}
