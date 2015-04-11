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

#ifndef LIBMYGPO_QT_JSONPARSER_H
#define LIBMYGPO_QT_JSONPARSER_H

#include <QByteArray>
#include <QVariant>
#include <QList>
#include <QMap>
#include <QStringList>

#include "EpisodeAction.h"

class QUrl;
class QString;

namespace mygpo
{

class JsonCreator
{

public:
    static QByteArray addRemoveSubsToJSON( const QList<QUrl>& add, const QList<QUrl>& remove );
    static QByteArray saveSettingsToJSON( const QMap<QString, QVariant >& set, const QList<QString>& remove );
    static QByteArray episodeActionListToJSON( const QList<EpisodeActionPtr>& episodeActions );
    static QByteArray renameDeviceStringToJSON( const QString& caption, const QString& type );
    static QByteArray deviceSynchronizationListsToJSON( const QList<QStringList>& synchronize, const QList<QString>& stopSynchronize );
private:
    static QVariantList urlListToQVariantList( const QList<QUrl>& urls );
    static QVariantList stringListToQVariantList( const QList<QString>& strings );
    static QVariantMap stringMapToQVariantMap( const QMap<QString, QString >& stringmap );
    static QVariantMap episodeActionToQVariantMap( const EpisodeActionPtr episodeAction );
};

}

#endif // LIBMYGPO_QT_JSONPARSER_H
