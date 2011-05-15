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


#include "lexer.h"
#include "tokens.h"
#include "control.h"

#include <cctype>
#include <iostream>

scan_fun_ptr Lexer::s_scan_keyword_table[] = {
  &Lexer::scanKeyword0, &Lexer::scanKeyword0,
  &Lexer::scanKeyword2, &Lexer::scanKeyword3,
  &Lexer::scanKeyword4, &Lexer::scanKeyword5,
  &Lexer::scanKeyword6, &Lexer::scanKeyword7,
  &Lexer::scanKeyword8, &Lexer::scanKeyword9,
  &Lexer::scanKeyword10, &Lexer::scanKeyword11,
  &Lexer::scanKeyword12, &Lexer::scanKeyword13,
  &Lexer::scanKeyword14, &Lexer::scanKeyword0,
  &Lexer::scanKeyword16
};

void LocationManager::extract_line(int offset, int *line, QString *filename) const
{
  *line = 0;
  if (token_stream.size () < 1)
    return;

  const unsigned char *begin_buffer = reinterpret_cast<const unsigned char *>(token_stream[0].text);
  const unsigned char *cursor = begin_buffer + offset;

  ++cursor; // skip '#'
  if (std::isspace(*cursor) && std::isdigit(*(cursor + 1)))
    {
      ++cursor;
      char buffer[1024], *cp = buffer;
      do {
        *cp++ = *cursor++;
      } while (std::isdigit(*cursor));
      *cp = '\0';
      int l = strtol(buffer, 0, 0);

      Q_ASSERT(std::isspace(*cursor));
      ++cursor;

      Q_ASSERT(*cursor == '"');
      ++cursor;

      cp = buffer;
      while (*cursor && *cursor != '"') {
        *cp++ = *cursor++;
      }
      *cp = '\0';
      Q_ASSERT(*cursor == '"');
      ++cursor;

      *filename = buffer;
      *line = l;
      // printf("filename: %s line: %d\n", buffer, line);
    }
}

void LocationManager::positionAt(std::size_t offset, int *line, int *column,
                                 QString *filename) const
{
  int ppline, ppcolumn;
  line_table.positionAt(offset, &ppline, &ppcolumn);

  int base_line;
  extract_line((int) line_table[ppline-1], &base_line, filename);

  int line2, column2;
  location_table.positionAt((int) line_table[ppline-1], &line2, &column2);

  location_table.positionAt(offset, line, column);
  *line = base_line + *line - line2  - 1;
}

scan_fun_ptr Lexer::s_scan_table[256];
bool Lexer::s_initialized = false;

void Lexer::tokenize(const char *contents, std::size_t size)
{
  if (!s_initialized)
    initialize_scan_table();

  token_stream.resize(1024);
  token_stream[0].kind = Token_EOF;
  token_stream[0].text = contents;

  index = 1;

  cursor = (const unsigned char *) contents;
  begin_buffer = (const unsigned char *) contents;
  end_buffer = cursor + size;

  location_table.resize(1024);
  location_table[0] = 0;
  location_table.current_line = 1;

  line_table.resize(1024);
  line_table[0] = 0;
  line_table.current_line = 1;

  do {
    if (index == token_stream.size())
      token_stream.resize(token_stream.size() * 2);

    Token *current_token = &token_stream[(int) index];
    current_token->text = reinterpret_cast<const char*>(begin_buffer);
    current_token->position = cursor - begin_buffer;
    (this->*s_scan_table[*cursor])();
    current_token->size = cursor - begin_buffer - current_token->position;
  } while (cursor < end_buffer);

  if (index == token_stream.size())
      token_stream.resize(token_stream.size() * 2);

  Q_ASSERT(index < token_stream.size());
  token_stream[(int) index].position = cursor - begin_buffer;
  token_stream[(int) index].kind = Token_EOF;
}

void Lexer::reportError(const QString& msg)
{
    int line, column;
    QString fileName;

    std::size_t tok = token_stream.cursor();
    _M_location.positionAt(token_stream.position(tok),
        &line, &column, &fileName);

    Control::ErrorMessage errmsg;
    errmsg.setLine(line + 1);
    errmsg.setColumn(column);
    errmsg.setFileName(fileName);
    errmsg.setMessage(QLatin1String("** LEXER ERROR ") + msg);
    control->reportError(errmsg);
}

void Lexer::initialize_scan_table()
{
  s_initialized = true;

  for (int i=0; i<256; ++i)
    {
      if (isspace(i))
	s_scan_table[i] = &Lexer::scan_white_spaces;
      else if (isalpha(i) || i == '_')
	s_scan_table[i] = &Lexer::scan_identifier_or_keyword;
      else if (isdigit(i))
	s_scan_table[i] = &Lexer::scan_int_constant;
      else
	s_scan_table[i] = &Lexer::scan_invalid_input;
    }

  s_scan_table[int('L')] = &Lexer::scan_identifier_or_literal;
  s_scan_table[int('\n')] = &Lexer::scan_newline;
  s_scan_table[int('#')] = &Lexer::scan_preprocessor;

  s_scan_table[int('\'')] = &Lexer::scan_char_constant;
  s_scan_table[int('"')]  = &Lexer::scan_string_constant;

  s_scan_table[int('.')] = &Lexer::scan_int_constant;

  s_scan_table[int('!')] = &Lexer::scan_not;
  s_scan_table[int('%')] = &Lexer::scan_remainder;
  s_scan_table[int('&')] = &Lexer::scan_and;
  s_scan_table[int('(')] = &Lexer::scan_left_paren;
  s_scan_table[int(')')] = &Lexer::scan_right_paren;
  s_scan_table[int('*')] = &Lexer::scan_star;
  s_scan_table[int('+')] = &Lexer::scan_plus;
  s_scan_table[int(',')] = &Lexer::scan_comma;
  s_scan_table[int('-')] = &Lexer::scan_minus;
  s_scan_table[int('/')] = &Lexer::scan_divide;
  s_scan_table[int(':')] = &Lexer::scan_colon;
  s_scan_table[int(';')] = &Lexer::scan_semicolon;
  s_scan_table[int('<')] = &Lexer::scan_less;
  s_scan_table[int('=')] = &Lexer::scan_equal;
  s_scan_table[int('>')] = &Lexer::scan_greater;
  s_scan_table[int('?')] = &Lexer::scan_question;
  s_scan_table[int('[')] = &Lexer::scan_left_bracket;
  s_scan_table[int(']')] = &Lexer::scan_right_bracket;
  s_scan_table[int('^')] = &Lexer::scan_xor;
  s_scan_table[int('{')] = &Lexer::scan_left_brace;
  s_scan_table[int('|')] = &Lexer::scan_or;
  s_scan_table[int('}')] = &Lexer::scan_right_brace;
  s_scan_table[int('~')] = &Lexer::scan_tilde;

  s_scan_table[0] = &Lexer::scan_EOF;
}

void Lexer::scan_preprocessor()
{
  if (line_table.current_line == line_table.size())
    line_table.resize(line_table.current_line * 2);

  line_table[(int) line_table.current_line++] = (cursor - begin_buffer);

  while (*cursor && *cursor != '\n')
    ++cursor;

  if (*cursor != '\n')
      reportError("expected newline");
}

void Lexer::scan_char_constant()
{
  const unsigned char *begin = cursor;

  ++cursor;
  while (*cursor && *cursor != '\'')
    {
      if (*cursor == '\n')
        reportError("did not expect newline");

      if (*cursor == '\\')
	++cursor;
      ++cursor;
    }

    if (*cursor != '\'')
      reportError("expected \'");

  ++cursor;

  token_stream[(int) index].extra.symbol =
    control->findOrInsertName((const char*) begin, cursor - begin);

  token_stream[(int) index++].kind = Token_char_literal;
}

void Lexer::scan_string_constant()
{
  const unsigned char *begin = cursor;

  ++cursor;
  while (*cursor && *cursor != '"')
    {
      if (*cursor == '\n')
        reportError("did not expect newline");

      if (*cursor == '\\')
	++cursor;
      ++cursor;
    }

  if (*cursor != '"')
    reportError("expected \"");

  ++cursor;

  token_stream[(int) index].extra.symbol =
    control->findOrInsertName((const char*) begin, cursor - begin);

  token_stream[(int) index++].kind = Token_string_literal;
}

void Lexer::scan_newline()
{
  if (location_table.current_line == location_table.size())
    location_table.resize(location_table.current_line * 2);

  location_table[(int) location_table.current_line++] = (cursor - begin_buffer);
  ++cursor;
}

void Lexer::scan_white_spaces()
{
  while (isspace(*cursor))
    {
      if (*cursor == '\n')
	scan_newline();
      else
	++cursor;
    }
}

void Lexer::scan_identifier_or_literal()
{
  switch (*(cursor + 1))
    {
    case '\'':
      ++cursor;
      scan_char_constant();
      break;

    case '\"':
      ++cursor;
      scan_string_constant();
      break;

    default:
      scan_identifier_or_keyword();
      break;
    }
}

void Lexer::scan_identifier_or_keyword()
{
  const unsigned char *skip = cursor;
  while (isalnum(*skip) || *skip== '_')
    ++skip;

  int n = skip - cursor;
  Token *current_token = &token_stream[(int) index];
  (this->*s_scan_keyword_table[n < 17 ? n : 0])();

  if (current_token->kind == Token_identifier)
    {
      current_token->extra.symbol =
	control->findOrInsertName((const char*) cursor, n);
    }

  cursor = skip;
}

void Lexer::scan_int_constant()
{
  if (*cursor == '.' && !std::isdigit(*(cursor + 1)))
    {
      scan_dot();
      return;
    }

  const unsigned char *begin = cursor;

  while (isalnum(*cursor) || *cursor == '.')
    ++cursor;

  token_stream[(int) index].extra.symbol =
    control->findOrInsertName((const char*) begin, cursor - begin);

  token_stream[(int) index++].kind = Token_number_literal;
}

void Lexer::scan_not()
{
  /*
    '!'		::= not
    '!='		::= not_equal
  */

  ++cursor;

  if (*cursor == '=')
    {
      ++cursor;
      token_stream[(int) index++].kind = Token_not_eq;
    }
  else
    {
      token_stream[(int) index++].kind = '!';
    }
}

void Lexer::scan_remainder()
{
  /*
    '%'		::= remainder
    '%='		::= remainder_equal
  */

  ++cursor;

  if (*cursor == '=')
    {
      ++cursor;
      token_stream[(int) index++].kind = Token_assign;
    }
  else
    {
      token_stream[(int) index++].kind = '%';
    }
}

void Lexer::scan_and()
{
  /*
    '&&'		::= and_and
    '&'		::= and
    '&='		::= and_equal
  */

  ++cursor;
  if (*cursor == '=')
    {
      ++cursor;
      token_stream[(int) index++].kind = Token_assign;
    }
  else if (*cursor == '&')
    {
      ++cursor;
      token_stream[(int) index++].kind = Token_and;
    }
  else
    {
      token_stream[(int) index++].kind = '&';
    }
}

void Lexer::scan_left_paren()
{
  ++cursor;
  token_stream[(int) index++].kind = '(';
}

void Lexer::scan_right_paren()
{
  ++cursor;
  token_stream[(int) index++].kind = ')';
}

void Lexer::scan_star()
{
  /*
    '*'		::= star
    '*='		::= star_equal
  */

  ++cursor;

  if (*cursor == '=')
    {
      ++cursor;
      token_stream[(int) index++].kind = Token_assign;
    }
  else
    {
      token_stream[(int) index++].kind = '*';
    }
}

void Lexer::scan_plus()
{
  /*
    '+'		::= plus
    '++'		::= incr
    '+='		::= plus_equal
  */

  ++cursor;
  if (*cursor == '=')
    {
      ++cursor;
      token_stream[(int) index++].kind = Token_assign;
    }
  else if (*cursor == '+')
    {
      ++cursor;
      token_stream[(int) index++].kind = Token_incr;
    }
  else
    {
      token_stream[(int) index++].kind = '+';
    }
}

void Lexer::scan_comma()
{
  ++cursor;
  token_stream[(int) index++].kind = ',';
}

void Lexer::scan_minus()
{
  /*
    '-'		::= minus
    '--'		::= decr
    '-='		::= minus_equal
    '->'		::= left_arrow
  */

  ++cursor;
  if (*cursor == '=')
    {
      ++cursor;
      token_stream[(int) index++].kind = Token_assign;
    }
  else if (*cursor == '-')
    {
      ++cursor;
      token_stream[(int) index++].kind = Token_decr;
    }
  else if (*cursor == '>')
    {
      ++cursor;
      token_stream[(int) index++].kind = Token_arrow;
      if (*cursor == '*')
	{
	  ++cursor;
	  token_stream[(int) index++].kind = Token_ptrmem;
	}
    }
  else
    {
      token_stream[(int) index++].kind = '-';
    }
}

void Lexer::scan_dot()
{
  /*
    '.'		::= dot
    '...'		::= ellipsis
  */

  ++cursor;
  if (*cursor == '.' && *(cursor + 1) == '.')
    {
      cursor += 2;
      token_stream[(int) index++].kind = Token_ellipsis;
    }
  else if (*cursor == '.' && *(cursor + 1) == '*')
    {
      cursor += 2;
      token_stream[(int) index++].kind = Token_ptrmem;
    }
  else
    token_stream[(int) index++].kind = '.';
}

void Lexer::scan_divide()
{
  /*
    '/'		::= divide
    '/='	::= divide_equal
  */

  ++cursor;

  if (*cursor == '=')
    {
      ++cursor;
      token_stream[(int) index++].kind = Token_assign;
    }
  else
    {
      token_stream[(int) index++].kind = '/';
    }
}

void Lexer::scan_colon()
{
  ++cursor;
  if (*cursor == ':')
    {
      ++cursor;
      token_stream[(int) index++].kind = Token_scope;
    }
  else
    {
      token_stream[(int) index++].kind = ':';
    }
}

void Lexer::scan_semicolon()
{
  ++cursor;
  token_stream[(int) index++].kind = ';';
}

void Lexer::scan_less()
{
  /*
    '<'			::= less
    '<<'		::= left_shift
    '<<='		::= left_shift_equal
    '<='		::= less_equal
  */

  ++cursor;
  if (*cursor == '=')
    {
      ++cursor;
      token_stream[(int) index++].kind = Token_leq;
    }
  else if (*cursor == '<')
    {
      ++cursor;
      if (*cursor == '=')
	{
	  ++cursor;
	  token_stream[(int) index++].kind = Token_assign;
	}
      else
	{
	  token_stream[(int) index++].kind = Token_shift;
	}
    }
  else
    {
      token_stream[(int) index++].kind = '<';
    }
}

void Lexer::scan_equal()
{
  /*
    '='			::= equal
    '=='		::= equal_equal
  */
  ++cursor;

  if (*cursor == '=')
    {
      ++cursor;
      token_stream[(int) index++].kind = Token_eq;
    }
  else
    {
      token_stream[(int) index++].kind = '=';
    }
}

void Lexer::scan_greater()
{
  /*
    '>'			::= greater
    '>='		::= greater_equal
    '>>'		::= right_shift
    '>>='		::= right_shift_equal
  */

  ++cursor;
  if (*cursor == '=')
    {
      ++cursor;
      token_stream[(int) index++].kind = Token_geq;
    }
  else if (*cursor == '>')
    {
      ++cursor;
      if (*cursor == '=')
	{
	  ++cursor;
	  token_stream[(int) index++].kind = Token_assign;
	}
      else
	{
	  token_stream[(int) index++].kind = Token_shift;
	}
    }
  else
    {
      token_stream[(int) index++].kind = '>';
    }
}

void Lexer::scan_question()
{
  ++cursor;
  token_stream[(int) index++].kind = '?';
}

void Lexer::scan_left_bracket()
{
  ++cursor;
  token_stream[(int) index++].kind = '[';
}

void Lexer::scan_right_bracket()
{
  ++cursor;
  token_stream[(int) index++].kind = ']';
}

void Lexer::scan_xor()
{
  /*
    '^'			::= xor
    '^='		::= xor_equal
  */
  ++cursor;

  if (*cursor == '=')
    {
      ++cursor;
      token_stream[(int) index++].kind = Token_assign;
    }
  else
    {
      token_stream[(int) index++].kind = '^';
    }
}

void Lexer::scan_left_brace()
{
  ++cursor;
  token_stream[(int) index++].kind = '{';
}

void Lexer::scan_or()
{
  /*
    '|'			::= or
    '|='		::= or_equal
    '||'		::= or_or
  */
  ++cursor;
  if (*cursor == '=')
    {
      ++cursor;
      token_stream[(int) index++].kind = Token_assign;
    }
  else if (*cursor == '|')
    {
      ++cursor;
      token_stream[(int) index++].kind = Token_or;
    }
  else
    {
    token_stream[(int) index++].kind = '|';
  }
}

void Lexer::scan_right_brace()
{
  ++cursor;
  token_stream[(int) index++].kind = '}';
}

void Lexer::scan_tilde()
{
  ++cursor;
  token_stream[(int) index++].kind = '~';
}

void Lexer::scan_EOF()
{
  ++cursor;
  token_stream[(int) index++].kind = Token_EOF;
}

void Lexer::scan_invalid_input()
{
  QString errmsg("invalid input: %1");
  errmsg.arg(int(*cursor));
  reportError(errmsg);
  ++cursor;
}

void LocationTable::positionAt(std::size_t offset, int max_line,
			       int *line, int *column) const
{
  if (!(line && column && max_line != 0))
    return;

  int first = 0;
  int len = max_line;
  int half;
  int middle;

  while (len > 0)
    {
      half = len >> 1;
      middle = first;

      middle += half;

      if (lines[middle] < offset)
	{
	  first = middle;
	  ++first;
	  len = len - half - 1;
	}
      else
	len = half;
    }

  *line = std::max(first, 1);
  *column = (int) (offset - lines[*line - 1] - 1);

  if (*column < 0)
    {
      *column = 0;
    }
}

void Lexer::scanKeyword0()
{
  token_stream[(int) index++].kind = Token_identifier;
}

void Lexer::scanKeyword2()
{
  switch (*cursor)
    {
    case 'i':
      if (*(cursor + 1) == 'f')
	{
	  token_stream[(int) index++].kind = Token_if;
	  return;
	}
      break;

    case 'd':
      if (*(cursor + 1) == 'o')
	{
	  token_stream[(int) index++].kind = Token_do;
	  return;
	}
      break;

    case 'o':
      if (*(cursor + 1) == 'r')
	{
	  token_stream[(int) index++].kind = Token_or;
	  return;
	}
      break;

    }
  token_stream[(int) index++].kind = Token_identifier;
}

void Lexer::scanKeyword3()
{
  switch (*cursor)
    {
    case 'a':
      if (*(cursor + 1) == 'n' &&
	  *(cursor + 2) == 'd')
	{
	  token_stream[(int) index++].kind = Token_and;
	  return;
	}
      if (*(cursor + 1) == 's' &&
	  *(cursor + 2) == 'm')
	{
	  token_stream[(int) index++].kind = Token_asm;
	  return;
	}
      break;

    case 'f':
      if (*(cursor + 1) == 'o' &&
	  *(cursor + 2) == 'r')
	{
	  token_stream[(int) index++].kind = Token_for;
	  return;
	}
      break;

    case 'i':
      if (*(cursor + 1) == 'n' &&
	  *(cursor + 2) == 't')
	{
	  token_stream[(int) index++].kind = Token_int;
	  return;
	}
      break;

    case 'n':
      if (*(cursor + 1) == 'e' &&
	  *(cursor + 2) == 'w')
	{
	  token_stream[(int) index++].kind = Token_new;
	  return;
	}
      if (*(cursor + 1) == 'o' &&
	  *(cursor + 2) == 't')
	{
	  token_stream[(int) index++].kind = Token_not;
	  return;
	}
      break;

    case 't':
      if (*(cursor + 1) == 'r' &&
	  *(cursor + 2) == 'y')
	{
	  token_stream[(int) index++].kind = Token_try;
	  return;
	}
      break;

    case 'x':
      if (*(cursor + 1) == 'o' &&
	  *(cursor + 2) == 'r')
	{
	  token_stream[(int) index++].kind = Token_xor;
	  return;
	}
      break;

    }
  token_stream[(int) index++].kind = Token_identifier;
}

void Lexer::scanKeyword4()
{
  switch (*cursor)
    {
    case 'a':
      if (*(cursor + 1) == 'u' &&
	  *(cursor + 2) == 't' &&
	  *(cursor + 3) == 'o')
	{
	  token_stream[(int) index++].kind = Token_auto;
	  return;
	}
      break;

    case 'c':
      if (*(cursor + 1) == 'a' &&
	  *(cursor + 2) == 's' &&
	  *(cursor + 3) == 'e')
	{
	  token_stream[(int) index++].kind = Token_case;
	  return;
	}
      if (*(cursor + 1) == 'h' &&
	  *(cursor + 2) == 'a' &&
	  *(cursor + 3) == 'r')
	{
	  token_stream[(int) index++].kind = Token_char;
	  return;
	}
      break;

    case 'b':
      if (*(cursor + 1) == 'o' &&
	  *(cursor + 2) == 'o' &&
	  *(cursor + 3) == 'l')
	{
	  token_stream[(int) index++].kind = Token_bool;
	  return;
	}
      break;

    case 'e':
      if (*(cursor + 1) == 'l' &&
	  *(cursor + 2) == 's' &&
	  *(cursor + 3) == 'e')
	{
	  token_stream[(int) index++].kind = Token_else;
	  return;
	}
      if (*(cursor + 1) == 'm' &&
	  *(cursor + 2) == 'i' &&
	  *(cursor + 3) == 't')
	{
	  token_stream[(int) index++].kind = Token_emit;
	  return;
	}
      if (*(cursor + 1) == 'n' &&
	  *(cursor + 2) == 'u' &&
	  *(cursor + 3) == 'm')
	{
	  token_stream[(int) index++].kind = Token_enum;
	  return;
	}
      break;

    case 'g':
      if (*(cursor + 1) == 'o' &&
	  *(cursor + 2) == 't' &&
	  *(cursor + 3) == 'o')
	{
	  token_stream[(int) index++].kind = Token_goto;
	  return;
	}
      break;

    case 'l':
      if (*(cursor + 1) == 'o' &&
	  *(cursor + 2) == 'n' &&
	  *(cursor + 3) == 'g')
	{
	  token_stream[(int) index++].kind = Token_long;
	  return;
	}
      break;

    case 't':
      if (*(cursor + 1) == 'h' &&
	  *(cursor + 2) == 'i' &&
	  *(cursor + 3) == 's')
	{
	  token_stream[(int) index++].kind = Token_this;
	  return;
	}
      break;

    case 'v':
      if (*(cursor + 1) == 'o' &&
	  *(cursor + 2) == 'i' &&
	  *(cursor + 3) == 'd')
	{
	  token_stream[(int) index++].kind = Token_void;
	  return;
	}
      break;

    }
  token_stream[(int) index++].kind = Token_identifier;
}

void Lexer::scanKeyword5()
{
  switch (*cursor)
    {
    case 'c':
      if (*(cursor + 1) == 'a' &&
	  *(cursor + 2) == 't' &&
	  *(cursor + 3) == 'c' &&
	  *(cursor + 4) == 'h')
	{
	  token_stream[(int) index++].kind = Token_catch;
	  return;
	}
      if (*(cursor + 1) == 'l' &&
	  *(cursor + 2) == 'a' &&
	  *(cursor + 3) == 's' &&
	  *(cursor + 4) == 's')
	{
	  token_stream[(int) index++].kind = Token_class;
	  return;
	}
      if (*(cursor + 1) == 'o' &&
	  *(cursor + 2) == 'm' &&
	  *(cursor + 3) == 'p' &&
	  *(cursor + 4) == 'l')
	{
	  token_stream[(int) index++].kind = Token_compl;
	  return;
	}
      if (*(cursor + 1) == 'o' &&
	  *(cursor + 2) == 'n' &&
	  *(cursor + 3) == 's' &&
	  *(cursor + 4) == 't')
	{
	  token_stream[(int) index++].kind = Token_const;
	  return;
	}
      break;

    case 'b':
      if (*(cursor + 1) == 'i' &&
	  *(cursor + 2) == 't' &&
	  *(cursor + 3) == 'o' &&
	  *(cursor + 4) == 'r')
	{
	  token_stream[(int) index++].kind = Token_bitor;
	  return;
	}
      if (*(cursor + 1) == 'r' &&
	  *(cursor + 2) == 'e' &&
	  *(cursor + 3) == 'a' &&
	  *(cursor + 4) == 'k')
	{
	  token_stream[(int) index++].kind = Token_break;
	  return;
	}
      break;

    case 'f':
      if (*(cursor + 1) == 'l' &&
	  *(cursor + 2) == 'o' &&
	  *(cursor + 3) == 'a' &&
	  *(cursor + 4) == 't')
	{
	  token_stream[(int) index++].kind = Token_float;
	  return;
	}
      break;

    case 'o':
      if (*(cursor + 1) == 'r' &&
	  *(cursor + 2) == '_' &&
	  *(cursor + 3) == 'e' &&
	  *(cursor + 4) == 'q')
	{
	  token_stream[(int) index++].kind = Token_or_eq;
	  return;
	}
      break;

    case 's':
      if (*(cursor + 1) == 'h' &&
	  *(cursor + 2) == 'o' &&
	  *(cursor + 3) == 'r' &&
	  *(cursor + 4) == 't')
	{
	  token_stream[(int) index++].kind = Token_short;
	  return;
	}
      if (*(cursor + 1) == 'l' &&
	  *(cursor + 2) == 'o' &&
	  *(cursor + 3) == 't' &&
	  *(cursor + 4) == 's')
	{
	  token_stream[(int) index++].kind = Token_slots;
	  return;
	}
      break;

    case 'u':
      if (*(cursor + 1) == 'n' &&
	  *(cursor + 2) == 'i' &&
	  *(cursor + 3) == 'o' &&
	  *(cursor + 4) == 'n')
	{
	  token_stream[(int) index++].kind = Token_union;
	  return;
	}
      if (*(cursor + 1) == 's' &&
	  *(cursor + 2) == 'i' &&
	  *(cursor + 3) == 'n' &&
	  *(cursor + 4) == 'g')
	{
	  token_stream[(int) index++].kind = Token_using;
	  return;
	}
      break;

    case 't':
      if (*(cursor + 1) == 'h' &&
	  *(cursor + 2) == 'r' &&
	  *(cursor + 3) == 'o' &&
	  *(cursor + 4) == 'w')
	{
	  token_stream[(int) index++].kind = Token_throw;
	  return;
	}
      break;

    case 'w':
      if (*(cursor + 1) == 'h' &&
	  *(cursor + 2) == 'i' &&
	  *(cursor + 3) == 'l' &&
	  *(cursor + 4) == 'e')
	{
	  token_stream[(int) index++].kind = Token_while;
	  return;
	}
      break;

    }
  token_stream[(int) index++].kind = Token_identifier;
}

void Lexer::scanKeyword6()
{
  switch (*cursor)
    {
    case 'a':
      if (*(cursor + 1) == 'n' &&
	  *(cursor + 2) == 'd' &&
	  *(cursor + 3) == '_' &&
	  *(cursor + 4) == 'e' &&
	  *(cursor + 5) == 'q')
	{
	  token_stream[(int) index++].kind = Token_and_eq;
	  return;
	}
      break;

    case 'b':
      if (*(cursor + 1) == 'i' &&
	  *(cursor + 2) == 't' &&
	  *(cursor + 3) == 'a' &&
	  *(cursor + 4) == 'n' &&
	  *(cursor + 5) == 'd')
	{
	  token_stream[(int) index++].kind = Token_bitand;
	  return;
	}
      break;

    case 'e':
      if (*(cursor + 1) == 'x' &&
	  *(cursor + 2) == 'p' &&
	  *(cursor + 3) == 'o' &&
	  *(cursor + 4) == 'r' &&
	  *(cursor + 5) == 't')
	{
	  token_stream[(int) index++].kind = Token_export;
	  return;
	}
      if (*(cursor + 1) == 'x' &&
	  *(cursor + 2) == 't' &&
	  *(cursor + 3) == 'e' &&
	  *(cursor + 4) == 'r' &&
	  *(cursor + 5) == 'n')
	{
	  token_stream[(int) index++].kind = Token_extern;
	  return;
	}
      break;

    case 'd':
      if (*(cursor + 1) == 'e' &&
	  *(cursor + 2) == 'l' &&
	  *(cursor + 3) == 'e' &&
	  *(cursor + 4) == 't' &&
	  *(cursor + 5) == 'e')
	{
	  token_stream[(int) index++].kind = Token_delete;
	  return;
	}
      if (*(cursor + 1) == 'o' &&
	  *(cursor + 2) == 'u' &&
	  *(cursor + 3) == 'b' &&
	  *(cursor + 4) == 'l' &&
	  *(cursor + 5) == 'e')
	{
	  token_stream[(int) index++].kind = Token_double;
	  return;
	}
      break;

    case 'f':
      if (*(cursor + 1) == 'r' &&
	  *(cursor + 2) == 'i' &&
	  *(cursor + 3) == 'e' &&
	  *(cursor + 4) == 'n' &&
	  *(cursor + 5) == 'd')
	{
	  token_stream[(int) index++].kind = Token_friend;
	  return;
	}
      break;

    case 'i':
      if (*(cursor + 1) == 'n' &&
	  *(cursor + 2) == 'l' &&
	  *(cursor + 3) == 'i' &&
	  *(cursor + 4) == 'n' &&
	  *(cursor + 5) == 'e')
	{
	  token_stream[(int) index++].kind = Token_inline;
	  return;
	}
      break;

    case 'K':
      if (*(cursor + 1) == '_' &&
	  *(cursor + 2) == 'D' &&
	  *(cursor + 3) == 'C' &&
	  *(cursor + 4) == 'O' &&
	  *(cursor + 5) == 'P')
	{
	  token_stream[(int) index++].kind = Token_K_DCOP;
	  return;
	}
      break;

    case 'n':
      if (*(cursor + 1) == 'o' &&
	  *(cursor + 2) == 't' &&
	  *(cursor + 3) == '_' &&
	  *(cursor + 4) == 'e' &&
	  *(cursor + 5) == 'q')
	{
	  token_stream[(int) index++].kind = Token_not_eq;
	  return;
	}
      break;

    case 'p':
      if (*(cursor + 1) == 'u' &&
	  *(cursor + 2) == 'b' &&
	  *(cursor + 3) == 'l' &&
	  *(cursor + 4) == 'i' &&
	  *(cursor + 5) == 'c')
	{
	  token_stream[(int) index++].kind = Token_public;
	  return;
	}
      break;

    case 's':
      if (*(cursor + 1) == 'i' &&
	  *(cursor + 2) == 'g' &&
	  *(cursor + 3) == 'n' &&
	  *(cursor + 4) == 'e' &&
	  *(cursor + 5) == 'd')
	{
	  token_stream[(int) index++].kind = Token_signed;
	  return;
	}
      if (*(cursor + 1) == 'i' &&
	  *(cursor + 2) == 'z' &&
	  *(cursor + 3) == 'e' &&
	  *(cursor + 4) == 'o' &&
	  *(cursor + 5) == 'f')
	{
	  token_stream[(int) index++].kind = Token_sizeof;
	  return;
	}
      if (*(cursor + 1) == 't' &&
	  *(cursor + 2) == 'a' &&
	  *(cursor + 3) == 't' &&
	  *(cursor + 4) == 'i' &&
	  *(cursor + 5) == 'c')
	{
	  token_stream[(int) index++].kind = Token_static;
	  return;
	}
      if (*(cursor + 1) == 't' &&
	  *(cursor + 2) == 'r' &&
	  *(cursor + 3) == 'u' &&
	  *(cursor + 4) == 'c' &&
	  *(cursor + 5) == 't')
	{
	  token_stream[(int) index++].kind = Token_struct;
	  return;
	}
      if (*(cursor + 1) == 'w' &&
	  *(cursor + 2) == 'i' &&
	  *(cursor + 3) == 't' &&
	  *(cursor + 4) == 'c' &&
	  *(cursor + 5) == 'h')
	{
	  token_stream[(int) index++].kind = Token_switch;
	  return;
	}
      break;

    case 'r':
      if (*(cursor + 1) == 'e' &&
	  *(cursor + 2) == 't' &&
	  *(cursor + 3) == 'u' &&
	  *(cursor + 4) == 'r' &&
	  *(cursor + 5) == 'n')
	{
	  token_stream[(int) index++].kind = Token_return;
	  return;
	}
      break;

    case 't':
      if (*(cursor + 1) == 'y' &&
	  *(cursor + 2) == 'p' &&
	  *(cursor + 3) == 'e' &&
	  *(cursor + 4) == 'i' &&
	  *(cursor + 5) == 'd')
	{
	  token_stream[(int) index++].kind = Token_typeid;
	  return;
	}
      break;

    case 'x':
      if (*(cursor + 1) == 'o' &&
	  *(cursor + 2) == 'r' &&
	  *(cursor + 3) == '_' &&
	  *(cursor + 4) == 'e' &&
	  *(cursor + 5) == 'q')
	{
	  token_stream[(int) index++].kind = Token_xor_eq;
	  return;
	}
      break;

    case 'k':
      if (*(cursor + 1) == '_' &&
	  *(cursor + 2) == 'd' &&
	  *(cursor + 3) == 'c' &&
	  *(cursor + 4) == 'o' &&
	  *(cursor + 5) == 'p')
	{
	  token_stream[(int) index++].kind = Token_k_dcop;
	  return;
	}
      break;

    }
  token_stream[(int) index++].kind = Token_identifier;
}

void Lexer::scanKeyword7()
{
  switch (*cursor)
    {
    case 'd':
      if (*(cursor + 1) == 'e' &&
	  *(cursor + 2) == 'f' &&
	  *(cursor + 3) == 'a' &&
	  *(cursor + 4) == 'u' &&
	  *(cursor + 5) == 'l' &&
	  *(cursor + 6) == 't')
	{
	  token_stream[(int) index++].kind = Token_default;
	  return;
	}
      break;

    case 'm':
      if (*(cursor + 1) == 'u' &&
	  *(cursor + 2) == 't' &&
	  *(cursor + 3) == 'a' &&
	  *(cursor + 4) == 'b' &&
	  *(cursor + 5) == 'l' &&
	  *(cursor + 6) == 'e')
	{
	  token_stream[(int) index++].kind = Token_mutable;
	  return;
	}
      break;

    case 'p':
      if (*(cursor + 1) == 'r' &&
	  *(cursor + 2) == 'i' &&
	  *(cursor + 3) == 'v' &&
	  *(cursor + 4) == 'a' &&
	  *(cursor + 5) == 't' &&
	  *(cursor + 6) == 'e')
	{
	  token_stream[(int) index++].kind = Token_private;
	  return;
	}
      break;
    case 's':
      if (*(cursor + 1) == 'i' &&
	  *(cursor + 2) == 'g' &&
	  *(cursor + 3) == 'n' &&
	  *(cursor + 4) == 'a' &&
	  *(cursor + 5) == 'l' &&
	  *(cursor + 6) == 's')
	{
	  token_stream[(int) index++].kind = Token_signals;
	  return;
	}
      break;
    case 't':
      if (*(cursor + 1) == 'y' &&
	  *(cursor + 2) == 'p' &&
	  *(cursor + 3) == 'e' &&
	  *(cursor + 4) == 'd' &&
	  *(cursor + 5) == 'e' &&
	  *(cursor + 6) == 'f')
	{
	  token_stream[(int) index++].kind = Token_typedef;
	  return;
	}
      break;

    case 'v':
      if (*(cursor + 1) == 'i' &&
	  *(cursor + 2) == 'r' &&
	  *(cursor + 3) == 't' &&
	  *(cursor + 4) == 'u' &&
	  *(cursor + 5) == 'a' &&
	  *(cursor + 6) == 'l')
	{
	  token_stream[(int) index++].kind = Token_virtual;
	  return;
	}
      break;

    case 'Q':
      if (*(cursor + 1) == '_' &&
      *(cursor + 2) == 'E' &&
      *(cursor + 3) == 'N' &&
      *(cursor + 4) == 'U' &&
      *(cursor + 5) == 'M' &&
      *(cursor + 6) == 'S')
      {
        token_stream[(int) index++].kind = Token_Q_ENUMS;
        return;
      }
        break;

    }
  token_stream[(int) index++].kind = Token_identifier;
}

void Lexer::scanKeyword8()
{
  switch (*cursor)
    {
    case '_':
      if (*(cursor + 1) == '_' &&
	  *(cursor + 2) == 't' &&
	  *(cursor + 3) == 'y' &&
	  *(cursor + 4) == 'p' &&
	  *(cursor + 5) == 'e' &&
	  *(cursor + 6) == 'o' &&
	  *(cursor + 7) == 'f')
	{
	  token_stream[(int) index++].kind = Token___typeof;
	  return;
	}
      break;

    case 'c':
      if (*(cursor + 1) == 'o' &&
	  *(cursor + 2) == 'n' &&
	  *(cursor + 3) == 't' &&
	  *(cursor + 4) == 'i' &&
	  *(cursor + 5) == 'n' &&
	  *(cursor + 6) == 'u' &&
	  *(cursor + 7) == 'e')
	{
	  token_stream[(int) index++].kind = Token_continue;
	  return;
	}
      break;

    case 'e':
      if (*(cursor + 1) == 'x' &&
	  *(cursor + 2) == 'p' &&
	  *(cursor + 3) == 'l' &&
	  *(cursor + 4) == 'i' &&
	  *(cursor + 5) == 'c' &&
	  *(cursor + 6) == 'i' &&
	  *(cursor + 7) == 't')
	{
	  token_stream[(int) index++].kind = Token_explicit;
	  return;
	}
      break;

    case 'o':
      if (*(cursor + 1) == 'p' &&
	  *(cursor + 2) == 'e' &&
	  *(cursor + 3) == 'r' &&
	  *(cursor + 4) == 'a' &&
	  *(cursor + 5) == 't' &&
	  *(cursor + 6) == 'o' &&
	  *(cursor + 7) == 'r')
	{
	  token_stream[(int) index++].kind = Token_operator;
	  return;
	}
      break;

    case 'Q':
      if (*(cursor + 1) == '_' &&
	  *(cursor + 2) == 'O' &&
	  *(cursor + 3) == 'B' &&
	  *(cursor + 4) == 'J' &&
	  *(cursor + 5) == 'E' &&
	  *(cursor + 6) == 'C' &&
	  *(cursor + 7) == 'T')
	{
	  token_stream[(int) index++].kind = Token_Q_OBJECT;
	  return;
	}
      break;

    case 'r':
      if (*(cursor + 1) == 'e' &&
	  *(cursor + 2) == 'g' &&
	  *(cursor + 3) == 'i' &&
	  *(cursor + 4) == 's' &&
	  *(cursor + 5) == 't' &&
	  *(cursor + 6) == 'e' &&
	  *(cursor + 7) == 'r')
	{
	  token_stream[(int) index++].kind = Token_register;
	  return;
	}
      break;

    case 'u':
      if (*(cursor + 1) == 'n' &&
	  *(cursor + 2) == 's' &&
	  *(cursor + 3) == 'i' &&
	  *(cursor + 4) == 'g' &&
	  *(cursor + 5) == 'n' &&
	  *(cursor + 6) == 'e' &&
	  *(cursor + 7) == 'd')
	{
	  token_stream[(int) index++].kind = Token_unsigned;
	  return;
	}
      break;

    case 't':
      if (*(cursor + 1) == 'e' &&
	  *(cursor + 2) == 'm' &&
	  *(cursor + 3) == 'p' &&
	  *(cursor + 4) == 'l' &&
	  *(cursor + 5) == 'a' &&
	  *(cursor + 6) == 't' &&
	  *(cursor + 7) == 'e')
	{
	  token_stream[(int) index++].kind = Token_template;
	  return;
	}
      if (*(cursor + 1) == 'y' &&
	  *(cursor + 2) == 'p' &&
	  *(cursor + 3) == 'e' &&
	  *(cursor + 4) == 'n' &&
	  *(cursor + 5) == 'a' &&
	  *(cursor + 6) == 'm' &&
	  *(cursor + 7) == 'e')
	{
	  token_stream[(int) index++].kind = Token_typename;
	  return;
	}
      break;

    case 'v':
      if (*(cursor + 1) == 'o' &&
	  *(cursor + 2) == 'l' &&
	  *(cursor + 3) == 'a' &&
	  *(cursor + 4) == 't' &&
	  *(cursor + 5) == 'i' &&
	  *(cursor + 6) == 'l' &&
	  *(cursor + 7) == 'e')
	{
	  token_stream[(int) index++].kind = Token_volatile;
	  return;
	}
      break;

    }
  token_stream[(int) index++].kind = Token_identifier;
}

void Lexer::scanKeyword9()
{
  switch (*cursor)
    {
    case 'p':
      if (*(cursor + 1) == 'r' &&
	  *(cursor + 2) == 'o' &&
	  *(cursor + 3) == 't' &&
	  *(cursor + 4) == 'e' &&
	  *(cursor + 5) == 'c' &&
	  *(cursor + 6) == 't' &&
	  *(cursor + 7) == 'e' &&
	  *(cursor + 8) == 'd')
	{
	  token_stream[(int) index++].kind = Token_protected;
	  return;
	}
      break;

    case 'n':
      if (*(cursor + 1) == 'a' &&
	  *(cursor + 2) == 'm' &&
	  *(cursor + 3) == 'e' &&
	  *(cursor + 4) == 's' &&
	  *(cursor + 5) == 'p' &&
	  *(cursor + 6) == 'a' &&
	  *(cursor + 7) == 'c' &&
	  *(cursor + 8) == 'e')
	{
	  token_stream[(int) index++].kind = Token_namespace;
	  return;
	}
      break;

    }
  token_stream[(int) index++].kind = Token_identifier;
}

void Lexer::scanKeyword10()
{
  switch (*cursor)
    {
    case 'c':
      if (*(cursor + 1) == 'o' &&
	  *(cursor + 2) == 'n' &&
	  *(cursor + 3) == 's' &&
	  *(cursor + 4) == 't' &&
	  *(cursor + 5) == '_' &&
	  *(cursor + 6) == 'c' &&
	  *(cursor + 7) == 'a' &&
	  *(cursor + 8) == 's' &&
	  *(cursor + 9) == 't')
	{
	  token_stream[(int) index++].kind = Token_const_cast;
	  return;
	}
      break;

    case 'Q':
        if (*(cursor + 1) == '_' &&
            *(cursor + 2) == 'P' &&
            *(cursor + 3) == 'R' &&
            *(cursor + 4) == 'O' &&
            *(cursor + 5) == 'P' &&
            *(cursor + 6) == 'E' &&
            *(cursor + 7) == 'R' &&
            *(cursor + 8) == 'T' &&
            *(cursor + 9) == 'Y')
          {
            token_stream[(int) index++].kind = Token_Q_PROPERTY;
            return;
          }

        break;
    }

  token_stream[(int) index++].kind = Token_identifier;
}

void Lexer::scanKeyword11()
{
  switch (*cursor)
    {
    case 'Q':
      if (*(cursor + 1) == '_' &&
	  *(cursor + 2) == 'I' &&
	  *(cursor + 3) == 'N' &&
	  *(cursor + 4) == 'V' &&
	  *(cursor + 5) == 'O' &&
	  *(cursor + 6) == 'K' &&
	  *(cursor + 7) == 'A' &&
	  *(cursor + 8) == 'B' &&
	  *(cursor + 9) == 'L' &&
	  *(cursor + 10) == 'E')
	{
	  token_stream[(int) index++].kind = Token_Q_INVOKABLE;
	  return;
	}
      break;

    case 's':
      if (*(cursor + 1) == 't' &&
	  *(cursor + 2) == 'a' &&
	  *(cursor + 3) == 't' &&
	  *(cursor + 4) == 'i' &&
	  *(cursor + 5) == 'c' &&
	  *(cursor + 6) == '_' &&
	  *(cursor + 7) == 'c' &&
	  *(cursor + 8) == 'a' &&
	  *(cursor + 9) == 's' &&
	  *(cursor + 10) == 't')
	{
	  token_stream[(int) index++].kind = Token_static_cast;
	  return;
	}
      break;

    }
  token_stream[(int) index++].kind = Token_identifier;
}

void Lexer::scanKeyword12()
{
  switch (*cursor)
    {
    case 'd':
      if (*(cursor + 1) == 'y' &&
	  *(cursor + 2) == 'n' &&
	  *(cursor + 3) == 'a' &&
	  *(cursor + 4) == 'm' &&
	  *(cursor + 5) == 'i' &&
	  *(cursor + 6) == 'c' &&
	  *(cursor + 7) == '_' &&
	  *(cursor + 8) == 'c' &&
	  *(cursor + 9) == 'a' &&
	  *(cursor + 10) == 's' &&
	  *(cursor + 11) == 't')
	{
	  token_stream[(int) index++].kind = Token_dynamic_cast;
	  return;
	}
      break;

    }
  token_stream[(int) index++].kind = Token_identifier;
}

void Lexer::scanKeyword13()
{
  switch (*cursor)
    {
    case '_':
      if (*(cursor + 1) == '_' &&
	  *(cursor + 2) == 'a' &&
	  *(cursor + 3) == 't' &&
	  *(cursor + 4) == 't' &&
	  *(cursor + 5) == 'r' &&
	  *(cursor + 6) == 'i' &&
	  *(cursor + 7) == 'b' &&
	  *(cursor + 8) == 'u' &&
	  *(cursor + 9) == 't' &&
	  *(cursor + 10) == 'e' &&
	  *(cursor + 11) == '_' &&
	  *(cursor + 12) == '_')
	{
	  token_stream[(int) index++].kind = Token___attribute__;
	  return;
	}
      break;
    }
  token_stream[(int) index++].kind = Token_identifier;
}

void Lexer::scanKeyword14()
{
  switch (*cursor)
    {
    case 'k':
      if (*(cursor + 1) == '_' &&
	  *(cursor + 2) == 'd' &&
	  *(cursor + 3) == 'c' &&
	  *(cursor + 4) == 'o' &&
	  *(cursor + 5) == 'p' &&
	  *(cursor + 6) == '_' &&
	  *(cursor + 7) == 's' &&
	  *(cursor + 8) == 'i' &&
	  *(cursor + 9) == 'g' &&
	  *(cursor + 10) == 'n' &&
	  *(cursor + 11) == 'a' &&
	  *(cursor + 12) == 'l' &&
	  *(cursor + 13) == 's')
	{
	  token_stream[(int) index++].kind = Token_k_dcop_signals;
	  return;
	}
      break;
    }
  token_stream[(int) index++].kind = Token_identifier;
}

void Lexer::scanKeyword16()
{
  switch (*cursor)
    {
    case 'r':
      if (*(cursor + 1) == 'e' &&
	  *(cursor + 2) == 'i' &&
	  *(cursor + 3) == 'n' &&
	  *(cursor + 4) == 't' &&
	  *(cursor + 5) == 'e' &&
	  *(cursor + 6) == 'r' &&
	  *(cursor + 7) == 'p' &&
	  *(cursor + 8) == 'r' &&
	  *(cursor + 9) == 'e' &&
	  *(cursor + 10) == 't' &&
	  *(cursor + 11) == '_' &&
	  *(cursor + 12) == 'c' &&
	  *(cursor + 13) == 'a' &&
	  *(cursor + 14) == 's' &&
	  *(cursor + 15) == 't')
	{
	  token_stream[(int) index++].kind = Token_reinterpret_cast;
	  return;
	}
      break;
    }

  token_stream[(int) index++].kind = Token_identifier;
}

// kate: space-indent on; indent-width 2; replace-tabs on;
