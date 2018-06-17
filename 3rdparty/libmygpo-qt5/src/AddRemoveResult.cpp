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

#include "AddRemoveResult.h"
#include "AddRemoveResult_p.h"

#include "qjsonwrapper/Json.h"

using namespace mygpo;

AddRemoveResultPrivate::AddRemoveResultPrivate( AddRemoveResult* qq, QNetworkReply* reply ) : q( qq ), m_reply( reply ), m_error( QNetworkReply::NoError )
{
    QObject::connect( m_reply, SIGNAL( finished() ), this, SLOT( parseData() ) );
    QObject::connect( m_reply, SIGNAL( error( QNetworkReply::NetworkError ) ), this, SLOT( error( QNetworkReply::NetworkError ) ) );
}

AddRemoveResultPrivate::~AddRemoveResultPrivate()
{
}


qulonglong AddRemoveResultPrivate::timestamp() const
{
    return m_timestamp;
}

QVariant AddRemoveResultPrivate::updateUrls() const
{
    return m_updateUrls;
}

QList< QPair< QUrl, QUrl > > AddRemoveResultPrivate::updateUrlsList() const
{
    QVariantList updateVarList = updateUrls().toList();
    QList<QPair<QUrl, QUrl > > updateUrls;
    foreach( const QVariant & url, updateVarList )
    {
        QVariantList urlList = url.toList();
        QUrl first = QUrl( urlList.at( 0 ).toString() );
        QUrl second = QUrl( urlList.at( 1 ).toString() );
        updateUrls.append( qMakePair( first, second ) );
    }
    return updateUrls;
}

bool AddRemoveResultPrivate::parse( const QVariant& data )
{
    if( !data.canConvert( QVariant::Map ) )
        return false;
    QVariantMap resultMap = data.toMap();
    QVariant v = resultMap.value( QLatin1String( "timestamp" ) );
    if( !v.canConvert( QVariant::ULongLong ) )
        return false;
    m_timestamp = v.toULongLong();
    m_updateUrls = resultMap.value( QLatin1String( "update_urls" ) );
    return true;
}

bool AddRemoveResultPrivate::parse( const QByteArray& data )
{
    bool ok;
    QVariant variant = QJsonWrapper::parseJson( data, &ok );
    if( ok )
    {
        ok = ( parse( variant ) );
    }
    return ok;
}


void AddRemoveResultPrivate::parseData()
{
    if( m_reply->error() == QNetworkReply::NoError )
    {

        if( parse( m_reply->readAll( ) ) )
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

void AddRemoveResultPrivate::error( QNetworkReply::NetworkError error )
{
    this->m_error = error;
    emit q->requestError( error );
}

AddRemoveResult::AddRemoveResult( QNetworkReply* reply , QObject* parent ) : QObject( parent ), d( new AddRemoveResultPrivate( this, reply ) )
{

}

AddRemoveResult::~AddRemoveResult()
{
    delete d;
}

QVariant AddRemoveResult::updateUrls() const
{
    return d->updateUrls();
}

qulonglong AddRemoveResult::timestamp() const
{
    return d->timestamp();
}

QList<QPair<QUrl, QUrl> > AddRemoveResult::updateUrlsList() const
{
    return d->updateUrlsList();
}
