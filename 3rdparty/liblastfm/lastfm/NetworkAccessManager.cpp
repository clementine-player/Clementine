/*
   Copyright 2009 Last.fm Ltd. 
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
#include "NetworkAccessManager.h"
#include "InternetConnectionMonitor.h"
#include "ws.h"
#include "misc.h"
#include <QCoreApplication>
#include <QNetworkRequest>
#include <QNetworkProxyQuery>

class NetworkAccessManagerPrivate
{
    public:
    NetworkAccessManagerPrivate();
    QNetworkProxy userProxy;
};

NetworkAccessManagerPrivate::NetworkAccessManagerPrivate() :
    userProxy( QNetworkProxy( QNetworkProxy::DefaultProxy ) )
{
}


// TODO: Use a d-pointer on the next SONAME bump
typedef QHash< const lastfm::NetworkAccessManager *, NetworkAccessManagerPrivate *> NamPrivateHash;
Q_GLOBAL_STATIC( NamPrivateHash, d_func )
static NetworkAccessManagerPrivate * d( const lastfm::NetworkAccessManager * nam )
{
    NetworkAccessManagerPrivate * ret = d_func()->value( nam, 0 );
    if ( !ret )
    {
        ret = new NetworkAccessManagerPrivate;
        d_func()->insert( nam, ret );
    }
    return ret;
}
static void delete_d( const lastfm::NetworkAccessManager * nam )
{
    const NetworkAccessManagerPrivate * ret = d_func()->value( nam, 0 );
    delete ret;
    d_func()->remove( nam );
}


namespace lastfm 
{
    LASTFM_DLLEXPORT QByteArray UserAgent;
}


lastfm::NetworkAccessManager::NetworkAccessManager( QObject* parent )
               : QNetworkAccessManager( parent )
            #if defined WIN32 && ! defined __MINGW32__
               , m_monitor( 0 )
            #endif
{
    // can't be done in above init, as applicationName() won't be set
    if (lastfm::UserAgent.isEmpty())
    {
        QByteArray name = QCoreApplication::applicationName().toUtf8();
        QByteArray version = QCoreApplication::applicationVersion().toUtf8();
        if (version.size()) version.prepend( ' ' );
        lastfm::UserAgent = name + version + " (" + lastfm::platform() + ")";
    }
}


lastfm::NetworkAccessManager::~NetworkAccessManager()
{
    delete_d( this );
}

void
lastfm::NetworkAccessManager::setUserProxy( const QNetworkProxy& proxy )
{
    d( this )->userProxy = proxy;
}

QNetworkProxy
lastfm::NetworkAccessManager::proxy( const QNetworkRequest& request )
{   
    if ( d( this )->userProxy.type() != QNetworkProxy::DefaultProxy )
        return d( this )->userProxy;

    QList<QNetworkProxy> proxies = QNetworkProxyFactory::systemProxyForQuery( QNetworkProxyQuery( request.url() ) );

    return proxies[0];
}


QNetworkReply*
lastfm::NetworkAccessManager::createRequest( Operation op, const QNetworkRequest& request_, QIODevice* outgoingData )
{
    QNetworkRequest request = request_;

    request.setAttribute( QNetworkRequest::CacheLoadControlAttribute, QNetworkRequest::PreferCache );
    request.setRawHeader( "User-Agent", lastfm::UserAgent );
    
    // PAC proxies can vary by domain, so we have to check everytime :(
    QNetworkAccessManager::setProxy( this->proxy( request ) );

    return QNetworkAccessManager::createRequest( op, request, outgoingData );
}


void
lastfm::NetworkAccessManager::onConnectivityChanged( bool up )
{
    Q_UNUSED( up );
}
