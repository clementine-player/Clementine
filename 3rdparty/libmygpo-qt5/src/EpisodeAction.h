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

#ifndef LIBMYGPO_QT_EPISODEACTION_H
#define LIBMYGPO_QT_EPISODEACTION_H

#include <QUrl>
#include <QString>
#include <QNetworkReply>
#include <QSharedPointer>

#include "mygpo_export.h"

namespace mygpo
{

class EpisodeActionPrivate;

class MYGPO_EXPORT EpisodeAction : public QObject
{
    Q_OBJECT
    Q_ENUMS( ActionType )
    Q_PROPERTY( QUrl podcastUrl READ podcastUrl CONSTANT )
    Q_PROPERTY( QUrl episodeUrl READ episodeUrl CONSTANT )
    Q_PROPERTY( QString deviceName READ deviceName CONSTANT )
    Q_PROPERTY( ActionType action READ action CONSTANT )
    Q_PROPERTY( qulonglong timestamp READ timestamp CONSTANT )
    Q_PROPERTY( qulonglong started READ started CONSTANT )
    Q_PROPERTY( qulonglong position READ position CONSTANT )
    Q_PROPERTY( qulonglong total READ total CONSTANT )

public:
    enum ActionType { Download, Play, Delete, New, Flattr };
    EpisodeAction( const QVariant& variant, QObject* parent = 0 );
    EpisodeAction( const QUrl& podcastUrl, const QUrl& episodeUrl, const QString& deviceName, EpisodeAction::ActionType action, qulonglong timestamp, qulonglong started, qulonglong position, qulonglong total, QObject* parent = 0 );
    EpisodeAction( const QUrl& podcastUrl, const QUrl& episodeUrl, const QString& deviceName, EpisodeAction::ActionType action, qulonglong timestamp, qulonglong position, QObject* parent = 0 );
    EpisodeAction( const QUrl& podcastUrl, const QUrl& episodeUrl, const QString& deviceName, EpisodeAction::ActionType action, qulonglong timestamp, QObject* parent = 0 );
    virtual ~EpisodeAction();

    QUrl podcastUrl() const;
    QUrl episodeUrl() const;
    QString deviceName() const;
    EpisodeAction::ActionType action() const;
    qulonglong timestamp() const;
    qulonglong started() const;
    qulonglong position() const;
    qulonglong total() const;

private:
    Q_DISABLE_COPY( EpisodeAction )
    EpisodeActionPrivate* const d;
    friend class EpisodeActionPrivate;
};

typedef QSharedPointer<EpisodeAction> EpisodeActionPtr;

}

Q_DECLARE_METATYPE( mygpo::EpisodeActionPtr );

#endif // LIBMYGPO_QT_EPISODEACTION_H
