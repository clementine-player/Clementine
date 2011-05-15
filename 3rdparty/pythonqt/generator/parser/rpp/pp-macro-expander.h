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

#ifndef PP_MACRO_EXPANDER_H
#define PP_MACRO_EXPANDER_H

#include <stdio.h>

namespace rpp {

struct pp_frame
{
  pp_macro *expanding_macro;
  std::vector<std::string> *actuals;

  pp_frame (pp_macro *__expanding_macro, std::vector<std::string> *__actuals):
    expanding_macro (__expanding_macro), actuals (__actuals) {}
};

class pp_macro_expander
{
  pp_environment &env;
  pp_frame *frame;

  pp_skip_number skip_number;
  pp_skip_identifier skip_identifier;
  pp_skip_string_literal skip_string_literal;
  pp_skip_char_literal skip_char_literal;
  pp_skip_argument skip_argument;
  pp_skip_comment_or_divop skip_comment_or_divop;
  pp_skip_blanks skip_blanks;
  pp_skip_whitespaces skip_whitespaces;

  std::string const *resolve_formal (pp_fast_string const *__name)
  {
    assert (__name != 0);

    if (! frame)
      return 0;

    assert (frame->expanding_macro != 0);

    std::vector<pp_fast_string const *> const formals = frame->expanding_macro->formals;
    for (std::size_t index = 0; index < formals.size(); ++index)
      {
        pp_fast_string const *formal = formals[index];

        if (*formal != *__name)
          continue;

        else if (frame->actuals && index < frame->actuals->size())
          return &(*frame->actuals)[index];

        else
          assert (0); // internal error?
      }

    return 0;
  }

public: // attributes
  int lines;
  int generated_lines;

public:
  pp_macro_expander (pp_environment &__env, pp_frame *__frame = 0):
    env (__env), frame (__frame), lines (0), generated_lines (0) {}

  template <typename _InputIterator, typename _OutputIterator>
  _InputIterator operator () (_InputIterator __first, _InputIterator __last, _OutputIterator __result)
  {
    generated_lines = 0;
    __first = skip_blanks (__first, __last);
    lines = skip_blanks.lines;

    while (__first != __last)
      {
        if (*__first == '\n')
          {
            *__result++ = *__first;
            ++lines;

            __first = skip_blanks (++__first, __last);
            lines += skip_blanks.lines;

            if (__first != __last && *__first == '#')
              break;
          }
        else if (*__first == '#')
          {
            __first = skip_blanks (++__first, __last);
            lines += skip_blanks.lines;

            _InputIterator end_id = skip_identifier (__first, __last);

            // ### rewrite: not safe
            char name_buffer[512], *cp = name_buffer;
            std::copy (__first, end_id, cp);
            std::size_t name_size = end_id - __first;
            name_buffer[name_size] = '\0';

            pp_fast_string fast_name (name_buffer, name_size);

            if (std::string const *actual = resolve_formal (&fast_name))
              {
                *__result++ = '\"';

                for (std::string::const_iterator it = skip_whitespaces (actual->begin (), actual->end ());
                    it != actual->end (); ++it)
                  {
                    if (*it == '"')
                      {
                        *__result++ = '\\';
                        *__result++ = *it;
                      }

                    else if (*it == '\n')
                      {
                        *__result++ = '"';
                        *__result++ = '\n';
                        *__result++ = '"';
                      }

                    else
                      *__result++ = *it;
                  }

                *__result++ = '\"';
                __first = end_id;
              }
            else
              *__result++ = '#'; // ### warning message?
          }
        else if (*__first == '\"')
          {
            _InputIterator next_pos = skip_string_literal (__first, __last);
            lines += skip_string_literal.lines;
            std::copy (__first, next_pos, __result);
            __first = next_pos;
          }
        else if (*__first == '\'')
          {
            _InputIterator next_pos = skip_char_literal (__first, __last);
            lines += skip_char_literal.lines;
            std::copy (__first, next_pos, __result);
            __first = next_pos;
          }
        else if (_PP_internal::comment_p (__first, __last))
          {
            __first = skip_comment_or_divop (__first, __last);
            int n = skip_comment_or_divop.lines;
            lines += n;

            while (n-- > 0)
              *__result++ = '\n';
          }
        else if (pp_isspace (*__first))
          {
            for (; __first != __last; ++__first)
              {
                if (*__first == '\n' || !pp_isspace (*__first))
                  break;
              }

            *__result = ' ';
          }
        else if (pp_isdigit (*__first))
          {
            _InputIterator next_pos = skip_number (__first, __last);
            lines += skip_number.lines;
            std::copy (__first, next_pos, __result);
            __first = next_pos;
          }
        else if (pp_isalpha (*__first) || *__first == '_')
          {
            _InputIterator name_begin = __first;
            _InputIterator name_end = skip_identifier (__first, __last);
            __first = name_end; // advance

            // search for the paste token
            _InputIterator next = skip_blanks (__first, __last);
            if (next != __last && *next == '#')
              {
                ++next;
                if (next != __last && *next == '#')
                  __first = skip_blanks(++next, __last);
              }

            // ### rewrite: not safe

            std::ptrdiff_t name_size;
#if defined(__SUNPRO_CC)
             std::distance (name_begin, name_end, name_size);
#else
            name_size = std::distance (name_begin, name_end);
#endif
            assert (name_size >= 0 && name_size < 512);

            char name_buffer[512], *cp = name_buffer;
            std::size_t __size = name_end - name_begin;
            std::copy (name_begin, name_end, cp);
            name_buffer[__size] = '\0';

            pp_fast_string fast_name (name_buffer, name_size);

            if (std::string const *actual = resolve_formal (&fast_name))
              {
                std::copy (actual->begin (), actual->end (), __result);
                continue;
              }

            static bool hide_next = false; // ### remove me

            pp_macro *macro = env.resolve (name_buffer, name_size);
            if (! macro || macro->hidden || hide_next)
              {
                hide_next = ! strcmp (name_buffer, "defined");

                if (__size == 8 && name_buffer [0] == '_' && name_buffer [1] == '_')
                  {
                    if (! strcmp (name_buffer, "__LINE__"))
                      {
                        char buf [16];
                        char *end = buf + pp_snprintf (buf, 16, "%d", env.current_line + lines);

                        std::copy (&buf [0], end, __result);
                        continue;
                      }

                    else if (! strcmp (name_buffer, "__FILE__"))
                      {
                        __result++ = '"';
                        std::copy (env.current_file.begin (), env.current_file.end (), __result); // ### quote
                        __result++ = '"';
                        continue;
                      }
                  }

                std::copy (name_begin, name_end, __result);
                continue;
              }

            if (! macro->function_like)
              {
                pp_macro *m = 0;

                if (macro->definition)
                  {
                    macro->hidden = true;

                    std::string __tmp;
                    __tmp.reserve (256);

                    pp_macro_expander expand_macro (env);
                    expand_macro (macro->definition->begin (), macro->definition->end (), std::back_inserter (__tmp));
                    generated_lines += expand_macro.lines;

                    if (! __tmp.empty ())
                      {
                        std::string::iterator __begin_id = skip_whitespaces (__tmp.begin (), __tmp.end ());
                        std::string::iterator __end_id = skip_identifier (__begin_id, __tmp.end ());

                        if (__end_id == __tmp.end ())
                          {
                            std::string __id;
                            __id.assign (__begin_id, __end_id);

                            std::size_t x;
#if defined(__SUNPRO_CC)
                            std::distance (__begin_id, __end_id, x);
#else
                            x = std::distance (__begin_id, __end_id);
#endif
                            m = env.resolve (__id.c_str (), x);
                          }

                        if (! m)
                          std::copy (__tmp.begin (), __tmp.end (), __result);
                      }

                    macro->hidden = false;
                  }

                if (! m)
                  continue;

                macro = m;
              }

            // function like macro
            _InputIterator arg_it = skip_whitespaces (__first, __last);

            if (arg_it == __last || *arg_it != '(')
              {
                std::copy (name_begin, name_end, __result);
                lines += skip_whitespaces.lines;
                __first = arg_it;
                continue;
              }

            std::vector<std::string> actuals;
            actuals.reserve (5);
            ++arg_it; // skip '('

            pp_macro_expander expand_actual (env, frame);

            _InputIterator arg_end = skip_argument_variadics (actuals, macro, arg_it, __last);
            if (arg_it != arg_end)
              {
                std::string actual (arg_it, arg_end);
                actuals.resize (actuals.size() + 1);
                actuals.back ().reserve (255);
                expand_actual (actual.begin (), actual.end(), std::back_inserter (actuals.back()));
                arg_it = arg_end;
              }

            while (arg_it != __last && *arg_end == ',')
              {
                ++arg_it; // skip ','

                arg_end = skip_argument_variadics (actuals, macro, arg_it, __last);
                std::string actual (arg_it, arg_end);
                actuals.resize (actuals.size() + 1);
                actuals.back ().reserve (255);
                expand_actual (actual.begin (), actual.end(), std::back_inserter (actuals.back()));
                arg_it = arg_end;
              }

              assert (arg_it != __last && *arg_it == ')');

              ++arg_it; // skip ')'
              __first = arg_it;

#if 0 // ### enable me
              assert ((macro->variadics && macro->formals.size () >= actuals.size ())
                          || macro->formals.size() == actuals.size());
#endif

              pp_frame frame (macro, &actuals);
              pp_macro_expander expand_macro (env, &frame);
              macro->hidden = true;
              expand_macro (macro->definition->begin (), macro->definition->end (), __result);
              macro->hidden = false;
              generated_lines += expand_macro.lines;
          }
        else
          *__result++ = *__first++;
      }

    return __first;
  }

  template <typename _InputIterator>
  _InputIterator skip_argument_variadics (std::vector<std::string> const &__actuals, pp_macro *__macro,
                                          _InputIterator __first, _InputIterator __last)
  {
    _InputIterator arg_end = skip_argument (__first, __last);

    while (__macro->variadics && __first != arg_end && arg_end != __last && *arg_end == ','
        && (__actuals.size () + 1) == __macro->formals.size ())
      {
        arg_end = skip_argument (++arg_end, __last);
      }

    return arg_end;
  }
};

} // namespace rpp

#endif // PP_MACRO_EXPANDER_H

// kate: space-indent on; indent-width 2; replace-tabs on;
