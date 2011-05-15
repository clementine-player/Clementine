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

#ifndef ABSTRACTMETABUILDER_H
#define ABSTRACTMETABUILDER_H

#include "codemodel.h"
#include "abstractmetalang.h"
#include "typesystem.h"
#include "typeparser.h"

#include <QtCore/QSet>

class AbstractMetaBuilder
{
public:
    enum RejectReason {
        NotInTypeSystem,
        GenerationDisabled,
        RedefinedToNotClass,
        UnmatchedArgumentType,
        UnmatchedReturnType,
        NoReason
    };

    AbstractMetaBuilder();
    virtual ~AbstractMetaBuilder() {};

    AbstractMetaClassList classes() const { return m_meta_classes; }
    AbstractMetaClassList classesTopologicalSorted() const;

    FileModelItem model() const { return m_dom; }
    void setModel(FileModelItem item) { m_dom = item; }


    ScopeModelItem popScope() { return m_scopes.takeLast(); }
    void pushScope(ScopeModelItem item) { m_scopes << item; }
    ScopeModelItem currentScope() const { return m_scopes.last(); }

    QString fileName() const { return m_file_name; }
    void setFileName(const QString &fileName) { m_file_name = fileName; }

    void dumpLog();

    bool build();

    void figureOutEnumValuesForClass(AbstractMetaClass *meta_class, QSet<AbstractMetaClass *> *classes);
    int figureOutEnumValue(const QString &name, int value, AbstractMetaEnum *meta_enum, AbstractMetaFunction *meta_function = 0);
    void figureOutEnumValues();
    void figureOutDefaultEnumArguments();

    void addAbstractMetaClass(AbstractMetaClass *cls);
    AbstractMetaClass *traverseTypeAlias(TypeAliasModelItem item);
    AbstractMetaClass *traverseClass(ClassModelItem item);
    bool setupInheritance(AbstractMetaClass *meta_class);
    AbstractMetaClass *traverseNamespace(NamespaceModelItem item);
    AbstractMetaEnum *traverseEnum(EnumModelItem item, AbstractMetaClass *enclosing, const QSet<QString> &enumsDeclarations);
    void traverseEnums(ScopeModelItem item, AbstractMetaClass *parent, const QStringList &enumsDeclarations);
    void traverseFunctions(ScopeModelItem item, AbstractMetaClass *parent);
    void traverseFields(ScopeModelItem item, AbstractMetaClass *parent);
    void traverseStreamOperator(FunctionModelItem function_item);
    void traverseCompareOperator(FunctionModelItem item);
    void traverseBinaryArithmeticOperator(FunctionModelItem item);

    AbstractMetaFunction *traverseFunction(FunctionModelItem function);
    AbstractMetaField *traverseField(VariableModelItem field, const AbstractMetaClass *cls);
    void checkFunctionModifications();
    void registerHashFunction(FunctionModelItem function_item);
    void registerToStringCapability(FunctionModelItem function_item);

    void parseQ_Property(AbstractMetaClass *meta_class, const QStringList &declarations);
    void setupEquals(AbstractMetaClass *meta_class);
    void setupComparable(AbstractMetaClass *meta_class);
    void setupClonable(AbstractMetaClass *cls);
    void setupFunctionDefaults(AbstractMetaFunction *meta_function, AbstractMetaClass *meta_class);

    QString translateDefaultValue(ArgumentModelItem item, AbstractMetaType *type,
                                               AbstractMetaFunction *fnc, AbstractMetaClass *,
                                               int argument_index);
    AbstractMetaType *translateType(const TypeInfo &type, bool *ok, bool resolveType = true, bool resolveScope = true);

    void decideUsagePattern(AbstractMetaType *type);

    bool inheritTemplate(AbstractMetaClass *subclass,
                         const AbstractMetaClass *template_class,
                         const TypeParser::Info &info);
    AbstractMetaType *inheritTemplateType(const QList<AbstractMetaType *> &template_types, AbstractMetaType *meta_type, bool *ok = 0);

    bool isQObject(const QString &qualified_name);
    bool isEnum(const QStringList &qualified_name);

    void fixQObjectForScope  (TypeDatabase *types, 
			      NamespaceModelItem item);

    // QtScript
    QSet<QString> qtMetaTypeDeclaredTypeNames() const
        { return m_qmetatype_declared_typenames; }

protected:
    AbstractMetaClass *argumentToClass(ArgumentModelItem);

    virtual AbstractMetaClass *createMetaClass() = 0;
    virtual AbstractMetaEnum *createMetaEnum() = 0;
    virtual AbstractMetaEnumValue *createMetaEnumValue() = 0;
    virtual AbstractMetaField *createMetaField() = 0;
    virtual AbstractMetaFunction *createMetaFunction() = 0;
    virtual AbstractMetaArgument *createMetaArgument() = 0;
    virtual AbstractMetaType *createMetaType() = 0;

private:
    void sortLists();

    QString m_file_name;

    AbstractMetaClassList m_meta_classes;
    AbstractMetaClassList m_templates;
    FileModelItem m_dom;
    
    QSet<const TypeEntry *> m_used_types;

    QMap<QString, RejectReason> m_rejected_classes;
    QMap<QString, RejectReason> m_rejected_enums;
    QMap<QString, RejectReason> m_rejected_functions;
    QMap<QString, RejectReason> m_rejected_fields;

    QList<AbstractMetaEnum *> m_enums;

    QList<QPair<AbstractMetaArgument *, AbstractMetaFunction *> > m_enum_default_arguments;

    QHash<QString, AbstractMetaEnumValue *> m_enum_values;

    AbstractMetaClass *m_current_class;
    QList<ScopeModelItem> m_scopes;
    QString m_namespace_prefix;

    QSet<AbstractMetaClass *> m_setup_inheritance_done;

    // QtScript
    QSet<QString> m_qmetatype_declared_typenames;
};

#endif // ABSTRACTMETBUILDER_H
