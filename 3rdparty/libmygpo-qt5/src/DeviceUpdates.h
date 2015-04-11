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

#ifndef LIBMYGPO_QT_DEVICEUPDATES_H
#define LIBMYGPO_QT_DEVICEUPDATES_H

#include <QNetworkReply>
#include <QUrl>

#include "mygpo_export.h"
#include "Podcast.h"
#include "Episode.h"

namespace mygpo
{

class DeviceUpdatesPrivate;

class MYGPO_EXPORT DeviceUpdates : public QObject
{
    Q_OBJECT
    Q_PROPERTY( QVariant add READ add CONSTANT )
    Q_PROPERTY( QVariant update READ update CONSTANT )
    Q_PROPERTY( QVariant remove READ remove CONSTANT )
    Q_PROPERTY( qulonglong timestamp READ timestamp CONSTANT )
public:
    DeviceUpdates( QNetworkReply* reply, QObject* parent = 0 );
    virtual ~DeviceUpdates();
    QList<PodcastPtr> addList() const;
    QList<EpisodePtr> updateList() const;
    QList<QUrl> removeList() const;
    QVariant add() const;
    QVariant update() const;
    QVariant remove() const;
    qulonglong timestamp() const;
private:
    Q_DISABLE_COPY( DeviceUpdates )
    DeviceUpdatesPrivate* const d;
    friend class DeviceUpdatesPrivate;
signals:
    /**Gets emitted when the data is ready to read*/
    void finished();
    /**Gets emitted when an parse error ocurred*/
    void parseError();
    /**Gets emitted when an request error ocurred*/
    void requestError( QNetworkReply::NetworkError error );
};

typedef QSharedPointer<DeviceUpdates> DeviceUpdatesPtr;

}

#endif // LIBMYGPO_QT_DEVICEUPDATES_H
