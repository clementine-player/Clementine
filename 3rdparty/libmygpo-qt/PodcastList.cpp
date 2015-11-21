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

#include "PodcastList_p.h"

#include "qjsonwrapper/Json.h"

using namespace mygpo;

PodcastListPrivate::PodcastListPrivate( PodcastList* qq, QNetworkReply* reply, QObject* parent ) : QObject( parent ), m_reply( reply ), q( qq ), m_error( QNetworkReply::NoError )
{
    QObject::connect( m_reply, SIGNAL( finished() ), this, SLOT( parseData() ) );
    QObject::connect( m_reply, SIGNAL( error( QNetworkReply::NetworkError ) ), this, SLOT( error( QNetworkReply::NetworkError ) ) );
}

PodcastListPrivate::~PodcastListPrivate()
{
}


QList< PodcastPtr > PodcastListPrivate::list() const
{
    QList<PodcastPtr> list;
    QVariantList varList = m_podcasts.toList();
    foreach( QVariant var, varList )
    {
        list.append( var.value<mygpo::PodcastPtr>() );
    }
    return list;
}

QVariant PodcastListPrivate::podcasts() const
{
    return m_podcasts;
}

bool PodcastListPrivate::parse( const QVariant& data )
{
    if( !data.canConvert( QVariant::List ) )
        return false;
    QVariantList varList = data.toList();
    QVariantList podcastList;
    foreach( QVariant var, varList )
    {
        QVariant v;
        v.setValue<mygpo::PodcastPtr> ( PodcastPtr( new Podcast( var ) ) );
        podcastList.append( v );
    }
    m_podcasts = QVariant( podcastList );
    return true;
}

bool PodcastListPrivate::parse( const QByteArray& data )
{
    bool ok;
    QVariant variant = QJsonWrapper::parseJson( data, &ok );
    if( ok )
    {
        ok = ( parse( variant ) );
    }
    return ok;
}

void PodcastListPrivate::parseData()
{
    if( m_reply->error() == QNetworkReply::NoError )
    {
        if( parse( m_reply->readAll() ) )
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

void PodcastListPrivate::error( QNetworkReply::NetworkError error )
{
    this->m_error = error;
    emit q->requestError( error );
}

PodcastList::PodcastList( QNetworkReply* reply, QObject* parent ) : QObject( parent ), d( new PodcastListPrivate( this, reply ) )
{

}

PodcastList::~PodcastList()
{
    delete d;
}

QList<PodcastPtr> PodcastList::list() const
{
    return d->list();
}

QVariant PodcastList::podcasts() const
{
    return d->podcasts();
}
