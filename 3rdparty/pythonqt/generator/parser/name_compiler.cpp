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


#include "name_compiler.h"
#include "type_compiler.h"
#include "declarator_compiler.h"
#include "lexer.h"
#include "symbol.h"
#include "binder.h"

#include <QtCore/qdebug.h>

NameCompiler::NameCompiler(Binder *binder)
  : _M_binder (binder), _M_token_stream (binder->tokenStream ())
{
}

QString NameCompiler::decode_operator(std::size_t index) const
{
  const Token &tk = _M_token_stream->token((int) index);
  return QString::fromUtf8(&tk.text[tk.position], (int) tk.size);
}

QString NameCompiler::internal_run(AST *node)
{
  _M_name.clear();
  visit(node);
  return name();
}

void NameCompiler::visitUnqualifiedName(UnqualifiedNameAST *node)
{
  QString tmp_name;

  if (node->tilde)
    tmp_name += QLatin1String("~");

  if (node->id)
    tmp_name += _M_token_stream->symbol(node->id)->as_string();

  if (OperatorFunctionIdAST *op_id = node->operator_id)
    {
#if defined(__GNUC__)
#warning "NameCompiler::visitUnqualifiedName() -- implement me"
#endif

      if (op_id->op && op_id->op->op)
        {
          tmp_name += QLatin1String("operator");
          tmp_name += decode_operator(op_id->op->op);
          if (op_id->op->close)
            tmp_name += decode_operator(op_id->op->close);
        }
      else if (op_id->type_specifier)
        {
#if defined(__GNUC__)
#warning "don't use an hardcoded string as cast' name"
#endif
          Token const &tk = _M_token_stream->token ((int) op_id->start_token);
          Token const &end_tk = _M_token_stream->token ((int) op_id->end_token);
          tmp_name += QString::fromLatin1 (&tk.text[tk.position],
                                           (int) (end_tk.position - tk.position)).trimmed ();
      }
    }

  _M_name += tmp_name;
  if (node->template_arguments)
    {
      // ### cleanup
      _M_name.last() += QLatin1String("<");
      visitNodes(this, node->template_arguments);
      _M_name.last().truncate(_M_name.last().count() - 1); // remove the last ','
      _M_name.last() += QLatin1String(">");
    }

}

void NameCompiler::visitTemplateArgument(TemplateArgumentAST *node)
{
  if (node->type_id && node->type_id->type_specifier)
    {
      TypeCompiler type_cc(_M_binder);
      type_cc.run(node->type_id->type_specifier);

      DeclaratorCompiler decl_cc(_M_binder);
      decl_cc.run(node->type_id->declarator);

      if (type_cc.isConstant())
        _M_name.last() += "const ";

      QStringList q = type_cc.qualifiedName ();

      if (q.count () == 1)
        {
#if defined (RXX_RESOLVE_TYPEDEF) // ### it'll break :(
          TypeInfo tp;
          tp.setQualifiedName (q);
          tp = TypeInfo::resolveType (tp, _M_binder->currentScope ()->toItem ());
          q = tp.qualifiedName ();
#endif

          if (CodeModelItem item = _M_binder->model ()->findItem (q, _M_binder->currentScope ()->toItem ()))
            {
              if (item->name () == q.last ())
                q = item->qualifiedName ();
            }
        }

      _M_name.last() += q.join("::");

      if (decl_cc.isReference())
        _M_name.last() += "&";
      if (decl_cc.indirection())
        _M_name.last() += QString(decl_cc.indirection(), '*');

      _M_name.last() += QLatin1String(",");
    }
}

// kate: space-indent on; indent-width 2; replace-tabs on;
