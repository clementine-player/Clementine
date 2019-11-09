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

#ifndef EPISODE_PRIVATE_H
#define EPISODE_PRIVATE_H

#include "Episode.h"

namespace mygpo
{

class EpisodePrivate : QObject
{
    Q_OBJECT

public:
    EpisodePrivate ( Episode* qq, QNetworkReply* reply, QObject* parent = 0 );
    EpisodePrivate ( Episode* qq, const QVariant& variant, QObject* parent = 0 );
    virtual ~EpisodePrivate();
    QUrl url() const;
    QString title() const;
    QUrl podcastUrl() const;
    QString podcastTitle() const;
    QString description() const;
    QUrl website() const;
    QUrl mygpoUrl() const;
    QDateTime releaded() const;
    Episode::Status status() const;
private:
    QNetworkReply* m_reply;
    Episode* const q;
    QUrl m_url;
    QString m_title;
    QUrl m_podcastUrl;
    QString m_podcastTitle;
    QString m_description;
    QUrl m_website;
    QUrl m_mygpoUrl;
    QDateTime m_released;
    Episode::Status m_status;
    QNetworkReply::NetworkError m_error;
    bool parse ( const QVariant& data );
    bool parse ( const QByteArray& data );
private slots:
    void parseData();
    void error ( QNetworkReply::NetworkError error );

};

}  // namespace mygpo

#endif // EPISODE_PRIVATE_H
