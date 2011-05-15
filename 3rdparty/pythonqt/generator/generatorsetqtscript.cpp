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

#include "generatorsetqtscript.h"
#include "reporthandler.h"
#include "shellheadergenerator.h"
#include "shellimplgenerator.h"

GeneratorSet *GeneratorSet::getInstance() {
    return new GeneratorSetQtScript();
}

GeneratorSetQtScript::GeneratorSetQtScript() 
{}

QString GeneratorSetQtScript::usage() {
    QString usage =
        "QtScript:\n" 
        "  --nothing-to-report-yet                   \n";

    return usage;
}

bool GeneratorSetQtScript::readParameters(const QMap<QString, QString> args) {
    return GeneratorSet::readParameters(args);
}

void GeneratorSetQtScript::buildModel(const QString pp_file) {
    // Building the code inforamation...
    ReportHandler::setContext("MetaJavaBuilder");
    builder.setFileName(pp_file);
    builder.build();
}

void GeneratorSetQtScript::dumpObjectTree() {
 
}

QString GeneratorSetQtScript::generate() {
    AbstractMetaClassList classes = builder.classesTopologicalSorted();
    QSet<QString> declaredTypeNames = builder.qtMetaTypeDeclaredTypeNames();

    PriGenerator priGenerator;
    priGenerator.setOutputDirectory(outDir);

    SetupGenerator setupGenerator;
    setupGenerator.setOutputDirectory(outDir);
    setupGenerator.setQtMetaTypeDeclaredTypeNames(declaredTypeNames);
    setupGenerator.setClasses(classes);

    ShellImplGenerator shellImplGenerator(&priGenerator);
    shellImplGenerator.setOutputDirectory(outDir);
    shellImplGenerator.setClasses(classes);
    shellImplGenerator.setQtMetaTypeDeclaredTypeNames(declaredTypeNames);
    shellImplGenerator.generate();

    ShellHeaderGenerator shellHeaderGenerator(&priGenerator, &setupGenerator);
    shellHeaderGenerator.setOutputDirectory(outDir);
    shellHeaderGenerator.setClasses(classes);
    shellHeaderGenerator.generate();

    priGenerator.generate();
    setupGenerator.generate();

    return QString("Classes in typesystem: %1\n"
                   "Generated:\n"
                   "  - header....: %4 (%5)\n"
                   "  - impl......: %6 (%7)\n"
                   "  - modules...: %8 (%9)\n"
                   "  - pri.......: %10 (%11)\n"
                   )
        .arg(builder.classes().size())

        .arg(shellHeaderGenerator.numGenerated())
        .arg(shellHeaderGenerator.numGeneratedAndWritten())

        .arg(shellImplGenerator.numGenerated())
        .arg(shellImplGenerator.numGeneratedAndWritten())

        .arg(setupGenerator.numGenerated())
        .arg(setupGenerator.numGeneratedAndWritten())

        .arg(priGenerator.numGenerated())
        .arg(priGenerator.numGeneratedAndWritten());
}
