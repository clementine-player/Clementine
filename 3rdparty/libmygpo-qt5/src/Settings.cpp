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

#include "Settings_p.h"

#include "qjsonwrapper/Json.h"

using namespace mygpo;

SettingsPrivate::SettingsPrivate( Settings* qq, QNetworkReply* reply ): q( qq ), m_reply( reply ), m_error( QNetworkReply::NoError )
{
    QObject::connect( m_reply, SIGNAL( finished() ), this, SLOT( parseData() ) );
    QObject::connect( m_reply, SIGNAL( error( QNetworkReply::NetworkError ) ), this, SLOT( error( QNetworkReply::NetworkError ) ) );
}

SettingsPrivate::~SettingsPrivate()
{
}

QVariant SettingsPrivate::settings() const
{
    return m_settings;
}

bool SettingsPrivate::parse( const QVariant& data )
{
    m_settings = data;
    return true;
}

bool SettingsPrivate::parse( const QByteArray& data )
{
    bool ok;
    QVariant variant = QJsonWrapper::parseJson( data, &ok );
    if( ok )
    {
        ok = ( parse( variant ) );
    }
    return ok;
}

void SettingsPrivate::parseData()
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


void SettingsPrivate::error( QNetworkReply::NetworkError error )
{
    this->m_error = error;
    emit q->requestError( error );
}

Settings::Settings( QNetworkReply* reply, QObject* parent ): QObject( parent ), d( new SettingsPrivate( this, reply ) )
{

}

Settings::~Settings()
{
    delete d;
}

QVariant Settings::settings() const
{
    return d->settings();
}
