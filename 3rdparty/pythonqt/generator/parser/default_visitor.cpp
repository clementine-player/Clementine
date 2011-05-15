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


#include "default_visitor.h"

void DefaultVisitor::visitAccessSpecifier(AccessSpecifierAST *)
{
  // nothing to do
}

void DefaultVisitor::visitAsmDefinition(AsmDefinitionAST *)
{
  // nothing to do
}

void DefaultVisitor::visitBaseClause(BaseClauseAST *node)
{
  visitNodes(this, node->base_specifiers);
}

void DefaultVisitor::visitBaseSpecifier(BaseSpecifierAST *node)
{
  visit(node->name);
}

void DefaultVisitor::visitBinaryExpression(BinaryExpressionAST *node)
{
  visit(node->left_expression);
  visit(node->right_expression);
}

void DefaultVisitor::visitCastExpression(CastExpressionAST *node)
{
  visit(node->type_id);
  visit(node->expression);
}

void DefaultVisitor::visitClassMemberAccess(ClassMemberAccessAST *node)
{
  visit(node->name);
}

void DefaultVisitor::visitClassSpecifier(ClassSpecifierAST *node)
{
  visit(node->win_decl_specifiers);
  visit(node->name);
  visit(node->base_clause);
  visitNodes(this, node->member_specs);
}

void DefaultVisitor::visitCompoundStatement(CompoundStatementAST *node)
{
  visitNodes(this, node->statements);
}

void DefaultVisitor::visitCondition(ConditionAST *node)
{
  visit(node->type_specifier);
  visit(node->declarator);
  visit(node->expression);
}

void DefaultVisitor::visitConditionalExpression(ConditionalExpressionAST *node)
{
  visit(node->condition);
  visit(node->left_expression);
  visit(node->right_expression);
}

void DefaultVisitor::visitCppCastExpression(CppCastExpressionAST *node)
{
  visit(node->type_id);
  visit(node->expression);
  visitNodes(this, node->sub_expressions);
}

void DefaultVisitor::visitCtorInitializer(CtorInitializerAST *node)
{
  visitNodes(this, node->member_initializers);
}

void DefaultVisitor::visitDeclarationStatement(DeclarationStatementAST *node)
{
  visit(node->declaration);
}

void DefaultVisitor::visitDeclarator(DeclaratorAST *node)
{
  visit(node->sub_declarator);
  visitNodes(this, node->ptr_ops);
  visit(node->id);
  visit(node->bit_expression);
  visitNodes(this, node->array_dimensions);
  visit(node->parameter_declaration_clause);
  visit(node->exception_spec);
}

void DefaultVisitor::visitDeleteExpression(DeleteExpressionAST *node)
{
  visit(node->expression);
}

void DefaultVisitor::visitDoStatement(DoStatementAST *node)
{
  visit(node->statement);
  visit(node->expression);
}

void DefaultVisitor::visitElaboratedTypeSpecifier(ElaboratedTypeSpecifierAST *node)
{
  visit(node->name);
}

void DefaultVisitor::visitEnumSpecifier(EnumSpecifierAST *node)
{
  visit(node->name);
  visitNodes(this, node->enumerators);
}

void DefaultVisitor::visitEnumerator(EnumeratorAST *node)
{
  visit(node->expression);
}

void DefaultVisitor::visitExceptionSpecification(ExceptionSpecificationAST *node)
{
  visitNodes(this, node->type_ids);
}

void DefaultVisitor::visitExpressionOrDeclarationStatement(ExpressionOrDeclarationStatementAST *node)
{
  visit(node->expression);
  visit(node->declaration);
}

void DefaultVisitor::visitExpressionStatement(ExpressionStatementAST *node)
{
  visit(node->expression);
}

void DefaultVisitor::visitForStatement(ForStatementAST *node)
{
  visit(node->init_statement);
  visit(node->condition);
  visit(node->expression);
  visit(node->statement);
}

void DefaultVisitor::visitFunctionCall(FunctionCallAST *node)
{
  visit(node->arguments);
}

void DefaultVisitor::visitFunctionDefinition(FunctionDefinitionAST *node)
{
  visit(node->type_specifier);
  visit(node->init_declarator);
  visit(node->function_body);
  visit(node->win_decl_specifiers);
}

void DefaultVisitor::visitIfStatement(IfStatementAST *node)
{
  visit(node->condition);
  visit(node->statement);
  visit(node->else_statement);
}

void DefaultVisitor::visitIncrDecrExpression(IncrDecrExpressionAST *)
{
  // nothing to do
}

void DefaultVisitor::visitInitDeclarator(InitDeclaratorAST *node)
{
  visit(node->declarator);
  visit(node->initializer);
}

void DefaultVisitor::visitInitializer(InitializerAST *node)
{
  visit(node->initializer_clause);
  visit(node->expression);
}

void DefaultVisitor::visitInitializerClause(InitializerClauseAST *node)
{
  visit(node->expression);
}

void DefaultVisitor::visitLabeledStatement(LabeledStatementAST *)
{
  // nothing to do
}

void DefaultVisitor::visitLinkageBody(LinkageBodyAST *node)
{
  visitNodes(this, node->declarations);
}

void DefaultVisitor::visitLinkageSpecification(LinkageSpecificationAST *node)
{
  visit(node->linkage_body);
  visit(node->declaration);
}

void DefaultVisitor::visitMemInitializer(MemInitializerAST *node)
{
  visit(node->initializer_id);
  visit(node->expression);
}

void DefaultVisitor::visitName(NameAST *node)
{
  visitNodes(this, node->qualified_names);
  visit(node->unqualified_name);
}

void DefaultVisitor::visitNamespace(NamespaceAST *node)
{
  visit(node->linkage_body);
}

void DefaultVisitor::visitNamespaceAliasDefinition(NamespaceAliasDefinitionAST *node)
{
  visit(node->alias_name);
}

void DefaultVisitor::visitNewDeclarator(NewDeclaratorAST *node)
{
  visit(node->ptr_op);
  visit(node->sub_declarator);
  visitNodes(this, node->expressions);
}

void DefaultVisitor::visitNewExpression(NewExpressionAST *node)
{
  visit(node->expression);
  visit(node->type_id);
  visit(node->new_type_id);
  visit(node->new_initializer);
}

void DefaultVisitor::visitNewInitializer(NewInitializerAST *node)
{
  visit(node->expression);
}

void DefaultVisitor::visitNewTypeId(NewTypeIdAST *node)
{
  visit(node->type_specifier);
  visit(node->new_initializer);
  visit(node->new_declarator);
}

void DefaultVisitor::visitOperator(OperatorAST *)
{
  // nothing to do
}

void DefaultVisitor::visitOperatorFunctionId(OperatorFunctionIdAST *node)
{
  visit(node->op);
  visit(node->type_specifier);
  visitNodes(this, node->ptr_ops);
}

void DefaultVisitor::visitParameterDeclaration(ParameterDeclarationAST *node)
{
  visit(node->type_specifier);
  visit(node->declarator);
  visit(node->expression);
}

void DefaultVisitor::visitParameterDeclarationClause(ParameterDeclarationClauseAST *node)
{
  visitNodes(this, node->parameter_declarations);
}

void DefaultVisitor::visitPostfixExpression(PostfixExpressionAST *node)
{
  visit(node->type_specifier);
  visit(node->expression);
  visitNodes(this, node->sub_expressions);
}

void DefaultVisitor::visitPrimaryExpression(PrimaryExpressionAST *node)
{
  visit(node->literal);
  visit(node->expression_statement);
  visit(node->sub_expression);
  visit(node->name);
}

void DefaultVisitor::visitPtrOperator(PtrOperatorAST *node)
{
  visit(node->mem_ptr);
}

void DefaultVisitor::visitPtrToMember(PtrToMemberAST *)
{
  // nothing to do
}

void DefaultVisitor::visitReturnStatement(ReturnStatementAST *node)
{
  visit(node->expression);
}

void DefaultVisitor::visitSimpleDeclaration(SimpleDeclarationAST *node)
{
  visit(node->type_specifier);
  visitNodes(this, node->init_declarators);
  visit(node->win_decl_specifiers);
}

void DefaultVisitor::visitSimpleTypeSpecifier(SimpleTypeSpecifierAST *node)
{
  visit(node->name);
  visit(node->type_id);
  visit(node->expression);
}

void DefaultVisitor::visitSizeofExpression(SizeofExpressionAST *node)
{
  visit(node->type_id);
  visit(node->expression);
}

void DefaultVisitor::visitStringLiteral(StringLiteralAST *)
{
  // nothing to do
}

void DefaultVisitor::visitSubscriptExpression(SubscriptExpressionAST *node)
{
  visit(node->subscript);
}

void DefaultVisitor::visitSwitchStatement(SwitchStatementAST *node)
{
  visit(node->condition);
  visit(node->statement);
}

void DefaultVisitor::visitTemplateArgument(TemplateArgumentAST *node)
{
  visit(node->type_id);
  visit(node->expression);
}

void DefaultVisitor::visitTemplateDeclaration(TemplateDeclarationAST *node)
{
  visitNodes(this, node->template_parameters);
  visit(node->declaration);
}

void DefaultVisitor::visitTemplateParameter(TemplateParameterAST *node)
{
  visit(node->type_parameter);
  visit(node->parameter_declaration);
}

void DefaultVisitor::visitThrowExpression(ThrowExpressionAST *node)
{
  visit(node->expression);
}

void DefaultVisitor::visitTranslationUnit(TranslationUnitAST *node)
{
  visitNodes(this, node->declarations);
}

void DefaultVisitor::visitTryBlockStatement(TryBlockStatementAST *)
{
  // nothing to do
}

void DefaultVisitor::visitTypeId(TypeIdAST *node)
{
  visit(node->type_specifier);
  visit(node->declarator);
}

void DefaultVisitor::visitTypeIdentification(TypeIdentificationAST *node)
{
  visit(node->name);
  visit(node->expression);
}

void DefaultVisitor::visitTypeParameter(TypeParameterAST *node)
{
  visit(node->name);
  visit(node->type_id);
  visitNodes(this, node->template_parameters);
  visit(node->template_name);
}

void DefaultVisitor::visitTypedef(TypedefAST *node)
{
  visit(node->type_specifier);
  visitNodes(this, node->init_declarators);
}

void DefaultVisitor::visitUnaryExpression(UnaryExpressionAST *node)
{
  visit(node->expression);
}

void DefaultVisitor::visitUnqualifiedName(UnqualifiedNameAST *node)
{
  visit(node->operator_id);
  visitNodes(this, node->template_arguments);
}

void DefaultVisitor::visitUsing(UsingAST *node)
{
  visit(node->name);
}

void DefaultVisitor::visitUsingDirective(UsingDirectiveAST *node)
{
  visit(node->name);
}

void DefaultVisitor::visitWhileStatement(WhileStatementAST *node)
{
  visit(node->condition);
  visit(node->statement);
}

void DefaultVisitor::visitWinDeclSpec(WinDeclSpecAST *)
{
  // nothing to do
}

// kate: space-indent on; indent-width 2; replace-tabs on;
