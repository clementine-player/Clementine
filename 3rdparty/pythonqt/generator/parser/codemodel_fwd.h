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


#ifndef CODEMODEL_FWD_H
#define CODEMODEL_FWD_H

#include <codemodel_pointer.h>
#include <QtCore/QList>

// forward declarations
class CodeModel;
class _ArgumentModelItem;
class _ClassModelItem;
class _CodeModelItem;
class _EnumModelItem;
class _EnumeratorModelItem;
class _FileModelItem;
class _FunctionDefinitionModelItem;
class _FunctionModelItem;
class _NamespaceModelItem;
class _ScopeModelItem;
class _TemplateParameterModelItem;
class _TypeAliasModelItem;
class _VariableModelItem;
class _MemberModelItem;
class TypeInfo;

typedef CodeModelPointer<_ArgumentModelItem> ArgumentModelItem;
typedef CodeModelPointer<_ClassModelItem> ClassModelItem;
typedef CodeModelPointer<_CodeModelItem> CodeModelItem;
typedef CodeModelPointer<_EnumModelItem> EnumModelItem;
typedef CodeModelPointer<_EnumeratorModelItem> EnumeratorModelItem;
typedef CodeModelPointer<_FileModelItem> FileModelItem;
typedef CodeModelPointer<_FunctionDefinitionModelItem> FunctionDefinitionModelItem;
typedef CodeModelPointer<_FunctionModelItem> FunctionModelItem;
typedef CodeModelPointer<_NamespaceModelItem> NamespaceModelItem;
typedef CodeModelPointer<_ScopeModelItem> ScopeModelItem;
typedef CodeModelPointer<_TemplateParameterModelItem> TemplateParameterModelItem;
typedef CodeModelPointer<_TypeAliasModelItem> TypeAliasModelItem;
typedef CodeModelPointer<_VariableModelItem> VariableModelItem;
typedef CodeModelPointer<_MemberModelItem> MemberModelItem;

typedef QList<ArgumentModelItem> ArgumentList;
typedef QList<ClassModelItem> ClassList;
typedef QList<CodeModelItem> ItemList;
typedef QList<EnumModelItem> EnumList;
typedef QList<EnumeratorModelItem> EnumeratorList;
typedef QList<FileModelItem> FileList;
typedef QList<FunctionDefinitionModelItem> FunctionDefinitionList;
typedef QList<FunctionModelItem> FunctionList;
typedef QList<NamespaceModelItem> NamespaceList;
typedef QList<ScopeModelItem> ScopeList;
typedef QList<TemplateParameterModelItem> TemplateParameterList;
typedef QList<TypeAliasModelItem> TypeAliasList;
typedef QList<VariableModelItem> VariableList;
typedef QList<MemberModelItem> MemberList;

#endif // CODEMODEL_FWD_H
