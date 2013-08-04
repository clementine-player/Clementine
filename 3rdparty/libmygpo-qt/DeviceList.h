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

#ifndef LIBMYGPO_QT_DEVICELIST_H
#define LIBMYGPO_QT_DEVICELIST_H

#include <QNetworkReply>
#include <QSharedPointer>

#include "Device.h"
#include "mygpo_export.h"

namespace mygpo
{

class DeviceListPrivate;

class MYGPO_EXPORT DeviceList : public QObject
{
    Q_OBJECT
    Q_PROPERTY( QVariant devices READ devices CONSTANT )

public:
    DeviceList( QNetworkReply* reply, QObject* parent = 0 );
    virtual ~DeviceList();
    QVariant devices() const;
    QList< DevicePtr > devicesList() const;

private:
    Q_DISABLE_COPY( DeviceList )
    DeviceListPrivate* const d;
    friend class DeviceListPrivate;
signals:
    /**Gets emitted when the data is ready to read*/
    void finished();
    /**Gets emitted when an parse error ocurred*/
    void parseError();
    /**Gets emitted when an request error ocurred*/
    void requestError( QNetworkReply::NetworkError error );

};

typedef QSharedPointer<DeviceList> DeviceListPtr;

}

Q_DECLARE_METATYPE( mygpo::DeviceListPtr );

#endif //LIBMYGPO_QT_DEVICELIST_H
