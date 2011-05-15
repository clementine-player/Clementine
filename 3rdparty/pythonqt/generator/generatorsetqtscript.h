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

#ifndef GENERATOR_SET_QT_SCRIPT_H
#define GENERATOR_SET_QT_SCRIPT_H

#include "generatorset.h"
#include "metaqtscriptbuilder.h"

class GeneratorSetQtScript : public GeneratorSet
{
    Q_OBJECT

public:
    GeneratorSetQtScript();

    QString usage();
    bool readParameters(const QMap<QString, QString> args);

    void buildModel(const QString pp_file);
    void dumpObjectTree();

    QString generate(                                       );

private:
    MetaQtScriptBuilder builder;
  
};

#endif // GENERATOR_SET_QT_SCRIPT_H
