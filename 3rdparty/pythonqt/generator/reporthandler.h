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

#ifndef REPORTHANDLER_H
#define REPORTHANDLER_H

#include <QtCore/QString>
#include <QtCore/QSet>

class ReportHandler
{
public:
    enum DebugLevel { NoDebug, SparseDebug, MediumDebug, FullDebug };

    static void setContext(const QString &context) { m_context = context; }

    static DebugLevel debugLevel() { return m_debug_level; }
    static void setDebugLevel(DebugLevel level) { m_debug_level = level; }

    static int warningCount() { return m_warning_count; }

    static int suppressedCount() { return m_suppressed_count; }

    static void warning(const QString &str);

    static void debugSparse(const QString &str) {
        debug(SparseDebug, str);
    }
    static void debugMedium(const QString &str) {
        debug(MediumDebug, str);
    }
    static void debugFull(const QString &str) {
        debug(FullDebug, str);
    }
    static void debug(DebugLevel level, const QString &str);

private:
    static int m_warning_count;
    static int m_suppressed_count;
    static DebugLevel m_debug_level;
    static QString m_context;
    static QSet<QString> m_reported_warnings;
};

#endif // REPORTHANDLER_H
