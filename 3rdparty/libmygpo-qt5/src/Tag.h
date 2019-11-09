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

#ifndef LIBMYGPO_QT_TAG_H
#define LIBMYGPO_QT_TAG_H

#include "mygpo_export.h"

#include <QSharedPointer>
#include <QVariant>

namespace mygpo
{

class TagPrivate;

class MYGPO_EXPORT Tag : public QObject
{
    Q_OBJECT
    Q_PROPERTY( QString tag READ tag CONSTANT )
    Q_PROPERTY( uint usage READ usage CONSTANT )

public:
    Tag( const QVariant& variant, QObject* parent = 0 );
    virtual ~Tag();
    QString tag() const;
    uint usage() const;
private:
    Q_DISABLE_COPY( Tag )
    TagPrivate* const d;
    friend class TagPrivate;
};

typedef QSharedPointer<Tag> TagPtr;

}

Q_DECLARE_METATYPE(mygpo::TagPtr)

#endif // LIBMYGPO_QT_TAG_H
