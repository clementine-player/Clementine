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

#ifndef ADDREMOVERESULT_PRIVATE_H
#define ADDREMOVERESULT_PRIVATE_H

#include "AddRemoveResult.h"

#include <QUrl>

namespace mygpo
{

class AddRemoveResultPrivate : public QObject
{
    Q_OBJECT
public:
    AddRemoveResultPrivate( AddRemoveResult* qq, QNetworkReply* reply );
    virtual ~AddRemoveResultPrivate( );
    QVariant updateUrls() const;
    qulonglong timestamp() const;
    QList<QPair<QUrl, QUrl> > updateUrlsList() const;
private:
    AddRemoveResult* const q;
    qulonglong m_timestamp;
    QVariant m_updateUrls;

    QNetworkReply* m_reply;
    QNetworkReply::NetworkError m_error;

    bool parse( const QVariant& data );
    bool parse( const QByteArray& data );
private slots:
    void parseData();
    void error( QNetworkReply::NetworkError error );
};

}

#endif //ADDREMOVERESULT_PRIVATE_H