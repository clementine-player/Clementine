/****************************************************************************
**
** Copyright (C) 2008-2009 Nokia Corporation and/or its subsidiary(-ies).
** All rights reserved.
** Contact: Nokia Corporation (qt-info@nokia.com)
**
** This file is part of the Qt Script Generator project on Qt Labs.
**
** $QT_BEGIN_LICENSE:LGPL$
** No Commercial Usage
** This file contains pre-release code and may not be distributed.
** You may use this file in accordance with the terms and conditions
** contained in the Technology Preview License Agreement accompanying
** this package.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU Lesser General Public License version 2.1 requirements
** will be met: http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** In addition, as a special exception, Nokia gives you certain additional
** rights.  These rights are described in the Nokia Qt LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
** If you have questions regarding the use of this file, please contact
** Nokia at qt-info@nokia.com.
**
**
**
**
**
**
**
**
** $QT_END_LICENSE$
**
****************************************************************************/

#ifndef TYPEPARSER_H
#define TYPEPARSER_H

#include <QList>
#include <QString>
#include <QStringList>

class TypeParser
{
public:
    struct Info
    {
        Info() : is_reference(false), is_constant(false), is_busted(false), indirections(0) { }
        QStringList qualified_name;
        QStringList arrays;
        QList<Info> template_instantiations;
        uint is_reference : 1;
        uint is_constant : 1;
        uint is_busted : 1;
        uint indirections : 5;

        QString toString() const;
        QString instantiationName() const;
    };

    static Info parse(const QString &str);
};

#endif // TYPEPARSER_H
