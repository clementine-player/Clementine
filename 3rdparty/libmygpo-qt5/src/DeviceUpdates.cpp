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

#include "DeviceUpdates_p.h"

#include "qjsonwrapper/Json.h"

using namespace mygpo;

DeviceUpdatesPrivate::DeviceUpdatesPrivate( DeviceUpdates* qq, QNetworkReply* reply ): q( qq ), m_timestamp( 0 ), m_reply( reply ), m_error( QNetworkReply::NoError )
{
    QObject::connect( m_reply, SIGNAL( finished() ), this, SLOT( parseData() ) );
    QObject::connect( m_reply, SIGNAL( error( QNetworkReply::NetworkError ) ), this, SLOT( error( QNetworkReply::NetworkError ) ) );
}


DeviceUpdatesPrivate::~DeviceUpdatesPrivate()
{
}


QVariant DeviceUpdatesPrivate::add() const
{
    return m_add;
}

QList< PodcastPtr > DeviceUpdatesPrivate::addList() const
{
    QVariantList updateVarList = m_add.toList();
    QList<PodcastPtr> ret;
    foreach( const QVariant & var, updateVarList )
    {
        ret.append( PodcastPtr( new Podcast( var ) ) );
    }
    return ret;
}

QVariant DeviceUpdatesPrivate::remove() const
{
    return m_remove;
}

QList< QUrl > DeviceUpdatesPrivate::removeList() const
{
    QVariantList updateVarList = m_remove.toList();
    QList<QUrl> ret;
    foreach( const QVariant & var, updateVarList )
    {
        if( var.canConvert( QVariant::Url ) )
            ret.append( var.toUrl() );
    }
    return ret;
}

QVariant DeviceUpdatesPrivate::update() const
{
    return m_update;
}

QList< EpisodePtr > DeviceUpdatesPrivate::updateList() const
{
    QVariantList updateVarList = m_update.toList();
    QList<EpisodePtr> ret;
    foreach( const QVariant & var, updateVarList )
    {
        ret.append( EpisodePtr( new Episode( var ) ) );
    }
    return ret;
}

bool DeviceUpdatesPrivate::parse( const QVariant& data )
{
    if( !data.canConvert( QVariant::Map ) )
        return false;
    QVariantMap varMap = data.toMap();
    m_add = varMap.value( QLatin1String( "add" ) );
    m_remove = varMap.value( QLatin1String( "remove" ) );
    m_update = varMap.value( QLatin1String( "updates" ) );
    if( varMap.value( QLatin1String( "timestamp" ) ).canConvert( QVariant::LongLong ) )
        m_timestamp = varMap.value( QLatin1String( "timestamp" ) ).toLongLong();
    return true;
}

bool DeviceUpdatesPrivate::parse( const QByteArray& data )
{
    bool ok;
    QVariant variant = QJsonWrapper::parseJson( data, &ok );
    if( ok )
    {
        ok = ( parse( variant ) );
    }
    return ok;
}

void DeviceUpdatesPrivate::parseData()
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

void DeviceUpdatesPrivate::error( QNetworkReply::NetworkError error )
{
    m_error = error;
    emit q->requestError( error );
}

qulonglong DeviceUpdatesPrivate::timestamp() const
{
    return m_timestamp;
}


DeviceUpdates::DeviceUpdates( QNetworkReply* reply, QObject* parent ): QObject( parent ), d( new DeviceUpdatesPrivate( this, reply ) )
{

}

DeviceUpdates::~DeviceUpdates()
{
    delete d;
}


QVariant DeviceUpdates::add() const
{
    return d->add();
}

QList< PodcastPtr > DeviceUpdates::addList() const
{
    return d->addList();
}

QVariant mygpo::DeviceUpdates::remove() const
{
    return d->remove();
}

QList< QUrl > mygpo::DeviceUpdates::removeList() const
{
    return d->removeList();
}

QVariant mygpo::DeviceUpdates::update() const
{
    return d->update();
}

QList< mygpo::EpisodePtr > mygpo::DeviceUpdates::updateList() const
{
    return d->updateList();
}

qulonglong DeviceUpdates::timestamp() const
{
    return d->timestamp();
}
