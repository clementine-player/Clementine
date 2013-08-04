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

#ifndef LIBMYGPO_QT_DEVICESYNCRESULT_H
#define LIBMYGPO_QT_DEVICESYNCRESULT_H

#include <QNetworkReply>
#include <QObject>
#include <QSharedPointer>
#include <QStringList>

#include "mygpo_export.h"

namespace mygpo {

class DeviceSyncResultPrivate;
    
class MYGPO_EXPORT DeviceSyncResult : public QObject
{
    Q_OBJECT
    Q_PROPERTY( QVariant synchronized READ synchronized CONSTANT )
    Q_PROPERTY( QVariant notSynchronized READ notSynchronized CONSTANT )
public:
    DeviceSyncResult ( QNetworkReply* reply, QObject* parent = 0 );
    virtual ~DeviceSyncResult();
    
    QVariant synchronized() const;
    QVariant notSynchronized() const;
    
    QList<QStringList> synchronizedList() const;
    QList<QString> notSynchronizedList() const;
private:
    Q_DISABLE_COPY( DeviceSyncResult )
    DeviceSyncResultPrivate* const d;
    friend class DeviceSyncResultPrivate;
signals:
    /**Gets emitted when the data is ready to read*/
    void finished();
    /**Gets emitted when an parse error ocurred*/
    void parseError();
    /**Gets emitted when an request error ocurred*/
    void requestError( QNetworkReply::NetworkError error );
};

typedef QSharedPointer<DeviceSyncResult> DeviceSyncResultPtr;

}

#endif // LIBMYGPO_QT_DEVICESYNCRESULT_H
