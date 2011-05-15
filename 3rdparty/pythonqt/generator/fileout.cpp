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

#include "fileout.h"
#include "reporthandler.h"

#include <QFileInfo>
#include <QDir>

bool FileOut::dummy = false;
bool FileOut::diff = false;
bool FileOut::license = false;

#ifdef Q_OS_LINUX
const char* colorDelete = "\033[31m";
const char* colorAdd = "\033[32m";
const char* colorInfo = "\033[36m";
const char* colorReset = "\033[0m";
#else
const char* colorDelete = "";
const char* colorAdd = "";
const char* colorInfo = "";
const char* colorReset = ""; 
#endif

FileOut::FileOut(QString n):
    name(n),
    stream(&tmp),
    isDone(false)
{}

static int* lcsLength(QList<QByteArray> a, QList<QByteArray> b) {
    const int height = a.size() + 1;
    const int width = b.size() + 1;

    int *res = new int[width * height];

    for (int row=0; row<height; row++) {
        res[width * row] = 0;
    }
    for (int col=0; col<width; col++) {
        res[col] = 0;
    }

    for (int row=1; row<height; row++) {
        for (int col=1; col<width; col++) {
            
            if (a[row-1] == b[col-1])
                res[width * row + col] = res[width * (row-1) + col-1] + 1;
            else
                res[width * row + col] = qMax(res[width * row     + col-1],
                                              res[width * (row-1) + col]);
        }
    }
    return res;
}

enum Type {Add, Delete, Unchanged};

struct Unit
{
    Unit(Type type, int pos) :
        type(type),
        start(pos),
        end(pos)
    {}

    Type type;
    int start;
    int end;

    void print(QList<QByteArray> a, QList<QByteArray> b){
        {
            if (type == Unchanged) {
                if ((end - start) > 9) {
                    for (int i = start; i <= start+2; i++)
                        printf("  %s\n", a[i].data());
                    printf("%s=\n= %d more lines\n=%s\n", colorInfo, end - start - 6, colorReset);
                    for (int i = end-2; i <= end; i++) 
                        printf("  %s\n", a[i].data());
                }
                else 
                    for (int i = start; i <= end; i++) 
                        printf("  %s\n", a[i].data());
            }
            else if(type == Add) {
                printf("%s", colorAdd);
                for (int i = start; i <= end; i++){
                    printf("+ %s\n", b[i].data());
                }
                printf("%s", colorReset);
            } 
            else if (type == Delete) {
                printf("%s", colorDelete);
                for (int i = start; i <= end; i++) {
                    printf("- %s\n", a[i].data());
                }
                printf("%s", colorReset);
            }    
        }
    }
};

static QList<Unit*> *unitAppend(QList<Unit*> *res, Type type, int pos)
{
    if (res == 0) {
        res = new QList<Unit*>;
        res->append(new Unit(type, pos));
        return res;
    }

    Unit *last = res->last();
    if (last->type == type) {
        last->end = pos;
    } else {
        res->append(new Unit(type, pos));
    }
    return res;
}

static QList<Unit*> *diffHelper(int *lcs, QList<QByteArray> a, QList<QByteArray> b, int row, int col) {
    if (row>0 && col>0 && (a[row-1] == b[col-1])) {
        return unitAppend(diffHelper(lcs, a, b, row-1, col-1), Unchanged, row-1);
    }
    else {
        int width = b.size()+1;
        if ((col > 0) && ((row==0) || 
                          lcs[width * row + col-1] >= lcs[width * (row-1) + col]))
            {
                return unitAppend(diffHelper(lcs, a, b, row, col-1), Add, col-1);
            }
        else if((row > 0) && ((col==0) ||
                              lcs[width * row + col-1] < lcs[width * (row-1) + col])){ 
            return unitAppend(diffHelper(lcs, a, b, row-1, col), Delete, row-1);;
        }
    }
    delete lcs;
    return 0;
}

static void diff(QList<QByteArray> a, QList<QByteArray> b) {
    QList<Unit*> *res = diffHelper(lcsLength(a, b), a, b, a.size(), b.size());
    for (int i=0; i < res->size(); i++) {
        Unit *unit = res->at(i);
        unit->print(a, b);
        delete(unit);
    }
    delete(res);
}


bool FileOut::done() {
    Q_ASSERT( !isDone );
    isDone = true;
    bool fileEqual = false;
    QFile fileRead(name);
    QFileInfo info(fileRead);
    stream.flush();
    QByteArray original;
    if (info.exists() && (diff || (info.size() == tmp.size()))) {
        if ( !fileRead.open(QIODevice::ReadOnly) ) {
            ReportHandler::warning(QString("failed to open file '%1' for reading")
                                   .arg(fileRead.fileName()));
            return false;
        }
        
        original = fileRead.readAll();
        fileRead.close();
        fileEqual = (original == tmp);
    }
    
    if( !fileEqual ) {
        if( !FileOut::dummy ) {
            QDir dir(info.absolutePath());
            if (!dir.mkpath(dir.absolutePath())) {
                ReportHandler::warning(QString("unable to create directory '%1'")
                                       .arg(dir.absolutePath()));
                return false;
            }
            
            QFile fileWrite(name);
            if (!fileWrite.open(QIODevice::WriteOnly)) {
                ReportHandler::warning(QString("failed to open file '%1' for writing")
                                       .arg(fileWrite.fileName()));
                return false;
            }
            stream.setDevice(&fileWrite);
            stream << tmp;
        }
        if (diff) {
            printf("%sFile: %s%s\n", colorInfo, qPrintable(name), colorReset);
         
            ::diff(original.split('\n'), tmp.split('\n'));
            
            printf("\n");
        }
        return true;
    }
    return false;
}
