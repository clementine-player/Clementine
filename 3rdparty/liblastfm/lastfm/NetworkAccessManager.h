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
#ifndef LASTFM_WS_ACCESS_MANAGER_H
#define LASTFM_WS_ACCESS_MANAGER_H

#include "global.h"
#include <QtNetwork/QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkProxy>

class QNetworkReply;

namespace lastfm {

/** Sets useragent and proxy. Auto detecting the proxy where possible. */
class LASTFM_DLLEXPORT NetworkAccessManager : public QNetworkAccessManager
{
    Q_OBJECT

#ifdef Q_OS_WIN
    class InternetConnectionMonitor* m_monitor;
#endif

public:
    NetworkAccessManager( QObject *parent = 0 );
    ~NetworkAccessManager();

    void setUserProxy( const QNetworkProxy& proxy );

    /** PAC allows different proxy configurations depending on the request
      * URL and even UserAgent! Thus we allow you to pass that in, we
      * automatically configure the proxy for every request through 
      * WsAccessManager */
    QNetworkProxy proxy( const QNetworkRequest& = QNetworkRequest() );

protected:
    virtual QNetworkReply* createRequest( Operation, const QNetworkRequest&, QIODevice* outgoingdata = 0 );

private slots:
    void onConnectivityChanged( bool );

private:
    /** this function calls QNetworkAccessManager::setProxy, and thus 
      * configures the proxy correctly for the next request created by
      * createRequest. This is necessary due */
    void applyProxy( const QNetworkRequest& );
};

} //namespace lastfm

#endif
