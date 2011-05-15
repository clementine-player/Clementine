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

#ifndef SHELLGENERATOR_H
#define SHELLGENERATOR_H

#include "generator.h"
#include "metaqtscript.h"
#include "prigenerator.h"

#define MAX_CLASSES_PER_FILE 30

class ShellGenerator : public Generator
{
    Q_OBJECT

public:
    virtual QString subDirectoryForClass(const AbstractMetaClass *cls) const
    {
        return "generated_cpp/" + cls->package().replace(".", "_") + "/";
    }

    static void writeTypeInfo(QTextStream &s, const AbstractMetaType *type, Option option = NoOption);
    static void writeFunctionSignature(QTextStream &s, const AbstractMetaFunction *meta_function,
                                const AbstractMetaClass *implementor = 0,
                                const QString &name_prefix = QString(),
                                Option option = NoOption,
                                const QString &classname_prefix = QString(),
                                const QStringList &extra_arguments = QStringList(),
                                int numArguments = -1);
    static void writeFunctionArguments(QTextStream &s, const AbstractMetaClass* owner, const AbstractMetaArgumentList &arguments,
                                Option option = NoOption,
                                int numArguments = -1);

    bool shouldGenerate(const AbstractMetaClass *meta_class) const;

    static QString shellClassName(const AbstractMetaClass *meta_class) {
      return "PythonQtShell_" + meta_class->name();
    }
    static QString wrapperClassName(const AbstractMetaClass *meta_class) {
      return "PythonQtWrapper_" + meta_class->name();
    }
    static QString promoterClassName(const AbstractMetaClass *meta_class) {
      return "PythonQtPublicPromoter_" + meta_class->name();
    }

    static AbstractMetaFunctionList getFunctionsToWrap(const AbstractMetaClass* cls);
    static AbstractMetaFunctionList getVirtualFunctionsForShell(const AbstractMetaClass* cls);
    static AbstractMetaFunctionList getProtectedFunctionsThatNeedPromotion(const AbstractMetaClass* cls);

    // PythonQt builtins..., dont put them in pri files and dont register them, but generate the code
    static bool isBuiltIn(const QString& name);

    static bool isSpecialStreamingOperator(const AbstractMetaFunction *fun);
    
    static void writeInclude(QTextStream &stream, const Include &inc);
  
 protected:
    PriGenerator *priGenerator;

};


#endif // SHELLGENERATOR_H
