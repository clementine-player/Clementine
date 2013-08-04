/***************************************************************************
* This file is part of libmygpo-qt                                         *
* Copyright (c) 2011 - 2013 Stefan Derkits <stefan@derkits.at>             *
*                                                                          *
* This library is free software; you can redistribute it and/or            *
* modify it under the terms of the GNU Lesser General Public               *
* License as published by the Free Software Foundation; either             *
* version 2.1 of the License, or (at your option) any later version.       *
*                                                                          *
* This library is distributed in the hope that it will be useful,          *
* but WITHOUT ANY WARRANTY; without even the implied warranty of           *
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU        *
* Lesser General Public License for more details.                          *
*                                                                          *
* You should have received a copy of the GNU Lesser General Public         *
* License along with this library; if not, write to the Free Software      *
* Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 *
* USA                                                                      *
***************************************************************************/

#ifndef DEVICESYNCRESULT_PRIVATE_H
#define DEVICESYNCRESULT_PRIVATE_H

#include "DeviceSyncResult.h"

namespace mygpo {

class DeviceSyncResultPrivate : public QObject
{
   Q_OBJECT
public:
    DeviceSyncResultPrivate( DeviceSyncResult* qq, QNetworkReply* reply );
    virtual ~DeviceSyncResultPrivate();
    
    QVariant synchronized() const;
    QVariant notSynchronized() const;
    
    QList<QStringList> synchronizedList() const;
    QList<QString> notSynchronizedList() const;
private:
    DeviceSyncResult* q;
    QVariant m_synchronized;
    QVariant m_notSynchronized;
    
    QNetworkReply* m_reply;
    QNetworkReply::NetworkError m_error;

    bool parse( const QVariant& data );
    bool parse( const QByteArray& data );
private slots:
    void parseData();
    void error( QNetworkReply::NetworkError error );

};

}

#endif // DEVICESYNCRESULT_H
