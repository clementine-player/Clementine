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

#ifndef PP_STRING_H
#define PP_STRING_H

namespace rpp {

template <typename _CharT>
class pp_string
{
  typedef std::char_traits<_CharT> traits_type;
  typedef std::size_t size_type;

  _CharT const *_M_begin;
  std::size_t _M_size;

public:
  inline pp_string ():
    _M_begin (0), _M_size(0) {}

  explicit pp_string (std::string const &__s):
    _M_begin (__s.c_str ()), _M_size (__s.size ()) {}

  inline pp_string (_CharT const *__begin, std::size_t __size):
    _M_begin (__begin), _M_size (__size) {}

  inline _CharT const *begin () const { return _M_begin; }
  inline _CharT const *end () const { return _M_begin + _M_size; }

  inline _CharT at (std::size_t index) const { return _M_begin [index]; }

  inline std::size_t size () const { return _M_size; }

  inline int compare (pp_string const &__other) const
  {
    size_type const __size = this->size();
    size_type const __osize = __other.size();
    size_type const __len = std::min (__size,  __osize);

    int __r = traits_type::compare (_M_begin, __other._M_begin, __len);
    if (!__r)
        __r =  (int) (__size - __osize);

    return __r;
  }

  inline bool operator == (pp_string const &__other) const
  { return compare (__other) == 0; }

  inline bool operator != (pp_string const &__other) const
  { return compare (__other) != 0; }

  inline bool operator < (pp_string const &__other) const
  { return compare (__other) < 0; }

  inline bool operator == (char const *s) const
  {
    std::size_t n = strlen (s);

    if (n != _M_size)
      return false;

    return ! strncmp (_M_begin, s, n);
  }

  inline bool operator != (char const *s) const
  { return ! operator == (s); }
};

} // namespace rpp

#endif // PP_STRING_H

// kate: space-indent on; indent-width 2; replace-tabs on;
