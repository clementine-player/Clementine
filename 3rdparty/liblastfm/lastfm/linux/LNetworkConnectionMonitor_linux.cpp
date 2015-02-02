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


#include "LNetworkConnectionMonitor.h"

#include <QDBusConnection>
#include <QDBusInterface>
#include <QDBusReply>
#include <QDebug>

#define NM_DBUS_SERVICE   "org.freedesktop.NetworkManager"
#define NM_DBUS_PATH      "/org/freedesktop/NetworkManager"
#define NM_DBUS_INTERFACE "org.freedesktop.NetworkManager"

lastfm::LNetworkConnectionMonitor::LNetworkConnectionMonitor( QObject* parent ) :
    NetworkConnectionMonitor( parent )
{
    m_nmInterface = new QDBusInterface( NM_DBUS_SERVICE,
                                        NM_DBUS_PATH,
                                        NM_DBUS_INTERFACE,
                                        QDBusConnection::systemBus(),
                                        this );
    if ( !m_nmInterface->isValid() )
    {
        qDebug() << "Unable to watch network state changes via D-Bus.";
        return;
    }

    //get current connection state
    QDBusReply<uint> reply = m_nmInterface->call( QDBus::AutoDetect, "state" );
    if ( reply.isValid() )
    {
        if ( reply.value() == NM_STATE_CONNECTED_GLOBAL )
        {
            setConnected( true );
        }
        else if ( reply.value() == NM_STATE_DISCONNECTED || reply.value() == NM_STATE_ASLEEP )
        {
            setConnected( false );
        }
    }
    else
    {
        qDebug() << "Error: " << reply.error();
    }

    //connect network manager signals
   m_nmInterface->connection().connect( NM_DBUS_SERVICE,
                                        NM_DBUS_PATH,
                                        NM_DBUS_INTERFACE,
                                        "StateChanged",
                                        this,
                                        SLOT( onStateChange( uint ) )
                                      );

}

lastfm::LNetworkConnectionMonitor::~LNetworkConnectionMonitor()
{
    delete m_nmInterface;
}


void
lastfm::LNetworkConnectionMonitor::onStateChange( uint newState )
{
    qDebug() << "Networkmanager state change!";

    if ( newState == NM_STATE_DISCONNECTED || newState == NM_STATE_ASLEEP )
    {
       setConnected( false );
    }
    else if ( newState == NM_STATE_CONNECTED_GLOBAL )
    {
       setConnected( true );
    }
}

#include "moc_LNetworkConnectionMonitor.cpp"
