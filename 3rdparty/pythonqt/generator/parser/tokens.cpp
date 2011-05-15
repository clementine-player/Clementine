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


#include <QtCore/qglobal.h>

#include "tokens.h"

static char const * const _S_token_names[] = {
  "K_DCOP",
  "Q_OBJECT",
  "Q_PROPERTY",
  "__attribute__",
  "__typeof",
  "and",
  "and_eq",
  "arrow",
  "asm",
  "assign",
  "auto",
  "bitand",
  "bitor",
  "bool",
  "break",
  "case",
  "catch",
  "char",
  "char_literal",
  "class",
  "comment",
  "compl",
  "concat",
  "const",
  "const_cast",
  "continue",
  "decr",
  "default",
  "delete",
  "do",
  "double",
  "dynamic_cast",
  "ellipsis",
  "else",
  "emit",
  "enum",
  "eq",
  "explicit",
  "export",
  "extern",
  "false",
  "float",
  "for",
  "friend",
  "geq",
  "goto",
  "identifier",
  "if",
  "incr",
  "inline",
  "int",
  "k_dcop",
  "k_dcop_signals",
  "leq",
  "long",
  "mutable",
  "namespace",
  "new",
  "not",
  "not_eq",
  "number_literal",
  "operator",
  "or",
  "or_eq",
  "preproc",
  "private",
  "protected",
  "ptrmem",
  "public",
  "register",
  "reinterpret_cast",
  "return",
  "scope",
  "shift",
  "short",
  "signals",
  "signed",
  "sizeof",
  "slots",
  "static",
  "static_cast",
  "string_literal",
  "struct",
  "switch",
  "template",
  "this",
  "throw",
  "true",
  "try",
  "typedef",
  "typeid",
  "typename",
  "union",
  "unsigned",
  "using",
  "virtual",
  "void",
  "volatile",
  "wchar_t",
  "while",
  "whitespaces",
  "xor",
  "xor_eq",
  "Q_ENUMS"
};

static char _S_printable[][2] = {
  { char(32), '\0' },
  { char(33), '\0' },
  { char(34), '\0' },
  { char(35), '\0' },
  { char(36), '\0' },
  { char(37), '\0' },
  { char(38), '\0' },
  { char(39), '\0' },
  { char(40), '\0' },
  { char(41), '\0' },
  { char(42), '\0' },
  { char(43), '\0' },
  { char(44), '\0' },
  { char(45), '\0' },
  { char(46), '\0' },
  { char(47), '\0' },
  { char(48), '\0' },
  { char(49), '\0' },
  { char(50), '\0' },
  { char(51), '\0' },
  { char(52), '\0' },
  { char(53), '\0' },
  { char(54), '\0' },
  { char(55), '\0' },
  { char(56), '\0' },
  { char(57), '\0' },
  { char(58), '\0' },
  { char(59), '\0' },
  { char(60), '\0' },
  { char(61), '\0' },
  { char(62), '\0' },
  { char(63), '\0' },
  { char(64), '\0' },
  { char(65), '\0' },
  { char(66), '\0' },
  { char(67), '\0' },
  { char(68), '\0' },
  { char(69), '\0' },
  { char(70), '\0' },
  { char(71), '\0' },
  { char(72), '\0' },
  { char(73), '\0' },
  { char(74), '\0' },
  { char(75), '\0' },
  { char(76), '\0' },
  { char(77), '\0' },
  { char(78), '\0' },
  { char(79), '\0' },
  { char(80), '\0' },
  { char(81), '\0' },
  { char(82), '\0' },
  { char(83), '\0' },
  { char(84), '\0' },
  { char(85), '\0' },
  { char(86), '\0' },
  { char(87), '\0' },
  { char(88), '\0' },
  { char(89), '\0' },
  { char(90), '\0' },
  { char(91), '\0' },
  { char(92), '\0' },
  { char(93), '\0' },
  { char(94), '\0' },
  { char(95), '\0' },
  { char(96), '\0' },
  { char(97), '\0' },
  { char(98), '\0' },
  { char(99), '\0' },
  { char(100), '\0' },
  { char(101), '\0' },
  { char(102), '\0' },
  { char(103), '\0' },
  { char(104), '\0' },
  { char(105), '\0' },
  { char(106), '\0' },
  { char(107), '\0' },
  { char(108), '\0' },
  { char(109), '\0' },
  { char(110), '\0' },
  { char(111), '\0' },
  { char(112), '\0' },
  { char(113), '\0' },
  { char(114), '\0' },
  { char(115), '\0' },
  { char(116), '\0' },
  { char(117), '\0' },
  { char(118), '\0' },
  { char(119), '\0' },
  { char(120), '\0' },
  { char(121), '\0' },
  { char(122), '\0' },
  { char(123), '\0' },
  { char(124), '\0' },
  { char(125), '\0' },
  { char(126), '\0' },
  { char(127), '\0' },
};

char const *token_name(int token)
{
  if (token == 0)
    {
      return "eof";
    }
  else if (token >= 32 && token <= 127)
    {
      return _S_printable[token - 32];
    }
  else if (token >= 1000)
    {
      return _S_token_names[token - 1000];
    }

  Q_ASSERT(0);
  return 0;
}

// kate: space-indent on; indent-width 2; replace-tabs on;
