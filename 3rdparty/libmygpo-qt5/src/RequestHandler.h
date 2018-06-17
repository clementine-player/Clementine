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

#ifndef LIBMYGPO_QT_REQUESTHANDLER_H_
#define LIBMYGPO_QT_REQUESTHANDLER_H_

#include "mygpo_export.h"

#include <QNetworkAccessManager>
#include <QNetworkReply>

namespace mygpo
{

/**
 * Class for sending HTTP requests and handle the servers response.
 */
class RequestHandler
{

public:

    /**
     * @param username The username that should be used for authentication if required.
     * @param password The password that should be used for authentication if required
     */
    RequestHandler( const QString& username, const QString& password, QNetworkAccessManager* nam );
    RequestHandler( QNetworkAccessManager* nam );

    virtual ~RequestHandler();

    /**
     * Sends a GET request with the given url and receives the servers response.
     * @param response The servers response will be written into this QByteArray
     * @param url The request url (without http://) as QString
     * @return 0 if the request was successful, corresponding ErrorCode if unsuccessful
     */
    QNetworkReply* getRequest( const QString& url );

    /**
     * Sends a GET request with the given url, adds auth Data to the URL and receives the servers response.
     * @param response The servers response will be written into this QByteArray
     * @param url The request url (without http://) as QString
     * @return 0 if the request was successful, corresponding ErrorCode if unsuccessful
     */
    QNetworkReply* authGetRequest( const QString& url );

    /**
     * Sends a POST request with the given url and data, adds auth Data and receives the servers response
     * @param data The data to send to the url
     * @param url The request url (without http://) as QString
     * @return 0 if the request was successful, corresponding ErrorCode if unsuccessful
     */
    QNetworkReply* postRequest( const QByteArray data, const QString& url );

private:
    QString m_username;
    QString m_password;
    QNetworkAccessManager* m_nam;

    void addAuthData( QNetworkRequest& request );
    void addUserAgent( QNetworkRequest& request );
};

}

#endif /* LIBMYGPO_QT_REQUESTHANDLER_H_ */
