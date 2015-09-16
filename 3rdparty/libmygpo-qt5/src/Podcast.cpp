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

#include "Podcast_p.h"

#include "qjsonwrapper/Json.h"

using namespace mygpo;

PodcastPrivate::PodcastPrivate( Podcast* qq, QNetworkReply* reply ): m_reply( reply ), q( qq ), m_error( QNetworkReply::NoError )
{
    QObject::connect( m_reply, SIGNAL( finished() ), this, SLOT( parseData() ) );
    QObject::connect( m_reply, SIGNAL( error( QNetworkReply::NetworkError ) ), this, SLOT( error( QNetworkReply::NetworkError ) ) );
}

PodcastPrivate::PodcastPrivate( Podcast* qq, const QVariant& variant ): m_reply( 0 ), q( qq ), m_error( QNetworkReply::NoError )
{
    parse( variant );
}

PodcastPrivate::~PodcastPrivate()
{
}


QUrl PodcastPrivate::url() const
{
    return m_url;
}

QString PodcastPrivate::title() const
{
    return m_title;
}

QString PodcastPrivate::description() const
{
    return m_description;
}

uint PodcastPrivate::subscribers() const
{
    return m_subscribers;
}

uint PodcastPrivate::subscribersLastWeek() const
{
    return m_SubscribersLastWeek;
}


QUrl PodcastPrivate::logoUrl() const
{
    return m_logoUrl;
}

QUrl PodcastPrivate::website() const
{
    return m_website;
}

QUrl PodcastPrivate::mygpoUrl() const
{
    return m_mygpoUrl;
}

Podcast::Podcast( QNetworkReply* reply, QObject* parent ) : QObject( parent ), d( new PodcastPrivate( this, reply ) )
{

}

Podcast::Podcast( const QVariant& variant, QObject* parent ): QObject( parent ), d( new PodcastPrivate( this, variant ) )
{

}

Podcast::~Podcast()
{
    delete d;
}

QUrl Podcast::url() const
{
    return d->url();
}

QString Podcast::title() const
{
    return d->title();
}

QString Podcast::description() const
{
    return d->description();
}

uint Podcast::subscribers() const
{
    return d->subscribers();
}

uint Podcast::subscribersLastWeek() const
{
    return d->subscribersLastWeek();
}


QUrl Podcast::logoUrl() const
{
    return d->logoUrl();
}

QUrl Podcast::website() const
{
    return d->website();
}

QUrl Podcast::mygpoUrl() const
{
    return d->mygpoUrl();
}

bool PodcastPrivate::parse( const QVariant& data )
{
    if ( !data.canConvert( QVariant::Map ) )
        return false;
    QVariantMap podcastMap = data.toMap();
    QVariant v = podcastMap.value( QLatin1String( "url" ) );
    if ( !v.canConvert( QVariant::Url ) )
        return false;
    m_url = v.toUrl();
    v = podcastMap.value( QLatin1String( "title" ) );
    if ( !v.canConvert( QVariant::String ) )
        return false;
    m_title = v.toString();
    v = podcastMap.value( QLatin1String( "description" ) );
    if ( !v.canConvert( QVariant::String ) )
        return false;
    m_description = v.toString();
    v = podcastMap.value( QLatin1String( "subscribers" ) );
    if ( !v.canConvert( QVariant::Int ) )
        return false;
    m_subscribers = v.toUInt();
    v = podcastMap.value( QLatin1String( "subscribers_last_week" ) );
    if ( !v.canConvert( QVariant::Int ) )
        return false;
    m_SubscribersLastWeek = v.toUInt();
    v = podcastMap.value( QLatin1String( "logo_url" ) );
    if ( !v.canConvert( QVariant::Url ) )
        return false;
    m_logoUrl = v.toUrl();
    v = podcastMap.value( QLatin1String( "website" ) );
    if ( !v.canConvert( QVariant::Url ) )
        return false;
    m_website = v.toUrl();
    v = podcastMap.value( QLatin1String( "mygpo_link" ) );
    if ( !v.canConvert( QVariant::Url ) )
        return false;
    m_mygpoUrl = v.toUrl();
    return true;
}

bool PodcastPrivate::parse( const QByteArray& data )
{
    bool ok;
    QVariant variant = QJsonWrapper::parseJson( data, &ok );
    if ( ok )
    {
        if ( !parse( variant ) ) return false;
        return true;
    }
    else
    {
        return false;
    }
}

void PodcastPrivate::parseData()
{
    if ( parse( m_reply->readAll( ) ) )
    {
        emit q->finished();
    }
    else
    {
        emit q->parseError();
    }
    m_reply->deleteLater();
}

void PodcastPrivate::error( QNetworkReply::NetworkError error )
{
    this->m_error = error;
    emit q->requestError( error );
}
