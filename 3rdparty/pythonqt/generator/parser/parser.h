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


#ifndef PARSER_H
#define PARSER_H

#include "ast.h"
#include "lexer.h"

#include <QtCore/QString>

class FileSymbol;
class Control;

class Parser
{
public:
  Parser(Control *control);
  ~Parser();

  LocationManager &location() { return _M_location; }

  TranslationUnitAST *parse(const char *contents, std::size_t size, pool *p);

private:
  void reportError(const QString& msg);
  void syntaxError();
  void tokenRequiredError(int expected);

public:
  bool skipFunctionBody(StatementAST *&node);

public:
  bool parse_Attribute__();
  bool parseAbstractDeclarator(DeclaratorAST *&node);
  bool parseAccessSpecifier(DeclarationAST *&node);
  bool parseAdditiveExpression(ExpressionAST *&node);
  bool parseAndExpression(ExpressionAST *&node, bool templArgs = false);
  bool parseAsmDefinition(DeclarationAST *&node);
  bool parseAssignmentExpression(ExpressionAST *&node);
  bool parseBaseClause(BaseClauseAST *&node);
  bool parseBaseSpecifier(BaseSpecifierAST *&node);
  bool parseBlockDeclaration(DeclarationAST *&node);
  bool parseCastExpression(ExpressionAST *&node);
  bool parseClassSpecifier(TypeSpecifierAST *&node);
  bool parseForwardDeclarationSpecifier(TypeSpecifierAST *&node);
  bool parseCommaExpression(ExpressionAST *&node);
  bool parseCompoundStatement(StatementAST *&node);
  bool parseCondition(ConditionAST *&node, bool initRequired = true);
  bool parseConditionalExpression(ExpressionAST *&node);
  bool parseConstantExpression(ExpressionAST *&node);
  bool parseCtorInitializer(CtorInitializerAST *&node);
  bool parseCvQualify(const ListNode<std::size_t> *&node);
  bool parseDeclaration(DeclarationAST *&node);
  bool parseDeclarationInternal(DeclarationAST *&node);
  bool parseDeclarationStatement(StatementAST *&node);
  bool parseDeclarator(DeclaratorAST *&node);
  bool parseDeleteExpression(ExpressionAST *&node);
  bool parseDoStatement(StatementAST *&node);
  bool parseElaboratedTypeSpecifier(TypeSpecifierAST *&node);
  bool parseEnumSpecifier(TypeSpecifierAST *&node);
  bool parseEnumerator(EnumeratorAST *&node);
  bool parseEqualityExpression(ExpressionAST *&node,
			       bool templArgs = false);
  bool parseExceptionSpecification(ExceptionSpecificationAST *&node);
  bool parseExclusiveOrExpression(ExpressionAST *&node,
				  bool templArgs = false);
  bool parseExpression(ExpressionAST *&node);
  bool parseExpressionOrDeclarationStatement(StatementAST *&node);
  bool parseExpressionStatement(StatementAST *&node);
  bool parseForInitStatement(StatementAST *&node);
  bool parseForStatement(StatementAST *&node);
  bool parseFunctionBody(StatementAST *&node);
  bool parseFunctionSpecifier(const ListNode<std::size_t> *&node);
  bool parseIfStatement(StatementAST *&node);
  bool parseInclusiveOrExpression(ExpressionAST *&node,
				  bool templArgs = false);
  bool parseInitDeclarator(InitDeclaratorAST *&node);
  bool parseInitDeclaratorList(const ListNode<InitDeclaratorAST*> *&node);
  bool parseInitializer(InitializerAST *&node);
  bool parseInitializerClause(InitializerClauseAST *&node);
  bool parseLabeledStatement(StatementAST *&node);
  bool parseLinkageBody(LinkageBodyAST *&node);
  bool parseLinkageSpecification(DeclarationAST *&node);
  bool parseLogicalAndExpression(ExpressionAST *&node,
				 bool templArgs = false);
  bool parseLogicalOrExpression(ExpressionAST *&node,
				bool templArgs = false);
  bool parseMemInitializer(MemInitializerAST *&node);
  bool parseMemInitializerList(const ListNode<MemInitializerAST*> *&node);
  bool parseMemberSpecification(DeclarationAST *&node);
  bool parseMultiplicativeExpression(ExpressionAST *&node);
  bool parseName(NameAST *&node, bool acceptTemplateId = false);
  bool parseNamespace(DeclarationAST *&node);
  bool parseNamespaceAliasDefinition(DeclarationAST *&node);
  bool parseNewDeclarator(NewDeclaratorAST *&node);
  bool parseNewExpression(ExpressionAST *&node);
  bool parseNewInitializer(NewInitializerAST *&node);
  bool parseNewTypeId(NewTypeIdAST *&node);
  bool parseOperator(OperatorAST *&node);
  bool parseOperatorFunctionId(OperatorFunctionIdAST *&node);
  bool parseParameterDeclaration(ParameterDeclarationAST *&node);
  bool parseParameterDeclarationClause(ParameterDeclarationClauseAST *&node);
  bool parseParameterDeclarationList(const ListNode<ParameterDeclarationAST*> *&node);
  bool parsePmExpression(ExpressionAST *&node);
  bool parsePostfixExpression(ExpressionAST *&node);
  bool parsePostfixExpressionInternal(ExpressionAST *&node);
  bool parsePrimaryExpression(ExpressionAST *&node);
  bool parsePtrOperator(PtrOperatorAST *&node);
  bool parsePtrToMember(PtrToMemberAST *&node);
  bool parseRelationalExpression(ExpressionAST *&node,
				 bool templArgs = false);
  bool parseShiftExpression(ExpressionAST *&node);
  bool parseSimpleTypeSpecifier(TypeSpecifierAST *&node,
				bool onlyIntegral = false);
  bool parseStatement(StatementAST *&node);
  bool parseStorageClassSpecifier(const ListNode<std::size_t> *&node);
  bool parseStringLiteral(StringLiteralAST *&node);
  bool parseSwitchStatement(StatementAST *&node);
  bool parseTemplateArgument(TemplateArgumentAST *&node);
  bool parseTemplateArgumentList(const ListNode<TemplateArgumentAST*> *&node,
				 bool reportError = true);
  bool parseTemplateDeclaration(DeclarationAST *&node);
  bool parseTemplateParameter(TemplateParameterAST *&node);
  bool parseTemplateParameterList(const ListNode<TemplateParameterAST*> *&node);
  bool parseThrowExpression(ExpressionAST *&node);
  bool parseTranslationUnit(TranslationUnitAST *&node);
  bool parseTryBlockStatement(StatementAST *&node);
  bool parseTypeId(TypeIdAST *&node);
  bool parseTypeIdList(const ListNode<TypeIdAST*> *&node);
  bool parseTypeParameter(TypeParameterAST *&node);
  bool parseTypeSpecifier(TypeSpecifierAST *&node);
  bool parseTypeSpecifierOrClassSpec(TypeSpecifierAST *&node);
  bool parseTypedef(DeclarationAST *&node);
  bool parseUnaryExpression(ExpressionAST *&node);
  bool parseUnqualifiedName(UnqualifiedNameAST *&node,
			    bool parseTemplateId = true);
  bool parseUsing(DeclarationAST *&node);
  bool parseUsingDirective(DeclarationAST *&node);
  bool parseWhileStatement(StatementAST *&node);
  bool parseWinDeclSpec(WinDeclSpecAST *&node);

  bool parseQ_PROPERTY(DeclarationAST *&node);
  bool parseQ_ENUMS(DeclarationAST *&node);

  bool skipUntil(int token);
  bool skipUntilDeclaration();
  bool skipUntilStatement();
  bool skip(int l, int r);

  void advance();

  // private:
  TokenStream token_stream;
  LocationTable location_table;
  LocationTable line_table;

  bool block_errors(bool block);

private:
  QString tokenText(AST *) const;

  LocationManager _M_location;
  Control *control;
  Lexer lexer;
  pool *_M_pool;
  bool _M_block_errors;

private:
  Parser(const Parser& source);
  void operator = (const Parser& source);
};

#endif

// kate: space-indent on; indent-width 2; replace-tabs on;
