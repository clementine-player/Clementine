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

#ifndef LIBMYGPO_QT_SETTINGS_H
#define LIBMYGPO_QT_SETTINGS_H

#include <QSharedPointer>
#include <QMap>
#include <QNetworkReply>

class QVariant;

#include "mygpo_export.h"

namespace mygpo
{

class SettingsPrivate;

class MYGPO_EXPORT Settings : public QObject
{

    Q_OBJECT
    Q_PROPERTY( QVariant settings READ settings CONSTANT )

public:
    Settings( QNetworkReply* reply, QObject* parent = 0 );
    virtual ~Settings();
    QVariant settings() const;
private:
    Q_DISABLE_COPY( Settings )
    SettingsPrivate* d;
    friend class SettingsPrivate;
signals:
    /**Gets emitted when the data is ready to read*/
    void finished();
    /**Gets emitted when an parse error ocurred*/
    void parseError();
    /**Gets emitted when an request error ocurred*/
    void requestError( QNetworkReply::NetworkError error );
};

typedef QSharedPointer<Settings> SettingsPtr;

}

#endif // LIBMYGPO_QT_SETTINGS_H
