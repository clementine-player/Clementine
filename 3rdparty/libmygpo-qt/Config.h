/***************************************************************************
* This file is part of libmygpo-qt                                         *
* Copyright (c) 2012 - 2013 Stefan Derkits <stefan@derkits.at>             *
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

#ifndef LIBMYGPO_QT_CONFIG_H
#define LIBMYGPO_QT_CONFIG_H

#include "mygpo_export.h"

#include <QObject>
#include <QUrl>

namespace mygpo
{

class ConfigPrivate;
//TODO: More config entries (mygpo-feedservice baseurl), let class inherit from QObject, set everything as a Property
class MYGPO_EXPORT Config
{

public:
    static Config* instance();

    int majorVersion() const;
    int minorVersion() const;
    int patchVersion() const;

    QString version() const;

    QUrl mygpoBaseUrl() const;
    void setMygpoBaseUrl( const QUrl& mygpoBaseUrl );

    QString userAgent() const;

    QString userAgentPrefix() const;
    void setUserAgentPrefix( const QString& prefix );
private:
    Config();
    ~Config();

    static Config* s_instance;
  
    ConfigPrivate* const d;
};

}

#endif // LIBMYGPO_QT_CONFIG_H
