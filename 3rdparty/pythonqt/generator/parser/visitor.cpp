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


#include "visitor.h"

Visitor::visitor_fun_ptr Visitor::_S_table[AST::NODE_KIND_COUNT] = {
  0,
  reinterpret_cast<Visitor::visitor_fun_ptr>(&Visitor::visitAccessSpecifier),
  reinterpret_cast<Visitor::visitor_fun_ptr>(&Visitor::visitAsmDefinition),
  reinterpret_cast<Visitor::visitor_fun_ptr>(&Visitor::visitBaseClause),
  reinterpret_cast<Visitor::visitor_fun_ptr>(&Visitor::visitBaseSpecifier),
  reinterpret_cast<Visitor::visitor_fun_ptr>(&Visitor::visitBinaryExpression),
  reinterpret_cast<Visitor::visitor_fun_ptr>(&Visitor::visitCastExpression),
  reinterpret_cast<Visitor::visitor_fun_ptr>(&Visitor::visitClassMemberAccess),
  reinterpret_cast<Visitor::visitor_fun_ptr>(&Visitor::visitClassSpecifier),
  reinterpret_cast<Visitor::visitor_fun_ptr>(&Visitor::visitCompoundStatement),
  reinterpret_cast<Visitor::visitor_fun_ptr>(&Visitor::visitCondition),
  reinterpret_cast<Visitor::visitor_fun_ptr>(&Visitor::visitConditionalExpression),
  reinterpret_cast<Visitor::visitor_fun_ptr>(&Visitor::visitCppCastExpression),
  reinterpret_cast<Visitor::visitor_fun_ptr>(&Visitor::visitCtorInitializer),
  reinterpret_cast<Visitor::visitor_fun_ptr>(&Visitor::visitDeclarationStatement),
  reinterpret_cast<Visitor::visitor_fun_ptr>(&Visitor::visitDeclarator),
  reinterpret_cast<Visitor::visitor_fun_ptr>(&Visitor::visitDeleteExpression),
  reinterpret_cast<Visitor::visitor_fun_ptr>(&Visitor::visitDoStatement),
  reinterpret_cast<Visitor::visitor_fun_ptr>(&Visitor::visitElaboratedTypeSpecifier),
  reinterpret_cast<Visitor::visitor_fun_ptr>(&Visitor::visitEnumSpecifier),
  reinterpret_cast<Visitor::visitor_fun_ptr>(&Visitor::visitEnumerator),
  reinterpret_cast<Visitor::visitor_fun_ptr>(&Visitor::visitExceptionSpecification),
  reinterpret_cast<Visitor::visitor_fun_ptr>(&Visitor::visitExpressionOrDeclarationStatement),
  reinterpret_cast<Visitor::visitor_fun_ptr>(&Visitor::visitExpressionStatement),
  reinterpret_cast<Visitor::visitor_fun_ptr>(&Visitor::visitForStatement),
  reinterpret_cast<Visitor::visitor_fun_ptr>(&Visitor::visitFunctionCall),
  reinterpret_cast<Visitor::visitor_fun_ptr>(&Visitor::visitFunctionDefinition),
  reinterpret_cast<Visitor::visitor_fun_ptr>(&Visitor::visitIfStatement),
  reinterpret_cast<Visitor::visitor_fun_ptr>(&Visitor::visitIncrDecrExpression),
  reinterpret_cast<Visitor::visitor_fun_ptr>(&Visitor::visitInitDeclarator),
  reinterpret_cast<Visitor::visitor_fun_ptr>(&Visitor::visitInitializer),
  reinterpret_cast<Visitor::visitor_fun_ptr>(&Visitor::visitInitializerClause),
  reinterpret_cast<Visitor::visitor_fun_ptr>(&Visitor::visitLabeledStatement),
  reinterpret_cast<Visitor::visitor_fun_ptr>(&Visitor::visitLinkageBody),
  reinterpret_cast<Visitor::visitor_fun_ptr>(&Visitor::visitLinkageSpecification),
  reinterpret_cast<Visitor::visitor_fun_ptr>(&Visitor::visitMemInitializer),
  reinterpret_cast<Visitor::visitor_fun_ptr>(&Visitor::visitName),
  reinterpret_cast<Visitor::visitor_fun_ptr>(&Visitor::visitNamespace),
  reinterpret_cast<Visitor::visitor_fun_ptr>(&Visitor::visitNamespaceAliasDefinition),
  reinterpret_cast<Visitor::visitor_fun_ptr>(&Visitor::visitNewDeclarator),
  reinterpret_cast<Visitor::visitor_fun_ptr>(&Visitor::visitNewExpression),
  reinterpret_cast<Visitor::visitor_fun_ptr>(&Visitor::visitNewInitializer),
  reinterpret_cast<Visitor::visitor_fun_ptr>(&Visitor::visitNewTypeId),
  reinterpret_cast<Visitor::visitor_fun_ptr>(&Visitor::visitOperator),
  reinterpret_cast<Visitor::visitor_fun_ptr>(&Visitor::visitOperatorFunctionId),
  reinterpret_cast<Visitor::visitor_fun_ptr>(&Visitor::visitParameterDeclaration),
  reinterpret_cast<Visitor::visitor_fun_ptr>(&Visitor::visitParameterDeclarationClause),
  reinterpret_cast<Visitor::visitor_fun_ptr>(&Visitor::visitPostfixExpression),
  reinterpret_cast<Visitor::visitor_fun_ptr>(&Visitor::visitPrimaryExpression),
  reinterpret_cast<Visitor::visitor_fun_ptr>(&Visitor::visitPtrOperator),
  reinterpret_cast<Visitor::visitor_fun_ptr>(&Visitor::visitPtrToMember),
  reinterpret_cast<Visitor::visitor_fun_ptr>(&Visitor::visitReturnStatement),
  reinterpret_cast<Visitor::visitor_fun_ptr>(&Visitor::visitSimpleDeclaration),
  reinterpret_cast<Visitor::visitor_fun_ptr>(&Visitor::visitSimpleTypeSpecifier),
  reinterpret_cast<Visitor::visitor_fun_ptr>(&Visitor::visitSizeofExpression),
  reinterpret_cast<Visitor::visitor_fun_ptr>(&Visitor::visitStringLiteral),
  reinterpret_cast<Visitor::visitor_fun_ptr>(&Visitor::visitSubscriptExpression),
  reinterpret_cast<Visitor::visitor_fun_ptr>(&Visitor::visitSwitchStatement),
  reinterpret_cast<Visitor::visitor_fun_ptr>(&Visitor::visitTemplateArgument),
  reinterpret_cast<Visitor::visitor_fun_ptr>(&Visitor::visitTemplateDeclaration),
  reinterpret_cast<Visitor::visitor_fun_ptr>(&Visitor::visitTemplateParameter),
  reinterpret_cast<Visitor::visitor_fun_ptr>(&Visitor::visitThrowExpression),
  reinterpret_cast<Visitor::visitor_fun_ptr>(&Visitor::visitTranslationUnit),
  reinterpret_cast<Visitor::visitor_fun_ptr>(&Visitor::visitTryBlockStatement),
  reinterpret_cast<Visitor::visitor_fun_ptr>(&Visitor::visitTypeId),
  reinterpret_cast<Visitor::visitor_fun_ptr>(&Visitor::visitTypeIdentification),
  reinterpret_cast<Visitor::visitor_fun_ptr>(&Visitor::visitTypeParameter),
  reinterpret_cast<Visitor::visitor_fun_ptr>(&Visitor::visitTypedef),
  reinterpret_cast<Visitor::visitor_fun_ptr>(&Visitor::visitUnaryExpression),
  reinterpret_cast<Visitor::visitor_fun_ptr>(&Visitor::visitUnqualifiedName),
  reinterpret_cast<Visitor::visitor_fun_ptr>(&Visitor::visitUsing),
  reinterpret_cast<Visitor::visitor_fun_ptr>(&Visitor::visitUsingDirective),
  reinterpret_cast<Visitor::visitor_fun_ptr>(&Visitor::visitWhileStatement),
  reinterpret_cast<Visitor::visitor_fun_ptr>(&Visitor::visitWinDeclSpec),
  reinterpret_cast<Visitor::visitor_fun_ptr>(&Visitor::visitQProperty),
  reinterpret_cast<Visitor::visitor_fun_ptr>(&Visitor::visitForwardDeclarationSpecifier),
  reinterpret_cast<Visitor::visitor_fun_ptr>(&Visitor::visitQEnums)
};

Visitor::Visitor()
{
}

Visitor::~Visitor()
{
}

void Visitor::visit(AST *node)
{
  if (node)
    (this->*_S_table[node->kind])(node);
}

// kate: space-indent on; indent-width 2; replace-tabs on;
