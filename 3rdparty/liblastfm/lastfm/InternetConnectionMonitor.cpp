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

#include "InternetConnectionMonitor.h"
#include "linux/LNetworkConnectionMonitor.h"
#include "mac/MNetworkConnectionMonitor.h"
#include "win/WNetworkConnectionMonitor.h"
#include "NetworkConnectionMonitor.h"
#include "ws.h"

#include <QDebug>

class lastfm::InternetConnectionMonitorPrivate
{
public:
    bool m_up;
    NetworkConnectionMonitor* m_networkMonitor;
};

lastfm::InternetConnectionMonitor::InternetConnectionMonitor( QObject *parent )
                                 : QObject( parent )
                                 , d( new InternetConnectionMonitorPrivate)
{
    d->m_up = true;
    d->m_networkMonitor = createNetworkConnectionMonitor();

    if ( d->m_networkMonitor )
    {
        connect( d->m_networkMonitor, SIGNAL( networkUp() ), this, SLOT( onNetworkUp() ) );
        connect( d->m_networkMonitor, SIGNAL( networkDown() ), this, SLOT( onNetworkDown() ) );
    }

    connect( nam(), SIGNAL( finished( QNetworkReply* ) ), this, SLOT( onFinished( QNetworkReply* ) ) );
}

lastfm::InternetConnectionMonitor::~InternetConnectionMonitor()
{
    delete d;
}

bool lastfm::InternetConnectionMonitor::isDown() const
{
    return !d->m_up;
}

bool lastfm::InternetConnectionMonitor::isUp() const
{
    return d->m_up;
}

void
lastfm::InternetConnectionMonitor::onFinished( QNetworkReply* reply )
{
    if( reply->attribute( QNetworkRequest::SourceIsFromCacheAttribute).toBool() ) return;

    switch( reply->error() )
    {
        case QNetworkReply::NoError:
            if ( !d->m_up )
            {
                d->m_up = true;
                emit up();
                emit connectivityChanged( d->m_up );
                qDebug() << "Internet connection is reachable :)";
            }
            break;
        case QNetworkReply::HostNotFoundError:
        case QNetworkReply::TimeoutError:
        case QNetworkReply::ProxyConnectionRefusedError:
        case QNetworkReply::ProxyConnectionClosedError:
        case QNetworkReply::ProxyNotFoundError:
        case QNetworkReply::ProxyTimeoutError:
        case QNetworkReply::ProxyAuthenticationRequiredError:
            if ( d->m_up )
            {
                d->m_up = false;
                emit down();
                emit connectivityChanged( d->m_up );
            }
            break;
        default:
            break;
    }
}

void
lastfm::InternetConnectionMonitor::onNetworkUp()
{
#ifdef Q_OS_MAC
    // We don't need to check on mac as the
    // check is done as part of the reach api
    d->m_up = true;

    emit up();
    emit connectivityChanged( d->m_up );
    qDebug() << "Internet connection is reachable :)";
#else
    qDebug() << "Network seems to be up again. Let's try if there's internet connection!";
    nam()->head( QNetworkRequest( QUrl( "http://www.last.fm/" ) ) );
#endif
}

void
lastfm::InternetConnectionMonitor::onNetworkDown()
{
    qDebug() << "Internet is unreachable :(";
    d->m_up = false;
    emit down();
    emit connectivityChanged( d->m_up );
}

lastfm::NetworkConnectionMonitor*
lastfm::InternetConnectionMonitor::createNetworkConnectionMonitor()
{
    NetworkConnectionMonitor* ncm = 0;

#if defined(Q_OS_MAC)
    ncm = new MNetworkConnectionMonitor( this );
#elif defined(Q_OS_WIN) && ! defined __MINGW32__
    ncm = new WNetworkConnectionMonitor( this );
#elif defined(Q_OS_LINUX)
    ncm = new LNetworkConnectionMonitor( this );
#endif

    return ncm;
}
