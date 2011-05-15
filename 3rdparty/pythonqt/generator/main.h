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

#ifndef MAIN_H
#define MAIN_H

#include "pp.h"

#include <QFile>
#include <QDir>

struct Preprocess
{
    static bool preprocess(const QString &sourceFile, const QString &targetFile, const QString &commandLineIncludes = QString())
    {
        rpp::pp_environment env;
        rpp::pp preprocess(env);

        rpp::pp_null_output_iterator null_out;

        const char *ppconfig = ":/trolltech/generator/parser/rpp/pp-qt-configuration";

        QFile file(ppconfig);
        if (!file.open(QFile::ReadOnly)) {
            fprintf(stderr, "Preprocessor configuration file not found '%s'\n", ppconfig);
            return false;
        }

        QByteArray ba = file.readAll();
        file.close();
        preprocess.operator() (ba.constData(), ba.constData() + ba.size(), null_out);

        QStringList includes;
        includes << QString(".");

#if defined(Q_OS_WIN32)
        char *path_splitter = ";";
#else
        const char *path_splitter = ":";
#endif

        // Environment INCLUDE
        QString includePath = getenv("INCLUDE");
        if (!includePath.isEmpty())
            includes += includePath.split(path_splitter);        

        // Includes from the command line
        if (!commandLineIncludes.isEmpty())
            includes += commandLineIncludes.split(path_splitter);        

        // Include Qt
        QString qtdir = getenv ("QTDIR");
        if (qtdir.isEmpty()) {
#if defined(Q_OS_MAC)
            qWarning("QTDIR environment variable not set. Assuming standard binary install using frameworks.");
            QString frameworkDir = "/Library/Frameworks";
            includes << (frameworkDir + "/QtXml.framework/Headers");
            includes << (frameworkDir + "/QtNetwork.framework/Headers");
            includes << (frameworkDir + "/QtCore.framework/Headers");
            includes << (frameworkDir + "/QtGui.framework/Headers");
            includes << (frameworkDir + "/QtOpenGL.framework/Headers");
            includes << frameworkDir;
#else
            qWarning("QTDIR environment variable not set. This may cause problems with finding the necessary include files.");
#endif
        } else {
            qtdir += "/include";
            includes << (qtdir + "/QtXml");
            includes << (qtdir + "/QtNetwork");
            includes << (qtdir + "/QtCore");
            includes << (qtdir + "/QtGui");
            includes << (qtdir + "/QtOpenGL");
            includes << qtdir;
        }

        foreach (QString include, includes)
            preprocess.push_include_path(QDir::convertSeparators(include).toStdString());        

        QString currentDir = QDir::current().absolutePath();
        QFileInfo sourceInfo(sourceFile);
        QDir::setCurrent(sourceInfo.absolutePath());        

        std::string result;
        result.reserve (20 * 1024); // 20K

        result += "# 1 \"builtins\"\n";
        result += "# 1 \"";
        result += sourceFile.toStdString();
        result += "\"\n";

        preprocess.file (sourceInfo.fileName().toStdString(),
                         rpp::pp_output_iterator<std::string> (result));

        QDir::setCurrent(currentDir);

        QFile f(targetFile);
        if (!f.open(QIODevice::WriteOnly | QIODevice::Text)) {
            fprintf(stderr, "Failed to write preprocessed file: %s\n", qPrintable(targetFile));
        }
        f.write(result.c_str(), result.length());

        return true;
    }
};

#endif // MAIN_H
