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

#include "preprocessor.h"

#include <string>

// register callback for include hooks
static void includeFileHook(const std::string &, const std::string &, FILE *);

#define PP_HOOK_ON_FILE_INCLUDED(A, B, C) includeFileHook(A, B, C)
#include "pp.h"

using namespace rpp;

#include <QtCore/QtCore>

class PreprocessorPrivate
{
public:
    QByteArray result;
    pp_environment env;
    QStringList includePaths;

    void initPP(pp &proc)
    {
        foreach(QString path, includePaths)
            proc.push_include_path(path.toStdString());
    }
};

QHash<QString, QStringList> includedFiles;

void includeFileHook(const std::string &fileName, const std::string &filePath, FILE *)
{
    includedFiles[QString::fromStdString(fileName)].append(QString::fromStdString(filePath));
}

Preprocessor::Preprocessor()
{
    d = new PreprocessorPrivate;
    includedFiles.clear();
}

Preprocessor::~Preprocessor()
{
    delete d;
}

void Preprocessor::processFile(const QString &fileName)
{
    pp proc(d->env);
    d->initPP(proc);

    d->result.reserve(d->result.size() + 20 * 1024);

    d->result += "# 1 \"" + fileName.toLatin1() + "\"\n"; // ### REMOVE ME
    proc.file(fileName.toLocal8Bit().constData(), std::back_inserter(d->result));
}

void Preprocessor::processString(const QByteArray &str)
{
    pp proc(d->env);
    d->initPP(proc);

    proc(str.begin(), str.end(), std::back_inserter(d->result));
}

QByteArray Preprocessor::result() const
{
    return d->result;
}

void Preprocessor::addIncludePaths(const QStringList &includePaths)
{
    d->includePaths += includePaths;
}

QStringList Preprocessor::macroNames() const
{
    QStringList macros;

    pp_environment::const_iterator it = d->env.first_macro();
    while (it != d->env.last_macro()) {
        const pp_macro *m = *it;
        macros += QString::fromLatin1(m->name->begin(), m->name->size());
        ++it;
    }

    return macros;
}

QList<Preprocessor::MacroItem> Preprocessor::macros() const
{
    QList<MacroItem> items;

    pp_environment::const_iterator it = d->env.first_macro();
    while (it != d->env.last_macro()) {
        const pp_macro *m = *it;
        MacroItem item;
        item.name = QString::fromLatin1(m->name->begin(), m->name->size());
        item.definition = QString::fromLatin1(m->definition->begin(),
                                              m->definition->size());
        for (size_t i = 0; i < m->formals.size(); ++i) {
            item.parameters += QString::fromLatin1(m->formals[i]->begin(),
                    m->formals[i]->size());
        }
        item.isFunctionLike = m->function_like;

#ifdef PP_WITH_MACRO_POSITION
        item.fileName = QString::fromLatin1(m->file->begin(), m->file->size());
#endif
        items += item;

        ++it;
    }

    return items;
}

/*
int main()
{
    Preprocessor pp;

    QStringList paths;
    paths << "/usr/include";
    pp.addIncludePaths(paths);

    pp.processFile("pp-configuration");
    pp.processFile("/usr/include/stdio.h");

    qDebug() << pp.result();

    return 0;
}
*/

