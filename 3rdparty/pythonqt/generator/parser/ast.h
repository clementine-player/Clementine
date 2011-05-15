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


#ifndef AST_H
#define AST_H

#include "smallobject.h"
#include "list.h"

class QString;

#define DECLARE_AST_NODE(k) \
    enum { __node_kind = Kind_##k };

class TokenStream;

struct AccessSpecifierAST;
struct AsmDefinitionAST;
struct BaseClauseAST;
struct BaseSpecifierAST;
struct BinaryExpressionAST;
struct CastExpressionAST;
struct ClassMemberAccessAST;
struct ClassSpecifierAST;
struct CompoundStatementAST;
struct ConditionAST;
struct ConditionalExpressionAST;
struct CppCastExpressionAST;
struct CtorInitializerAST;
struct DeclarationAST;
struct DeclarationStatementAST;
struct DeclaratorAST;
struct DeleteExpressionAST;
struct DoStatementAST;
struct ElaboratedTypeSpecifierAST;
struct EnumSpecifierAST;
struct EnumeratorAST;
struct ExceptionSpecificationAST;
struct ExpressionAST;
struct ExpressionOrDeclarationStatementAST;
struct ExpressionStatementAST;
struct ForStatementAST;
struct FunctionCallAST;
struct FunctionDefinitionAST;
struct IfStatementAST;
struct IncrDecrExpressionAST;
struct InitDeclaratorAST;
struct InitializerAST;
struct InitializerClauseAST;
struct LabeledStatementAST;
struct LinkageBodyAST;
struct LinkageSpecificationAST;
struct MemInitializerAST;
struct NameAST;
struct NamespaceAST;
struct NamespaceAliasDefinitionAST;
struct NewDeclaratorAST;
struct NewExpressionAST;
struct NewInitializerAST;
struct NewTypeIdAST;
struct OperatorAST;
struct OperatorFunctionIdAST;
struct ParameterDeclarationAST;
struct ParameterDeclarationClauseAST;
struct PostfixExpressionAST;
struct PrimaryExpressionAST;
struct PtrOperatorAST;
struct PtrToMemberAST;
struct ReturnStatementAST;
struct SimpleDeclarationAST;
struct SimpleTypeSpecifierAST;
struct SizeofExpressionAST;
struct StatementAST;
struct StringLiteralAST;
struct SubscriptExpressionAST;
struct SwitchStatementAST;
struct TemplateArgumentAST;
struct TemplateDeclarationAST;
struct TemplateParameterAST;
struct ThrowExpressionAST;
struct TranslationUnitAST;
struct TryBlockStatementAST;
struct TypeIdAST;
struct TypeIdentificationAST;
struct TypeParameterAST;
struct TypeSpecifierAST;
struct TypedefAST;
struct UnaryExpressionAST;
struct UnqualifiedNameAST;
struct UsingAST;
struct UsingDirectiveAST;
struct WhileStatementAST;
struct WinDeclSpecAST;
struct QPropertyAST;
struct QEnumsAST;

struct AST
{
  enum NODE_KIND
    {
      Kind_UNKNOWN = 0,

      Kind_AccessSpecifier,
      Kind_AsmDefinition,
      Kind_BaseClause,
      Kind_BaseSpecifier,
      Kind_BinaryExpression,
      Kind_CastExpression,
      Kind_ClassMemberAccess,
      Kind_ClassSpecifier,
      Kind_CompoundStatement,
      Kind_Condition,
      Kind_ConditionalExpression,
      Kind_CppCastExpression,
      Kind_CtorInitializer,
      Kind_DeclarationStatement,
      Kind_Declarator,
      Kind_DeleteExpression,
      Kind_DoStatement,
      Kind_ElaboratedTypeSpecifier,
      Kind_EnumSpecifier,
      Kind_Enumerator,
      Kind_ExceptionSpecification,
      Kind_ExpressionOrDeclarationStatement,
      Kind_ExpressionStatement,
      Kind_ForStatement,
      Kind_FunctionCall,
      Kind_FunctionDefinition,
      Kind_IfStatement,
      Kind_IncrDecrExpression,
      Kind_InitDeclarator,
      Kind_Initializer,
      Kind_InitializerClause,
      Kind_LabeledStatement,
      Kind_LinkageBody,
      Kind_LinkageSpecification,
      Kind_MemInitializer,
      Kind_Name,
      Kind_Namespace,
      Kind_NamespaceAliasDefinition,
      Kind_NewDeclarator,
      Kind_NewExpression,
      Kind_NewInitializer,
      Kind_NewTypeId,
      Kind_Operator,
      Kind_OperatorFunctionId,
      Kind_ParameterDeclaration,
      Kind_ParameterDeclarationClause,
      Kind_PostfixExpression,
      Kind_PrimaryExpression,
      Kind_PtrOperator,
      Kind_PtrToMember,
      Kind_ReturnStatement,
      Kind_SimpleDeclaration,
      Kind_SimpleTypeSpecifier,
      Kind_SizeofExpression,
      Kind_StringLiteral,
      Kind_SubscriptExpression,
      Kind_SwitchStatement,
      Kind_TemplateArgument,
      Kind_TemplateDeclaration,
      Kind_TemplateParameter,
      Kind_ThrowExpression,
      Kind_TranslationUnit,
      Kind_TryBlockStatement,
      Kind_TypeId,
      Kind_TypeIdentification,
      Kind_TypeParameter,
      Kind_Typedef,
      Kind_UnaryExpression,
      Kind_UnqualifiedName,
      Kind_Using,
      Kind_UsingDirective,
      Kind_WhileStatement,
      Kind_WinDeclSpec,
      Kind_QPropertyAST,
      Kind_ForwardDeclarationSpecifier,
      Kind_QEnumsAST,

      NODE_KIND_COUNT
    };

  QString toString(TokenStream *stream) const;

  int kind;

  std::size_t start_token;
  std::size_t end_token;
};

struct TypeSpecifierAST: public AST
{
  const ListNode<std::size_t> *cv;
};

struct StatementAST: public AST
{
};

struct ExpressionAST: public AST
{
};

struct DeclarationAST: public AST
{
};

struct AccessSpecifierAST: public DeclarationAST
{
  DECLARE_AST_NODE(AccessSpecifier)

  const ListNode<std::size_t> *specs;
};

struct AsmDefinitionAST: public DeclarationAST
{
  DECLARE_AST_NODE(AsmDefinition)

  const ListNode<std::size_t> *cv;
};

struct BaseClauseAST: public AST // ### kill me
{
  DECLARE_AST_NODE(BaseClause)

  const ListNode<BaseSpecifierAST*> *base_specifiers;
};

struct BaseSpecifierAST: public AST
{
  DECLARE_AST_NODE(BaseSpecifier)

  std::size_t virt;
  std::size_t access_specifier;
  NameAST *name;
};

struct BinaryExpressionAST: public ExpressionAST
{
  DECLARE_AST_NODE(BinaryExpression)

  std::size_t op;
  ExpressionAST *left_expression;
  ExpressionAST *right_expression;
};

struct CastExpressionAST: public ExpressionAST
{
  DECLARE_AST_NODE(CastExpression)

  TypeIdAST *type_id;
  ExpressionAST *expression;
};

struct ClassMemberAccessAST: public ExpressionAST
{
  DECLARE_AST_NODE(ClassMemberAccess)

  std::size_t op;
  NameAST *name;
};

struct ClassSpecifierAST: public TypeSpecifierAST
{
  DECLARE_AST_NODE(ClassSpecifier)

  WinDeclSpecAST *win_decl_specifiers;
  std::size_t class_key;
  NameAST *name;
  BaseClauseAST *base_clause;
  const ListNode<DeclarationAST*> *member_specs;
};

struct ForwardDeclarationSpecifierAST: public TypeSpecifierAST
{
    DECLARE_AST_NODE(ForwardDeclarationSpecifier)

    std::size_t class_key;
    NameAST *name;
    BaseClauseAST *base_clause;
};

struct CompoundStatementAST: public StatementAST
{
  DECLARE_AST_NODE(CompoundStatement)

  const ListNode<StatementAST*> *statements;
};

struct ConditionAST: public AST
{
  DECLARE_AST_NODE(Condition)

  TypeSpecifierAST *type_specifier;
  DeclaratorAST *declarator;
  ExpressionAST *expression;
};

struct ConditionalExpressionAST: public ExpressionAST
{
  DECLARE_AST_NODE(ConditionalExpression)

  ExpressionAST *condition;
  ExpressionAST *left_expression;
  ExpressionAST *right_expression;
};

struct CppCastExpressionAST: public ExpressionAST
{
  DECLARE_AST_NODE(CppCastExpression)

  std::size_t op;
  TypeIdAST *type_id;
  ExpressionAST *expression;
  const ListNode<ExpressionAST*> *sub_expressions;
};

struct CtorInitializerAST: public AST
{
  DECLARE_AST_NODE(CtorInitializer)

  std::size_t colon;
  const ListNode<MemInitializerAST*> *member_initializers;
};

struct DeclarationStatementAST: public StatementAST
{
  DECLARE_AST_NODE(DeclarationStatement)

  DeclarationAST *declaration;
};

struct DeclaratorAST: public AST
{
  DECLARE_AST_NODE(Declarator)

  const ListNode<PtrOperatorAST*> *ptr_ops;
  DeclaratorAST *sub_declarator;
  NameAST *id;
  ExpressionAST *bit_expression;
  const ListNode<ExpressionAST*> *array_dimensions;
  ParameterDeclarationClauseAST *parameter_declaration_clause;
  const ListNode<std::size_t> *fun_cv;
  ExceptionSpecificationAST *exception_spec;
};

struct DeleteExpressionAST: public ExpressionAST
{
  DECLARE_AST_NODE(DeleteExpression)

  std::size_t scope_token;
  std::size_t delete_token;
  std::size_t lbracket_token;
  std::size_t rbracket_token;
  ExpressionAST *expression;
};

struct DoStatementAST: public StatementAST
{
  DECLARE_AST_NODE(DoStatement)

  StatementAST *statement;
  ExpressionAST *expression;
};

struct ElaboratedTypeSpecifierAST: public TypeSpecifierAST
{
  DECLARE_AST_NODE(ElaboratedTypeSpecifier)

  std::size_t type;
  NameAST *name;
};

struct EnumSpecifierAST: public TypeSpecifierAST
{
  DECLARE_AST_NODE(EnumSpecifier)

  NameAST *name;
  const ListNode<EnumeratorAST*> *enumerators;
};

struct EnumeratorAST: public AST
{
  DECLARE_AST_NODE(Enumerator)

  std::size_t id;
  ExpressionAST *expression;
};

struct ExceptionSpecificationAST: public AST
{
  DECLARE_AST_NODE(ExceptionSpecification)

  std::size_t ellipsis;
  const ListNode<TypeIdAST*> *type_ids;
};

struct ExpressionOrDeclarationStatementAST: public StatementAST
{
  DECLARE_AST_NODE(ExpressionOrDeclarationStatement)

  StatementAST *expression;
  StatementAST *declaration;
};

struct ExpressionStatementAST: public StatementAST
{
  DECLARE_AST_NODE(ExpressionStatement)

  ExpressionAST *expression;
};

struct FunctionCallAST: public ExpressionAST
{
  DECLARE_AST_NODE(FunctionCall)

  ExpressionAST *arguments;
};

struct FunctionDefinitionAST: public DeclarationAST
{
  DECLARE_AST_NODE(FunctionDefinition)

  const ListNode<std::size_t> *storage_specifiers;
  const ListNode<std::size_t> *function_specifiers;
  TypeSpecifierAST *type_specifier;
  InitDeclaratorAST *init_declarator;
  StatementAST *function_body;
  WinDeclSpecAST *win_decl_specifiers;
};

struct ForStatementAST: public StatementAST
{
  DECLARE_AST_NODE(ForStatement)

  StatementAST *init_statement;
  ConditionAST *condition;
  ExpressionAST *expression;
  StatementAST *statement;
};

struct IfStatementAST: public StatementAST
{
  DECLARE_AST_NODE(IfStatement)

  ConditionAST *condition;
  StatementAST *statement;
  StatementAST *else_statement;
};

struct IncrDecrExpressionAST: public ExpressionAST
{
  DECLARE_AST_NODE(IncrDecrExpression)

  std::size_t op;
};

struct InitDeclaratorAST: public AST
{
  DECLARE_AST_NODE(InitDeclarator)

  DeclaratorAST *declarator;
  InitializerAST *initializer;
};

struct InitializerAST: public AST
{
  DECLARE_AST_NODE(Initializer)

  InitializerClauseAST *initializer_clause;
  ExpressionAST *expression;
};

struct InitializerClauseAST: public AST
{
  DECLARE_AST_NODE(InitializerClause)

  ExpressionAST *expression;
};

struct LabeledStatementAST: public StatementAST
{
  DECLARE_AST_NODE(LabeledStatement)
};

struct LinkageBodyAST: public AST
{
  DECLARE_AST_NODE(LinkageBody)

  const ListNode<DeclarationAST*> *declarations;
};

struct LinkageSpecificationAST: public DeclarationAST
{
  DECLARE_AST_NODE(LinkageSpecification)

  std::size_t extern_type;
  LinkageBodyAST *linkage_body;
  DeclarationAST *declaration;
};

struct MemInitializerAST: public AST
{
  DECLARE_AST_NODE(MemInitializer)

  NameAST *initializer_id;
  ExpressionAST *expression;
};

struct NameAST: public AST
{
  DECLARE_AST_NODE(Name)

  bool global;
  const ListNode<UnqualifiedNameAST*> *qualified_names;
  UnqualifiedNameAST *unqualified_name;
};

struct NamespaceAST: public DeclarationAST
{
  DECLARE_AST_NODE(Namespace)

  std::size_t namespace_name;
  LinkageBodyAST *linkage_body;
};

struct NamespaceAliasDefinitionAST: public DeclarationAST
{
  DECLARE_AST_NODE(NamespaceAliasDefinition)

  std::size_t namespace_name;
  NameAST *alias_name;
};

struct NewDeclaratorAST: public AST
{
  DECLARE_AST_NODE(NewDeclarator)

  PtrOperatorAST *ptr_op;
  NewDeclaratorAST *sub_declarator;
  const ListNode<ExpressionAST*> *expressions;
};

struct NewExpressionAST: public ExpressionAST
{
  DECLARE_AST_NODE(NewExpression)

  std::size_t scope_token;
  std::size_t new_token;
  ExpressionAST *expression;
  TypeIdAST *type_id;
  NewTypeIdAST *new_type_id;
  NewInitializerAST *new_initializer;
};

struct NewInitializerAST: public AST
{
  DECLARE_AST_NODE(NewInitializer)

  ExpressionAST *expression;
};

struct NewTypeIdAST: public AST
{
  DECLARE_AST_NODE(NewTypeId)

  TypeSpecifierAST *type_specifier;
  NewInitializerAST *new_initializer;
  NewDeclaratorAST *new_declarator;
};

struct OperatorAST: public AST
{
  DECLARE_AST_NODE(Operator)

  std::size_t op;
  std::size_t open;
  std::size_t close;
};

struct OperatorFunctionIdAST: public AST
{
  DECLARE_AST_NODE(OperatorFunctionId)

  OperatorAST *op;
  TypeSpecifierAST *type_specifier;
  const ListNode<PtrOperatorAST*> *ptr_ops;
};

struct ParameterDeclarationAST: public AST
{
  DECLARE_AST_NODE(ParameterDeclaration)

  TypeSpecifierAST *type_specifier;
  DeclaratorAST *declarator;
  ExpressionAST *expression;
};

struct ParameterDeclarationClauseAST: public AST
{
  DECLARE_AST_NODE(ParameterDeclarationClause)

  const ListNode<ParameterDeclarationAST*> *parameter_declarations;
  std::size_t ellipsis;
};

struct PostfixExpressionAST: public ExpressionAST
{
  DECLARE_AST_NODE(PostfixExpression)

  TypeSpecifierAST *type_specifier;
  ExpressionAST *expression;
  const ListNode<ExpressionAST*> *sub_expressions;
};

struct PrimaryExpressionAST: public ExpressionAST
{
  DECLARE_AST_NODE(PrimaryExpression)

  StringLiteralAST *literal;
  std::size_t token;
  StatementAST *expression_statement;
  ExpressionAST *sub_expression;
  NameAST *name;
};

struct PtrOperatorAST: public AST
{
  DECLARE_AST_NODE(PtrOperator)

  const ListNode<std::size_t> *cv;
  std::size_t op;
  PtrToMemberAST *mem_ptr;
};

struct PtrToMemberAST: public AST
{
  DECLARE_AST_NODE(PtrToMember)
};

struct ReturnStatementAST: public StatementAST
{
  DECLARE_AST_NODE(ReturnStatement)

  ExpressionAST *expression;
};

struct SimpleDeclarationAST: public DeclarationAST
{
  DECLARE_AST_NODE(SimpleDeclaration)

  const ListNode<std::size_t> *storage_specifiers;
  const ListNode<std::size_t> *function_specifiers;
  TypeSpecifierAST *type_specifier;
  const ListNode<InitDeclaratorAST*> *init_declarators;
  WinDeclSpecAST *win_decl_specifiers;
};

struct SimpleTypeSpecifierAST: public TypeSpecifierAST
{
  DECLARE_AST_NODE(SimpleTypeSpecifier)

  const ListNode<std::size_t> *integrals;
  std::size_t type_of;
  TypeIdAST *type_id;
  ExpressionAST *expression;
  NameAST *name;
};

struct SizeofExpressionAST: public ExpressionAST
{
  DECLARE_AST_NODE(SizeofExpression)

  std::size_t sizeof_token;
  TypeIdAST *type_id;
  ExpressionAST *expression;
};

struct StringLiteralAST: public AST
{
  DECLARE_AST_NODE(StringLiteral)

  const ListNode<std::size_t> *literals;
};

struct SubscriptExpressionAST: public ExpressionAST
{
  DECLARE_AST_NODE(SubscriptExpression)

  ExpressionAST *subscript;
};

struct SwitchStatementAST: public StatementAST
{
  DECLARE_AST_NODE(SwitchStatement)

  ConditionAST *condition;
  StatementAST *statement;
};

struct TemplateArgumentAST: public AST
{
  DECLARE_AST_NODE(TemplateArgument)

  TypeIdAST *type_id;
  ExpressionAST *expression;
};

struct TemplateDeclarationAST: public DeclarationAST
{
  DECLARE_AST_NODE(TemplateDeclaration)

  std::size_t exported;
  const ListNode<TemplateParameterAST*> *template_parameters;
  DeclarationAST* declaration;
};

struct TemplateParameterAST: public AST
{
  DECLARE_AST_NODE(TemplateParameter)

  TypeParameterAST *type_parameter;
  ParameterDeclarationAST *parameter_declaration;
};

struct ThrowExpressionAST: public ExpressionAST
{
  DECLARE_AST_NODE(ThrowExpression)

  std::size_t throw_token;
  ExpressionAST *expression;
};

struct TranslationUnitAST: public AST
{
  DECLARE_AST_NODE(TranslationUnit)

  const ListNode<DeclarationAST*> *declarations;
};

struct TryBlockStatementAST: public StatementAST
{
  DECLARE_AST_NODE(TryBlockStatement)
};

struct TypeIdAST: public AST
{
  DECLARE_AST_NODE(TypeId)

  TypeSpecifierAST *type_specifier;
  DeclaratorAST *declarator;
};

struct TypeIdentificationAST: public ExpressionAST
{
  DECLARE_AST_NODE(TypeIdentification)

  std::size_t typename_token;
  NameAST *name;
  ExpressionAST *expression;
};

struct TypeParameterAST: public AST
{
  DECLARE_AST_NODE(TypeParameter)

  std::size_t type;
  NameAST *name;
  TypeIdAST *type_id;
  const ListNode<TemplateParameterAST*> *template_parameters;
  NameAST *template_name;
};

struct TypedefAST: public DeclarationAST
{
  DECLARE_AST_NODE(Typedef)

  TypeSpecifierAST *type_specifier;
  const ListNode<InitDeclaratorAST*> *init_declarators;
};

struct UnaryExpressionAST: public ExpressionAST
{
  DECLARE_AST_NODE(UnaryExpression)

  std::size_t op;
  ExpressionAST *expression;
};

struct UnqualifiedNameAST: public AST
{
  DECLARE_AST_NODE(UnqualifiedName)

  std::size_t tilde;
  std::size_t id;
  OperatorFunctionIdAST *operator_id;
  const ListNode<TemplateArgumentAST*> *template_arguments;
};

struct UsingAST: public DeclarationAST
{
  DECLARE_AST_NODE(Using)

  std::size_t type_name;
  NameAST *name;
};

struct UsingDirectiveAST: public DeclarationAST
{
  DECLARE_AST_NODE(UsingDirective)

  NameAST *name;
};

struct WhileStatementAST: public StatementAST
{
  DECLARE_AST_NODE(WhileStatement)

  ConditionAST *condition;
  StatementAST *statement;
};

struct WinDeclSpecAST: public AST
{
  DECLARE_AST_NODE(WinDeclSpec)

  std::size_t specifier;
  std::size_t modifier;
};

struct QPropertyAST : public DeclarationAST
{
  DECLARE_AST_NODE(QPropertyAST)
};

struct QEnumsAST : public DeclarationAST
{
  DECLARE_AST_NODE(QEnumsAST)
};

template <class _Tp>
_Tp *CreateNode(pool *memory_pool)
{
  _Tp *node = reinterpret_cast<_Tp*>(memory_pool->allocate(sizeof(_Tp)));
  node->kind = _Tp::__node_kind;
  return node;
}

template <class _Tp>
_Tp ast_cast(AST *item)
{
  if (item && static_cast<_Tp>(0)->__node_kind == item->kind)
    return static_cast<_Tp>(item);

  return 0;
}

#endif // AST_H

// kate: space-indent on; indent-width 2; replace-tabs on;
