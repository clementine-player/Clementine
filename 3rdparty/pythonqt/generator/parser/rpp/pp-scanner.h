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

#ifndef PP_SCANNER_H
#define PP_SCANNER_H

namespace rpp {

struct pp_skip_blanks
{
  int lines;

  template <typename _InputIterator>
  _InputIterator operator () (_InputIterator __first, _InputIterator __last)
  {
    lines = 0;

    for (; __first != __last; lines += (*__first != '\n' ? 0 : 1), ++__first)
      {
        if (*__first == '\\')
          {
            _InputIterator __begin = __first;
            ++__begin;

            if (__begin != __last && *__begin == '\n')
                ++__first;
            else
              break;
          }
        else if (*__first == '\n' || !pp_isspace (*__first))
          break;
      }

    return __first;
  }
};

struct pp_skip_whitespaces
{
  int lines;

  template <typename _InputIterator>
  _InputIterator operator () (_InputIterator __first, _InputIterator __last)
  {
    lines = 0;

    for (; __first != __last; lines += (*__first != '\n' ? 0 : 1), ++__first)
      {
        if (! pp_isspace (*__first))
          break;
      }

    return __first;
  }
};

struct pp_skip_comment_or_divop
{
  int lines;

  template <typename _InputIterator>
  _InputIterator operator () (_InputIterator __first, _InputIterator __last)
  {
    enum {
      MAYBE_BEGIN,
      BEGIN,
      MAYBE_END,
      END,
      IN_COMMENT,
      IN_CXX_COMMENT
    } state (MAYBE_BEGIN);

    lines = 0;

    for (; __first != __last; lines += (*__first != '\n' ? 0 : 1), ++__first)
      {
        switch (state)
          {
            default:
              assert (0);
              break;

            case MAYBE_BEGIN:
              if (*__first != '/')
                return __first;

              state = BEGIN;
              break;

            case BEGIN:
              if (*__first == '*')
                state = IN_COMMENT;
              else if (*__first == '/')
                state = IN_CXX_COMMENT;
              else
                return __first;
              break;

            case IN_COMMENT:
              if (*__first == '*')
                state = MAYBE_END;
              break;

            case IN_CXX_COMMENT:
              if (*__first == '\n')
                return __first;
              break;

            case MAYBE_END:
              if (*__first == '/')
                state = END;
              else if (*__first != '*')
                state = IN_COMMENT;
              break;

            case END:
              return __first;
          }
      }

    return __first;
  }
};

struct pp_skip_identifier
{
  int lines;

  template <typename _InputIterator>
  _InputIterator operator () (_InputIterator __first, _InputIterator __last)
  {
    lines = 0;

    for (; __first != __last; lines += (*__first != '\n' ? 0 : 1), ++__first)
      {
        if (! pp_isalnum (*__first) && *__first != '_')
          break;
      }

    return __first;
  }
};

struct pp_skip_number
{
  int lines;

  template <typename _InputIterator>
  _InputIterator operator () (_InputIterator __first, _InputIterator __last)
  {
    lines = 0;

    for (; __first != __last; lines += (*__first != '\n' ? 0 : 1), ++__first)
      {
        if (! pp_isalnum (*__first) && *__first != '.')
          break;
      }

    return __first;
  }
};

struct pp_skip_string_literal
{
  int lines;

  template <typename _InputIterator>
  _InputIterator operator () (_InputIterator __first, _InputIterator __last)
  {
    enum {
      BEGIN,
      IN_STRING,
      QUOTE,
      END
    } state (BEGIN);

    lines = 0;

    for (; __first != __last; lines += (*__first != '\n' ? 0 : 1), ++__first)
      {
        switch (state)
          {
            default:
              assert (0);
              break;

            case BEGIN:
              if (*__first != '\"')
                return __first;
              state = IN_STRING;
              break;

            case IN_STRING:
              assert (*__first != '\n');

              if (*__first == '\"')
                state = END;
              else if (*__first == '\\')
                state = QUOTE;
              break;

            case QUOTE:
              state = IN_STRING;
              break;

            case END:
              return __first;
          }
      }

    return __first;
  }
};

struct pp_skip_char_literal
{
  int lines;

  template <typename _InputIterator>
  _InputIterator operator () (_InputIterator __first, _InputIterator __last)
  {
    enum {
      BEGIN,
      IN_STRING,
      QUOTE,
      END
    } state (BEGIN);

    lines = 0;

    for (; state != END && __first != __last; lines += (*__first != '\n' ? 0 : 1), ++__first)
      {
        switch (state)
          {
            default:
              assert (0);
              break;

            case BEGIN:
              if (*__first != '\'')
                return __first;
              state = IN_STRING;
              break;

            case IN_STRING:
              assert (*__first != '\n');

              if (*__first == '\'')
                state = END;
              else if (*__first == '\\')
                state = QUOTE;
              break;

            case QUOTE:
              state = IN_STRING;
              break;
          }
      }

    return __first;
  }
};

struct pp_skip_argument
{
  pp_skip_identifier skip_number;
  pp_skip_identifier skip_identifier;
  pp_skip_string_literal skip_string_literal;
  pp_skip_char_literal skip_char_literal;
  pp_skip_comment_or_divop skip_comment_or_divop;
  int lines;

  template <typename _InputIterator>
  _InputIterator operator () (_InputIterator __first, _InputIterator __last)
  {
    int depth = 0;
    lines = 0;

    while (__first != __last)
      {
        if (!depth && (*__first == ')' || *__first == ','))
          break;
        else if (*__first == '(')
          ++depth, ++__first;
        else if (*__first == ')')
          --depth, ++__first;
        else if (*__first == '\"')
          {
            __first = skip_string_literal (__first, __last);
            lines += skip_string_literal.lines;
          }
        else if (*__first == '\'')
          {
            __first = skip_char_literal (__first, __last);
            lines += skip_char_literal.lines;
          }
        else if (*__first == '/')
          {
            __first = skip_comment_or_divop (__first, __last);
            lines += skip_comment_or_divop.lines;
          }
        else if (pp_isalpha (*__first) || *__first == '_')
          {
            __first = skip_identifier (__first, __last);
            lines += skip_identifier.lines;
          }
        else if (pp_isdigit (*__first))
          {
            __first = skip_number (__first, __last);
            lines += skip_number.lines;
          }
        else if (*__first == '\n')
          {
            ++__first;
            ++lines;
          }
        else
          ++__first;
      }

    return __first;
  }
};

} // namespace rpp

#endif // PP_SCANNER_H

// kate: space-indent on; indent-width 2; replace-tabs on;
