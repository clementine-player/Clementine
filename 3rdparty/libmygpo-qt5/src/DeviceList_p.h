/***************************************************************************
* This file is part of libmygpo-qt                                         *
* Copyright (c) 2010 - 2013 Stefan Derkits <stefan@derkits.at>             *
* Copyright (c) 2010 - 2011 Christian Wagner <christian.wagner86@gmx.at>   *
* Copyright (c) 2010 - 2011 Felix Winter <ixos01@gmail.com>                *
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

#ifndef DEVICELIST_PRIVATE_H
#define DEVICELIST_PRIVATE_H

#include "DeviceList.h"

namespace mygpo
{

class DeviceListPrivate : public QObject
{
    Q_OBJECT
public:
    DeviceListPrivate( DeviceList* qq, QNetworkReply* reply );
    virtual ~DeviceListPrivate();
    QVariant devices() const;
    QList< DevicePtr > devicesList() const;

private:
    DeviceList* q;
    QNetworkReply* m_reply;
    QVariant m_devices;
    QList<DevicePtr> m_devicesList;
    QNetworkReply::NetworkError m_error;

    bool parse( const QVariant& data );
    bool parse( const QByteArray& data );

private slots:
    void parseData();
    void error( QNetworkReply::NetworkError error );
};

}

#endif //DEVICELIST_PRIVATE_H
