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

#ifndef EPISODEACTIONLIST_PRIVATE_H
#define EPISODEACTIONLIST_PRIVATE_H

#include "EpisodeActionList.h"

namespace mygpo
{

class EpisodeActionListPrivate : public QObject
{
    Q_OBJECT

public:
    EpisodeActionListPrivate( EpisodeActionList* qq, QNetworkReply* reply );
    virtual ~EpisodeActionListPrivate();
    QList<EpisodeActionPtr> list() const;
    QVariant episodeActions() const;

    qulonglong timestamp() const;

private:
    QNetworkReply* m_reply;
    EpisodeActionList* const q;
    QVariant m_episodeActions;
    QNetworkReply::NetworkError m_error;
    qulonglong m_timestamp;

    bool parse( const QVariant& data );
    bool parse( const QByteArray& data );

private slots:
    void parseData();
    void error( QNetworkReply::NetworkError error );

};

}

#endif //EPISODEACTIONLIST_PRIVATE_H
