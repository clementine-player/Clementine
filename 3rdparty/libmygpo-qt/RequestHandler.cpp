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

#include <QAuthenticator>
#include <QCoreApplication>

#include "RequestHandler.h"
#include "Config.h"

using namespace mygpo;

RequestHandler::RequestHandler( const QString& username, const QString& password, QNetworkAccessManager* nam ) : m_username( username ), m_password( password ), m_nam( nam )
{
}

RequestHandler::RequestHandler( QNetworkAccessManager* nam ) : m_username(), m_password(), m_nam( nam )
{
}

RequestHandler::~RequestHandler()
{
}

QNetworkReply* RequestHandler::getRequest( const QString& url )
{
    QUrl reqUrl( url );
    QNetworkRequest request( reqUrl );
    addUserAgent( request );
    QNetworkReply* reply = m_nam->get( request );
    return reply;
}

QNetworkReply* RequestHandler::authGetRequest( const QString& url )
{
    QNetworkRequest request( url );
    addUserAgent( request );
    addAuthData( request );
    QNetworkReply* reply = m_nam->get( request );
    return reply;
}


QNetworkReply* RequestHandler::postRequest( const QByteArray data, const QString& url )
{
    QNetworkRequest request( url );
    addUserAgent( request );
    addAuthData( request );
    request.setHeader(QNetworkRequest::ContentTypeHeader, QLatin1String("application/json"));
    QNetworkReply* reply = m_nam->post( request, data );
    return reply;
}

void RequestHandler::addAuthData( QNetworkRequest& request )
{
    QByteArray headerData = "Basic " + QString(m_username + QLatin1String(":") + m_password).toLocal8Bit().toBase64();
    request.setRawHeader("Authorization", headerData );
}

void RequestHandler::addUserAgent( QNetworkRequest &request )
{
    request.setRawHeader("User-Agent", Config::instance()->userAgent().toLatin1() );
}
