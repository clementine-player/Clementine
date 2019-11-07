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

#ifndef EPISODEACTION_PRIVATE_H
#define EPISODEACTION_PRIVATE_H

#include "EpisodeAction.h"

namespace mygpo
{

class EpisodeActionPrivate : QObject
{
    Q_OBJECT

public:
    EpisodeActionPrivate( EpisodeAction* qq, const QVariant& variant, QObject* parent = 0 );
    EpisodeActionPrivate( EpisodeAction* qq, const QUrl& podcastUrl, const QUrl& episodeUrl, const QString& deviceName, EpisodeAction::ActionType action, qulonglong timestamp, qulonglong started, qulonglong position, qulonglong total, QObject* parent = 0 );
    virtual ~EpisodeActionPrivate();

    QUrl podcastUrl() const;
    QUrl episodeUrl() const;
    QString deviceName() const;
    EpisodeAction::ActionType action() const;
    qulonglong timestamp() const;
    qulonglong started() const;
    qulonglong position() const;
    qulonglong total() const;
private:
    EpisodeAction* const q;

    QUrl m_podcastUrl;
    QUrl m_episodeUrl;
    QString m_deviceName;
    EpisodeAction::ActionType m_action;
    qulonglong m_timestamp;
    qulonglong m_started;
    qulonglong m_position;
    qulonglong m_total;

    bool parse( const QVariant& data );
    bool parse( const QByteArray& data );
    bool parseActionType( const QString& data );
};

}

#endif //EPISODEACTION_PRIVATE_H
