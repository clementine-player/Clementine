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

#include "shellgenerator.h"
#include "reporthandler.h"

#include "metaqtscript.h"

bool ShellGenerator::shouldGenerate(const AbstractMetaClass *meta_class) const
{
    uint cg = meta_class->typeEntry()->codeGeneration();
    if (meta_class->name().startsWith("QtScript")) return false;
    if (meta_class->name().startsWith("QFuture")) return false;
    if (meta_class->name().startsWith("Global")) return false;
    if (meta_class->name().startsWith("QStyleOptionComplex")) return false;
    if (meta_class->name().startsWith("QTextLayout")) return false;
    if (meta_class->name().startsWith("QPersistentModelIndex")) return false;
    return ((cg & TypeEntry::GenerateCode) != 0);
}

void ShellGenerator::writeTypeInfo(QTextStream &s, const AbstractMetaType *type, Option options)
{
    if ((options & OriginalTypeDescription) && !type->originalTypeDescription().isEmpty()) {
        s << type->originalTypeDescription();
        return;
    }

    if (type->isArray()) {
        writeTypeInfo(s, type->arrayElementType(), options);
        if (options & ArrayAsPointer) {
            s << "*";
        } else {
            s << "[" << type->arrayElementCount() << "]";
        }
        return;
    }

    const TypeEntry *te = type->typeEntry();

    if (type->isConstant() && !(options & ExcludeConst))
        s << "const ";

    if ((options & EnumAsInts) && (te->isEnum() || te->isFlags())) {
        s << "int";
    } else if (te->isFlags()) {
        s << ((FlagsTypeEntry *) te)->originalName();
    } else {
        s << fixCppTypeName(te->qualifiedCppName());
    }

    if (type->instantiations().size() > 0
        && (!type->isContainer() 
            || (static_cast<const ContainerTypeEntry *>(te))->type() != ContainerTypeEntry::StringListContainer)) {
        s << '<';
        QList<AbstractMetaType *> args = type->instantiations();
        bool nested_template = false;
        for (int i=0; i<args.size(); ++i) {
            if (i != 0)
                s << ", ";
            nested_template |= args.at(i)->isContainer();
            writeTypeInfo(s, args.at(i));
        }
        if (nested_template)
            s << ' ';
        s << '>';
    }

    s << QString(type->indirections(), '*');

    if (type->isReference() && !(options & ExcludeReference) && !(options & ConvertReferenceToPtr))
        s << "&";
  
    if (type->isReference() && (options & ConvertReferenceToPtr)) {
      s << "*";
    }
    

    if (!(options & SkipName))
        s << ' ';
}


void ShellGenerator::writeFunctionArguments(QTextStream &s, const AbstractMetaClass* owner,
                                          const AbstractMetaArgumentList &arguments,
                                          Option option,
                                          int numArguments)
{
    if (numArguments < 0) numArguments = arguments.size();

    for (int i=0; i<numArguments; ++i) {
        if (i != 0)
            s << ", ";
        AbstractMetaArgument *arg = arguments.at(i);
        writeTypeInfo(s, arg->type(), option);
        if (!(option & SkipName))
            s << " " << arg->argumentName();
        if ((option & IncludeDefaultExpression) && !arg->originalDefaultValueExpression().isEmpty()) {
            s << " = "; 

          QString expr = arg->originalDefaultValueExpression();
          if (expr != "0") {
            QString qualifier;
            if (arg->type()->typeEntry()->isEnum() && expr.indexOf("::") < 0) {
              qualifier =  ((EnumTypeEntry *)arg->type()->typeEntry())->qualifier();
            } else if (arg->type()->typeEntry()->isFlags() && expr.indexOf("::") < 0) {
              qualifier = ((FlagsTypeEntry *)arg->type()->typeEntry())->originator()->qualifier();
            }
            if (!qualifier.isEmpty()) {
              s << qualifier << "::";
            }
          }
          if (expr.contains("defaultConnection")) {
            expr.replace("defaultConnection","QSqlDatabase::defaultConnection");
          }
          if (expr == "MediaSource()") {
            expr = "Phonon::MediaSource()";
          }
          s << expr; 
        }
    }
}

/*!
 * Writes the function \a meta_function signature to the textstream \a s.
 *
 * The \a name_prefix can be used to give the function name a prefix,
 * like "__public_" or "__override_" and \a classname_prefix can
 * be used to give the class name a prefix.
 *
 * The \a option flags can be used to tweak various parameters, such as
 * showing static, original vs renamed name, underscores for space etc.
 *
 * The \a extra_arguments list is a list of extra arguments on the
 * form "bool static_call".
 */

void ShellGenerator::writeFunctionSignature(QTextStream &s,
                                          const AbstractMetaFunction *meta_function,
                                          const AbstractMetaClass *implementor,
                                          const QString &name_prefix,
                                          Option option,
                                          const QString &classname_prefix,
                                          const QStringList &extra_arguments,
                                          int numArguments)
{
// ### remove the implementor
    AbstractMetaType *function_type = meta_function->type();


    if ((option & SkipReturnType) == 0) {
        if (function_type) {
            writeTypeInfo(s, function_type, option);
            s << " ";
        } else if (!meta_function->isConstructor()) {
            s << "void ";
        }
    }

    if (implementor) {
        if (classname_prefix.isEmpty())
            s << wrapperClassName(implementor) << "::";
        else
            s << classname_prefix << implementor->name() << "::";
    }


    QString function_name;
    if (option & OriginalName)
        function_name = meta_function->originalName();
    else
        function_name = meta_function->name();

    if (option & UnderscoreSpaces)
        function_name = function_name.replace(' ', '_');

    if (meta_function->isConstructor())
        function_name = meta_function->ownerClass()->name();

    if (meta_function->isStatic() && (option & ShowStatic)) {
      function_name = "static_" + meta_function->ownerClass()->name() + "_" + function_name;
    }
  
    if (function_name.startsWith("operator")) {
      function_name = meta_function->name();
    }

  if (meta_function->attributes() & AbstractMetaAttributes::SetterFunction)
    s << "py_set_";
  else if (meta_function->attributes() & AbstractMetaAttributes::GetterFunction)
    s << "py_get_";

  s << name_prefix << function_name;

    s << "(";

  if ((option & FirstArgIsWrappedObject) && meta_function->ownerClass() && !meta_function->isConstructor() && !meta_function->isStatic()) {
    s << meta_function->ownerClass()->qualifiedCppName() << "* theWrappedObject"; 
    if (meta_function->arguments().size() != 0) {
      s << ", ";
    }
  }
  
   writeFunctionArguments(s, meta_function->ownerClass(), meta_function->arguments(), Option(option & Option(~ConvertReferenceToPtr)), numArguments);

    // The extra arguments...
    for (int i=0; i<extra_arguments.size(); ++i) {
        if (i > 0 || meta_function->arguments().size() != 0)
            s << ", ";
        s << extra_arguments.at(i);
    }

    s << ")";
    if (meta_function->isConstant())
        s << " const";
}

bool function_sorter(AbstractMetaFunction *a, AbstractMetaFunction *b);

AbstractMetaFunctionList ShellGenerator::getFunctionsToWrap(const AbstractMetaClass* meta_class)
{
  AbstractMetaFunctionList functions = meta_class->queryFunctions( 
    AbstractMetaClass::NormalFunctions | AbstractMetaClass::WasPublic
    | AbstractMetaClass::NotRemovedFromTargetLang | AbstractMetaClass::ClassImplements
    );
  AbstractMetaFunctionList functions2 = meta_class->queryFunctions( 
    AbstractMetaClass::VirtualFunctions | AbstractMetaClass::WasVisible
    | AbstractMetaClass::NotRemovedFromTargetLang | AbstractMetaClass::ClassImplements
    );
  QSet<AbstractMetaFunction*> set1 = QSet<AbstractMetaFunction*>::fromList(functions);
  foreach(AbstractMetaFunction* func, functions2) {
    set1.insert(func);
  }

  AbstractMetaFunctionList resultFunctions;

  foreach(AbstractMetaFunction* func, set1.toList()) {
    if (!func->isAbstract() && func->implementingClass()==meta_class) {
      resultFunctions << func;
    }
  }
  qSort(resultFunctions.begin(), resultFunctions.end(), function_sorter);
  return resultFunctions;
}

AbstractMetaFunctionList ShellGenerator::getVirtualFunctionsForShell(const AbstractMetaClass* meta_class)
{
  AbstractMetaFunctionList functions = meta_class->queryFunctions( 
    AbstractMetaClass::VirtualFunctions | AbstractMetaClass::WasVisible
//    | AbstractMetaClass::NotRemovedFromTargetLang
    );
  qSort(functions.begin(), functions.end(), function_sorter);
  return functions;
}

AbstractMetaFunctionList ShellGenerator::getProtectedFunctionsThatNeedPromotion(const AbstractMetaClass* meta_class)
{
  AbstractMetaFunctionList functions; 
  AbstractMetaFunctionList functions1 = getFunctionsToWrap(meta_class); 
  foreach(AbstractMetaFunction* func, functions1) {
    if (!func->isPublic() || func->isVirtual()) {
      functions << func;
    }
  }
  qSort(functions.begin(), functions.end(), function_sorter);
  return functions;
}

/*!
Writes the include defined by \a inc to \a stream.
*/
void ShellGenerator::writeInclude(QTextStream &stream, const Include &inc)
{
  if (inc.name.isEmpty())
    return;
  if (inc.type == Include::TargetLangImport)
    return;
  stream << "#include ";
  if (inc.type == Include::IncludePath)
    stream << "<";
  else
    stream << "\"";
  stream << inc.name;
  if (inc.type == Include::IncludePath)
    stream << ">";
  else
    stream << "\"";
  stream << endl;
}

/*!
Returns true if the given function \a fun is operator>>() or
operator<<() that streams from/to a Q{Data,Text}Stream, false
otherwise.
*/
bool ShellGenerator::isSpecialStreamingOperator(const AbstractMetaFunction *fun)
{
  return ((fun->functionType() == AbstractMetaFunction::GlobalScopeFunction)
    && (fun->arguments().size() == 1)
    && (((fun->originalName() == "operator>>") && (fun->modifiedName() == "readFrom"))
    || ((fun->originalName() == "operator<<") && (fun->modifiedName() == "writeTo"))));
}

bool ShellGenerator::isBuiltIn(const QString& name) {

  static QSet<QString> builtIn;
  if (builtIn.isEmpty()) {
    builtIn.insert("Qt");
    builtIn.insert("QFont");
    builtIn.insert("QPixmap");
    builtIn.insert("QBrush");
    builtIn.insert("QBitArray");
    builtIn.insert("QByteArray");
    builtIn.insert("QPalette");
    builtIn.insert("QPen");
    builtIn.insert("QIcon");
    builtIn.insert("QImage");
    builtIn.insert("QPolygon");
    builtIn.insert("QRegion");
    builtIn.insert("QBitmap");
    builtIn.insert("QCursor");
    builtIn.insert("QColor");
    builtIn.insert("QSizePolicy");
    builtIn.insert("QKeySequence");
    builtIn.insert("QTextLength");
    builtIn.insert("QTextFormat");
    builtIn.insert("QMatrix");
    builtIn.insert("QDate");
    builtIn.insert("QTime");
    builtIn.insert("QDateTime");
    builtIn.insert("QUrl");
    builtIn.insert("QLocale");
    builtIn.insert("QRect");
    builtIn.insert("QRectF");
    builtIn.insert("QSize");
    builtIn.insert("QSizeF");
    builtIn.insert("QLine");
    builtIn.insert("QLineF");
    builtIn.insert("QPoint");
    builtIn.insert("QPointF");
    builtIn.insert("QRegExp");
  }
  return builtIn.contains(name);
}

