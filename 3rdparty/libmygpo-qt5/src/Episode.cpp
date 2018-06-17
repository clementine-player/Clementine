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

#include "Episode_p.h"

#include "qjsonwrapper/Json.h"

using namespace mygpo;

EpisodePrivate::~EpisodePrivate()
{
}

EpisodePrivate::EpisodePrivate ( Episode* qq, QNetworkReply* reply, QObject* parent ) : QObject ( parent ), m_reply ( reply ), q ( qq ), m_error ( QNetworkReply::NoError )
{
    QObject::connect ( m_reply, SIGNAL ( finished() ), this, SLOT ( parseData() ) );
    QObject::connect ( m_reply, SIGNAL ( error ( QNetworkReply::NetworkError ) ), this, SLOT ( error ( QNetworkReply::NetworkError ) ) );
}

EpisodePrivate::EpisodePrivate ( Episode* qq, const QVariant& variant, QObject* parent ) : QObject ( parent ), m_reply ( 0 ), q ( qq )
{
    parse ( variant );
}

bool EpisodePrivate::parse ( const QVariant& data )
{
    if ( !data.canConvert ( QVariant::Map ) )
        return false;
    QVariantMap episodeMap = data.toMap();
    QVariant s = episodeMap.value ( QLatin1String ( "url" ) );
    if ( !s.canConvert ( QVariant::Url ) )
        return false;
    m_url = s.toUrl();
    s = episodeMap.value ( QLatin1String ( "title" ) );
    if ( !s.canConvert ( QVariant::String ) )
        return false;
    m_title = s.toString();
    s = episodeMap.value ( QLatin1String ( "podcast_url" ) );
    if ( !s.canConvert ( QVariant::Url ) )
        return false;
    m_podcastUrl = s.toUrl();
    s = episodeMap.value ( QLatin1String ( "podcast_title" ) );
    if ( !s.canConvert ( QVariant::String ) )
        return false;
    m_podcastTitle = s.toString();
    s = episodeMap.value ( QLatin1String ( "description" ) );
    if ( !s.canConvert ( QVariant::String ) )
        return false;
    m_description = s.toString();
    s = episodeMap.value ( QLatin1String ( "website" ) );
    if ( !s.canConvert ( QVariant::Url ) )
        return false;
    m_website = s.toUrl();
    s = episodeMap.value ( QLatin1String ( "mygpo_link" ) );
    if ( !s.canConvert ( QVariant::Url ) )
        return false;
    m_mygpoUrl = s.toUrl();
    s = episodeMap.value ( QLatin1String ( "status" ) );
    if ( s.canConvert ( QVariant::String ) )
    {
        QString status = s.toString();
        m_status = Episode::UNKNOWN;
        if ( QString::compare ( status, QLatin1String ( "new" ) ,Qt::CaseInsensitive ) == 0 )
        {
            m_status = Episode::NEW;
        }
        else if ( QString::compare ( status, QLatin1String ( "play" ) ,Qt::CaseInsensitive ) == 0 )
        {
            m_status = Episode::PLAY;
        }
        else if ( QString::compare ( status, QLatin1String ( "download" ) ,Qt::CaseInsensitive ) == 0 )
        {
            m_status = Episode::DOWNLOAD;
        }
        else if ( QString::compare ( status, QLatin1String ( "delete" ) ,Qt::CaseInsensitive ) == 0 )
        {
            m_status = Episode::DELETE;
        }
    }
    else
    {
        m_status = Episode::UNKNOWN;
    }
    s = episodeMap.value( QLatin1String ( "released" ) );
    if ( s.canConvert( QVariant::String ) )
    {
        QString date = s.toString();
        m_released = QDateTime::fromString( date, Qt::ISODate );
    }
    else
    {
        m_released = QDateTime::currentDateTime();
    }
    return true;
}

bool EpisodePrivate::parse ( const QByteArray& data )
{
    bool ok;
    QVariant variant = QJsonWrapper::parseJson( data, &ok );
    if ( ok )
    {
        if ( !parse ( variant ) ) return false;
        return true;
    }
    else
    {
        return false;
    }
}

void EpisodePrivate::parseData()
{
    //parse and send signal
    if ( m_reply->error() == QNetworkReply::NoError )
    {
        if ( parse ( m_reply->readAll() ) )
        {
            emit q->finished();
        }
        else
        {
            emit q->parseError();
        }
    }
    m_reply->deleteLater();
}

void EpisodePrivate::error ( QNetworkReply::NetworkError error )
{
    this->m_error = error;
    emit q->requestError ( error );
}

QString EpisodePrivate::description() const
{
    return m_description;
}

QUrl EpisodePrivate::mygpoUrl() const
{
    return m_mygpoUrl;
}

QString EpisodePrivate::podcastTitle() const
{
    return m_podcastTitle;
}

QUrl EpisodePrivate::podcastUrl() const
{
    return m_podcastUrl;
}

QString EpisodePrivate::title() const
{
    return m_title;
}

QUrl EpisodePrivate::url() const
{
    return m_url;
}

QUrl EpisodePrivate::website() const
{
    return m_website;
}

Episode::Status EpisodePrivate::status() const
{
    return m_status;
}

QDateTime EpisodePrivate::releaded() const
{
    return m_released;
}

Episode::Episode ( QNetworkReply* reply, QObject* parent ) : QObject ( parent ), d ( new EpisodePrivate ( this, reply ) )
{

}

Episode::Episode ( const QVariant& variant, QObject* parent ) : QObject ( parent ), d ( new EpisodePrivate ( this, variant ) )
{

}

Episode::~Episode()
{
    delete d;
}

QUrl Episode::url() const
{
    return d->url();
}

QString Episode::title() const
{
    return d->title();
}

QUrl Episode::podcastUrl() const
{
    return d->podcastUrl();
}

QString Episode::podcastTitle() const
{
    return d->podcastTitle();
}

QString Episode::description() const
{
    return d->description();
}

QUrl Episode::website() const
{
    return d->website();
}

QUrl Episode::mygpoUrl() const
{
    return d->mygpoUrl();
}

Episode::Status Episode::status() const
{
    return d->status();
}

QDateTime Episode::released() const
{
    return d->releaded();
}
