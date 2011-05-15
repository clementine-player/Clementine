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


#ifndef CODEMODEL_H
#define CODEMODEL_H

#include "codemodel_fwd.h"
#include <codemodel_pointer.h>

#include <QtCore/QHash>
#include <QtCore/QList>
#include <QtCore/QString>
#include <QtCore/QStringList>
#include <QtCore/QVector>

#define DECLARE_MODEL_NODE(k) \
    enum { __node_kind = Kind_##k }; \
    typedef CodeModelPointer<k##ModelItem> Pointer;

template <class _Target, class _Source>
_Target model_static_cast(_Source item)
{
  typedef typename _Target::Type * _Target_pointer;

  _Target ptr (static_cast<_Target_pointer>(item.data()));
  return ptr;
}

class CodeModel
{
public:
  enum AccessPolicy
  {
    Public,
    Protected,
    Private
  };

  enum FunctionType
  {
    Normal,
    Signal,
    Slot
  };

  enum ClassType
  {
    Class,
    Struct,
    Union
  };

public:
  CodeModel();
  virtual ~CodeModel();

  template <class _Target> _Target create()
  {
    typedef typename _Target::Type _Target_type;

    _Target result = _Target_type::create(this);
    result->setCreationId(_M_creation_id++);
    return result;
  }

  FileList files() const;
  NamespaceModelItem globalNamespace() const;

  void addFile(FileModelItem item);
  void removeFile(FileModelItem item);
  FileModelItem findFile(const QString &name) const;
  QHash<QString, FileModelItem> fileMap() const;

  CodeModelItem findItem(const QStringList &qualifiedName, CodeModelItem scope) const;

  void wipeout();

private:
  QHash<QString, FileModelItem> _M_files;
  NamespaceModelItem _M_globalNamespace;
  std::size_t _M_creation_id;

private:
  CodeModel(const CodeModel &other);
  void operator = (const CodeModel &other);
};

class TypeInfo
{
public:
  TypeInfo(const TypeInfo &other)
    : flags(other.flags),
      m_qualifiedName(other.m_qualifiedName),
      m_arrayElements(other.m_arrayElements),
      m_arguments(other.m_arguments)
  {
  }

  TypeInfo():
    flags (0) {}

  QStringList qualifiedName() const { return m_qualifiedName; }
  void setQualifiedName(const QStringList &qualified_name) { m_qualifiedName = qualified_name; }

  bool isConstant() const { return m_constant; }
  void setConstant(bool is) { m_constant = is; }

  bool isVolatile() const { return m_volatile; }
  void setVolatile(bool is) { m_volatile = is; }

  bool isReference() const { return m_reference; }
  void setReference(bool is) { m_reference = is; }

  int indirections() const { return m_indirections; }
  void setIndirections(int indirections) { m_indirections = indirections; }

  bool isFunctionPointer() const { return m_functionPointer; }
  void setFunctionPointer(bool is) { m_functionPointer = is; }

  QStringList arrayElements() const { return m_arrayElements; }
  void setArrayElements(const QStringList &arrayElements) { m_arrayElements = arrayElements; }

  QList<TypeInfo> arguments() const { return m_arguments; }
  void setArguments(const QList<TypeInfo> &arguments);
  void addArgument(const TypeInfo &arg) { m_arguments.append(arg); }

  bool operator==(const TypeInfo &other);
  bool operator!=(const TypeInfo &other) { return !(*this==other); }

  // ### arrays and templates??

  QString toString() const;

  static TypeInfo combine (const TypeInfo &__lhs, const TypeInfo &__rhs);
  static TypeInfo resolveType (TypeInfo const &__type, CodeModelItem __scope);

private:
  union
  {
    uint flags;

    struct
    {
      uint m_constant: 1;
      uint m_volatile: 1;
      uint m_reference: 1;
      uint m_functionPointer: 1;
      uint m_indirections: 6;
      uint m_padding: 22;
    };
  };

  QStringList m_qualifiedName;
  QStringList m_arrayElements;
  QList<TypeInfo> m_arguments;
};

class _CodeModelItem: public QSharedData
{
public:
  enum Kind
  {
    /* These are bit-flags resembling inheritance */
    Kind_Scope = 0x1,
    Kind_Namespace = 0x2 | Kind_Scope,
    Kind_Member = 0x4,
    Kind_Function = 0x8 | Kind_Member,
    KindMask = 0xf,

    /* These are for classes that are not inherited from */
    FirstKind = 0x8,
    Kind_Argument = 1 << FirstKind,
    Kind_Class = 2 << FirstKind | Kind_Scope,
    Kind_Enum = 3 << FirstKind,
    Kind_Enumerator = 4 << FirstKind,
    Kind_File = 5 << FirstKind | Kind_Namespace,
    Kind_FunctionDefinition = 6 << FirstKind | Kind_Function,
    Kind_TemplateParameter = 7 << FirstKind,
    Kind_TypeAlias = 8 << FirstKind,
    Kind_Variable = 9 << FirstKind | Kind_Member
  };

public:
  virtual ~_CodeModelItem();

  int kind() const;

  QStringList qualifiedName() const;

  QString name() const;
  void setName(const QString &name);

  QStringList scope() const;
  void setScope(const QStringList &scope);

  QString fileName() const;
  void setFileName(const QString &fileName);

  FileModelItem file() const;

  void getStartPosition(int *line, int *column);
  void setStartPosition(int line, int column);

  void getEndPosition(int *line, int *column);
  void setEndPosition(int line, int column);

  inline std::size_t creationId() const { return _M_creation_id; }
  inline void setCreationId(std::size_t creation_id) { _M_creation_id = creation_id; }

  inline CodeModel *model() const { return _M_model; }

  CodeModelItem toItem() const;

protected:
  _CodeModelItem(CodeModel *model, int kind);
  void setKind(int kind);

private:
  CodeModel *_M_model;
  int _M_kind;
  int _M_startLine;
  int _M_startColumn;
  int _M_endLine;
  int _M_endColumn;
  std::size_t _M_creation_id;
  QString _M_name;
  QString _M_fileName;
  QStringList _M_scope;

private:
  _CodeModelItem(const _CodeModelItem &other);
  void operator = (const _CodeModelItem &other);
};

class _ScopeModelItem: public _CodeModelItem
{
public:
  DECLARE_MODEL_NODE(Scope)

  static ScopeModelItem create(CodeModel *model);

public:
  ClassList classes() const;
  EnumList enums() const;
  FunctionDefinitionList functionDefinitions() const;
  FunctionList functions() const;
  TypeAliasList typeAliases() const;
  VariableList variables() const;

  void addClass(ClassModelItem item);
  void addEnum(EnumModelItem item);
  void addFunction(FunctionModelItem item);
  void addFunctionDefinition(FunctionDefinitionModelItem item);
  void addTypeAlias(TypeAliasModelItem item);
  void addVariable(VariableModelItem item);

  void removeClass(ClassModelItem item);
  void removeEnum(EnumModelItem item);
  void removeFunction(FunctionModelItem item);
  void removeFunctionDefinition(FunctionDefinitionModelItem item);
  void removeTypeAlias(TypeAliasModelItem item);
  void removeVariable(VariableModelItem item);

  ClassModelItem findClass(const QString &name) const;
  EnumModelItem findEnum(const QString &name) const;
  FunctionDefinitionList findFunctionDefinitions(const QString &name) const;
  FunctionList findFunctions(const QString &name) const;
  TypeAliasModelItem findTypeAlias(const QString &name) const;
  VariableModelItem findVariable(const QString &name) const;

  void addEnumsDeclaration(const QString &enumsDeclaration);
  QStringList enumsDeclarations() const { return _M_enumsDeclarations; }

  inline QHash<QString, ClassModelItem> classMap() const { return _M_classes; }
  inline QHash<QString, EnumModelItem> enumMap() const { return _M_enums; }
  inline QHash<QString, TypeAliasModelItem> typeAliasMap() const { return _M_typeAliases; }
  inline QHash<QString, VariableModelItem> variableMap() const { return _M_variables; }
  inline QMultiHash<QString, FunctionDefinitionModelItem> functionDefinitionMap() const { return _M_functionDefinitions; }
  inline QMultiHash<QString, FunctionModelItem> functionMap() const { return _M_functions; }

  FunctionModelItem declaredFunction(FunctionModelItem item);

protected:
  _ScopeModelItem(CodeModel *model, int kind = __node_kind)
    : _CodeModelItem(model, kind) {}

private:
  QHash<QString, ClassModelItem> _M_classes;
  QHash<QString, EnumModelItem> _M_enums;
  QHash<QString, TypeAliasModelItem> _M_typeAliases;
  QHash<QString, VariableModelItem> _M_variables;
  QMultiHash<QString, FunctionDefinitionModelItem> _M_functionDefinitions;
  QMultiHash<QString, FunctionModelItem> _M_functions;

private:
  _ScopeModelItem(const _ScopeModelItem &other);
  void operator = (const _ScopeModelItem &other);

  QStringList _M_enumsDeclarations;
};

class _ClassModelItem: public _ScopeModelItem
{
public:
  DECLARE_MODEL_NODE(Class)

  static ClassModelItem create(CodeModel *model);

public:
  QStringList baseClasses() const;

  void setBaseClasses(const QStringList &baseClasses);
  void addBaseClass(const QString &baseClass);
  void removeBaseClass(const QString &baseClass);

  TemplateParameterList templateParameters() const;
  void setTemplateParameters(const TemplateParameterList &templateParameters);

  bool extendsClass(const QString &name) const;

  void setClassType(CodeModel::ClassType type);
  CodeModel::ClassType classType() const;

  void addPropertyDeclaration(const QString &propertyDeclaration);
  QStringList propertyDeclarations() const { return _M_propertyDeclarations; }

protected:
  _ClassModelItem(CodeModel *model, int kind = __node_kind)
    : _ScopeModelItem(model, kind), _M_classType(CodeModel::Class) {}

private:
  QStringList _M_baseClasses;
  TemplateParameterList _M_templateParameters;
  CodeModel::ClassType _M_classType;

  QStringList _M_propertyDeclarations;

private:
  _ClassModelItem(const _ClassModelItem &other);
  void operator = (const _ClassModelItem &other);
};

class _NamespaceModelItem: public _ScopeModelItem
{
public:
  DECLARE_MODEL_NODE(Namespace)

  static NamespaceModelItem create(CodeModel *model);

public:
  NamespaceList namespaces() const;

  void addNamespace(NamespaceModelItem item);
  void removeNamespace(NamespaceModelItem item);

  NamespaceModelItem findNamespace(const QString &name) const;

  inline QHash<QString, NamespaceModelItem> namespaceMap() const { return _M_namespaces; };

protected:
  _NamespaceModelItem(CodeModel *model, int kind = __node_kind)
    : _ScopeModelItem(model, kind) {}

private:
  QHash<QString, NamespaceModelItem> _M_namespaces;

private:
  _NamespaceModelItem(const _NamespaceModelItem &other);
  void operator = (const _NamespaceModelItem &other);
};

class _FileModelItem: public _NamespaceModelItem
{
public:
  DECLARE_MODEL_NODE(File)

  static FileModelItem create(CodeModel *model);

protected:
  _FileModelItem(CodeModel *model, int kind = __node_kind)
    : _NamespaceModelItem(model, kind) {}

private:
  _FileModelItem(const _FileModelItem &other);
  void operator = (const _FileModelItem &other);
};

class _ArgumentModelItem: public _CodeModelItem
{
public:
  DECLARE_MODEL_NODE(Argument)

  static ArgumentModelItem create(CodeModel *model);

public:
  TypeInfo type() const;
  void setType(const TypeInfo &type);

  bool defaultValue() const;
  void setDefaultValue(bool defaultValue);

  QString defaultValueExpression() const { return _M_defaultValueExpression; }
  void setDefaultValueExpression(const QString &expr) { _M_defaultValueExpression = expr; }

protected:
  _ArgumentModelItem(CodeModel *model, int kind = __node_kind)
    : _CodeModelItem(model, kind), _M_defaultValue(false) {}

private:
  TypeInfo _M_type;
  QString _M_defaultValueExpression;
  bool _M_defaultValue;

private:
  _ArgumentModelItem(const _ArgumentModelItem &other);
  void operator = (const _ArgumentModelItem &other);
};

class _MemberModelItem: public _CodeModelItem
{
public:
  DECLARE_MODEL_NODE(Member)

  bool isConstant() const;
  void setConstant(bool isConstant);

  bool isVolatile() const;
  void setVolatile(bool isVolatile);

  bool isStatic() const;
  void setStatic(bool isStatic);

  bool isAuto() const;
  void setAuto(bool isAuto);

  bool isFriend() const;
  void setFriend(bool isFriend);

  bool isRegister() const;
  void setRegister(bool isRegister);

  bool isExtern() const;
  void setExtern(bool isExtern);

  bool isMutable() const;
  void setMutable(bool isMutable);

  CodeModel::AccessPolicy accessPolicy() const;
  void setAccessPolicy(CodeModel::AccessPolicy accessPolicy);

  TemplateParameterList templateParameters() const
  { return _M_templateParameters; }

  void setTemplateParameters(const TemplateParameterList &templateParameters)
  { _M_templateParameters = templateParameters; }

  TypeInfo type() const;
  void setType(const TypeInfo &type);

protected:
  _MemberModelItem(CodeModel *model, int kind)
    : _CodeModelItem(model, kind),
      _M_accessPolicy(CodeModel::Public),
      _M_flags(0)
  {}

private:
  TemplateParameterList _M_templateParameters;
  TypeInfo _M_type;
  CodeModel::AccessPolicy _M_accessPolicy;
  union
  {
    struct
    {
      uint _M_isConstant: 1;
      uint _M_isVolatile: 1;
      uint _M_isStatic: 1;
      uint _M_isAuto: 1;
      uint _M_isFriend: 1;
      uint _M_isRegister: 1;
      uint _M_isExtern: 1;
      uint _M_isMutable: 1;
    };
    uint _M_flags;
  };

};

class _FunctionModelItem: public _MemberModelItem
{
public:
  DECLARE_MODEL_NODE(Function)

  static FunctionModelItem create(CodeModel *model);

public:
  ArgumentList arguments() const;

  void addArgument(ArgumentModelItem item);
  void removeArgument(ArgumentModelItem item);

  CodeModel::FunctionType functionType() const;
  void setFunctionType(CodeModel::FunctionType functionType);

  bool isVirtual() const;
  void setVirtual(bool isVirtual);

  bool isInline() const;
  void setInline(bool isInline);

  bool isExplicit() const;
  void setExplicit(bool isExplicit);

  bool isInvokable() const; // Qt
  void setInvokable(bool isInvokable); // Qt

  bool isAbstract() const;
  void setAbstract(bool isAbstract);

  bool isVariadics() const;
  void setVariadics(bool isVariadics);

  bool isSimilar(FunctionModelItem other) const;

protected:
  _FunctionModelItem(CodeModel *model, int kind = __node_kind)
    : _MemberModelItem(model, kind),
      _M_functionType(CodeModel::Normal),
      _M_flags(0)
  {}

private:
  ArgumentList _M_arguments;
  CodeModel::FunctionType _M_functionType;
  union
  {
    struct
    {
      uint _M_isVirtual: 1;
      uint _M_isInline: 1;
      uint _M_isAbstract: 1;
      uint _M_isExplicit: 1;
      uint _M_isVariadics: 1;
      uint _M_isInvokable : 1; // Qt
    };
    uint _M_flags;
  };

private:
  _FunctionModelItem(const _FunctionModelItem &other);
  void operator = (const _FunctionModelItem &other);
};

class _FunctionDefinitionModelItem: public _FunctionModelItem
{
public:
  DECLARE_MODEL_NODE(FunctionDefinition)

  static FunctionDefinitionModelItem create(CodeModel *model);

protected:
  _FunctionDefinitionModelItem(CodeModel *model, int kind = __node_kind)
    : _FunctionModelItem(model, kind) {}

private:
  _FunctionDefinitionModelItem(const _FunctionDefinitionModelItem &other);
  void operator = (const _FunctionDefinitionModelItem &other);
};

class _VariableModelItem: public _MemberModelItem
{
public:
  DECLARE_MODEL_NODE(Variable)

  static VariableModelItem create(CodeModel *model);

protected:
  _VariableModelItem(CodeModel *model, int kind = __node_kind)
    : _MemberModelItem(model, kind)
  {}

private:
  _VariableModelItem(const _VariableModelItem &other);
  void operator = (const _VariableModelItem &other);
};

class _TypeAliasModelItem: public _CodeModelItem
{
public:
  DECLARE_MODEL_NODE(TypeAlias)

  static TypeAliasModelItem create(CodeModel *model);

public:
  TypeInfo type() const;
  void setType(const TypeInfo &type);

protected:
  _TypeAliasModelItem(CodeModel *model, int kind = __node_kind)
    : _CodeModelItem(model, kind) {}

private:
  TypeInfo _M_type;

private:
  _TypeAliasModelItem(const _TypeAliasModelItem &other);
  void operator = (const _TypeAliasModelItem &other);
};

class _EnumModelItem: public _CodeModelItem
{
public:
  DECLARE_MODEL_NODE(Enum)

  static EnumModelItem create(CodeModel *model);

public:
  CodeModel::AccessPolicy accessPolicy() const;
  void setAccessPolicy(CodeModel::AccessPolicy accessPolicy);

  EnumeratorList enumerators() const;
  void addEnumerator(EnumeratorModelItem item);
  void removeEnumerator(EnumeratorModelItem item);

protected:
  _EnumModelItem(CodeModel *model, int kind = __node_kind)
    : _CodeModelItem(model, kind),
      _M_accessPolicy(CodeModel::Public)
  {}

private:
  CodeModel::AccessPolicy _M_accessPolicy;
  EnumeratorList _M_enumerators;

private:
  _EnumModelItem(const _EnumModelItem &other);
  void operator = (const _EnumModelItem &other);
};

class _EnumeratorModelItem: public _CodeModelItem
{
public:
  DECLARE_MODEL_NODE(Enumerator)

  static EnumeratorModelItem create(CodeModel *model);

public:
  QString value() const;
  void setValue(const QString &value);

protected:
  _EnumeratorModelItem(CodeModel *model, int kind = __node_kind)
    : _CodeModelItem(model, kind) {}

private:
  QString _M_value;

private:
  _EnumeratorModelItem(const _EnumeratorModelItem &other);
  void operator = (const _EnumeratorModelItem &other);
};

class _TemplateParameterModelItem: public _CodeModelItem
{
public:
  DECLARE_MODEL_NODE(TemplateParameter)

  static TemplateParameterModelItem create(CodeModel *model);

public:
  TypeInfo type() const;
  void setType(const TypeInfo &type);

  bool defaultValue() const;
  void setDefaultValue(bool defaultValue);

protected:
  _TemplateParameterModelItem(CodeModel *model, int kind = __node_kind)
    : _CodeModelItem(model, kind), _M_defaultValue(false) {}

private:
  TypeInfo _M_type;
  bool _M_defaultValue;

private:
  _TemplateParameterModelItem(const _TemplateParameterModelItem &other);
  void operator = (const _TemplateParameterModelItem &other);
};

template <class _Target, class _Source>
_Target model_safe_cast(_Source item)
{
  typedef typename _Target::Type * _Target_pointer;
  typedef typename _Source::Type * _Source_pointer;

  _Source_pointer source = item.data();
  if (source && source->kind() == _Target_pointer(0)->__node_kind)
    {
      _Target ptr(static_cast<_Target_pointer>(source));
      return ptr;
    }

  return _Target();
}

template <typename _Target, typename _Source>
_Target model_dynamic_cast(_Source item)
{
  typedef typename _Target::Type * _Target_pointer;
  typedef typename _Source::Type * _Source_pointer;

  _Source_pointer source = item.data();
  if (source && (source->kind() == _Target_pointer(0)->__node_kind
         || (int(_Target_pointer(0)->__node_kind) <= int(_CodeModelItem::KindMask)
             && ((source->kind() & _Target_pointer(0)->__node_kind)
                  == _Target_pointer(0)->__node_kind))))
    {
      _Target ptr(static_cast<_Target_pointer>(source));
      return ptr;
    }

  return _Target();
}
#endif // CODEMODEL_H

// kate: space-indent on; indent-width 2; replace-tabs on;
