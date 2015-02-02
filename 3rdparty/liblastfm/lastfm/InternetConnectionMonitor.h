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

#ifndef LASTFM_CONNECTION_MONITOR_H
#define LASTFM_CONNECTION_MONITOR_H

#include "global.h"
#include <QObject>

class QNetworkReply;

namespace lastfm {

class NetworkConnectionMonitor;

class LASTFM_DLLEXPORT InternetConnectionMonitor : public QObject
{
    Q_OBJECT

public:
    /** if internet is unavailable you will get a down() signal soon, otherwise
      * you won't get a signal until the net goes down */
    InternetConnectionMonitor( QObject *parent = 0 );
    ~InternetConnectionMonitor();

    bool isDown() const;
    bool isUp() const;

    NetworkConnectionMonitor* createNetworkConnectionMonitor();

signals:
    /** yay! internet has returned */
    void up( const QString& connectionName = "" );
    
    /** we think the internet is unavailable, but well, still try, but show
      * an unhappy face in the statusbar or something */
    void down( const QString& connectionName = "" );
    
    /** emitted after the above */
    void connectivityChanged( bool );

private slots:
    void onFinished( QNetworkReply* reply );
    void onNetworkUp();
    void onNetworkDown();

private:
    class InternetConnectionMonitorPrivate * const d;
};

} //namespace lastfm

#endif
