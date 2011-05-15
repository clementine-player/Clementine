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

#ifndef PP_ENVIRONMENT_H
#define PP_ENVIRONMENT_H

#include <vector>
#include <cstring>

namespace rpp {

class pp_environment
{
public:
  typedef std::vector<pp_macro*>::const_iterator const_iterator;

public:
  pp_environment ():
    current_line (0),
    _M_hash_size (4093)
  {
    _M_base = (pp_macro **) memset (new pp_macro* [_M_hash_size], 0, _M_hash_size * sizeof (pp_macro*));
  }

  ~pp_environment ()
  {
    for (std::size_t i = 0; i < _M_macros.size (); ++i)
      delete _M_macros [i];

    delete [] _M_base;
  }

  const_iterator first_macro () const { return _M_macros.begin (); }
  const_iterator last_macro () const { return _M_macros.end (); }

  inline void bind (pp_fast_string const *__name, pp_macro const &__macro)
  {
    std::size_t h = hash_code (*__name) % _M_hash_size;
    pp_macro *m = new pp_macro (__macro);
    m->name = __name;
    m->next = _M_base [h];
    m->hash_code = h;
    _M_base [h] = m;

    _M_macros.push_back (m);

    if (_M_macros.size() == _M_hash_size)
      rehash();
  }

  inline void unbind (pp_fast_string const *__name)
  {
    if (pp_macro *m = resolve (__name))
      m->hidden = true;
  }

  inline void unbind (char const *__s, std::size_t __size)
  {
    pp_fast_string __tmp (__s, __size);
    unbind (&__tmp);
  }

  inline pp_macro *resolve (pp_fast_string const *__name) const
  {
    std::size_t h = hash_code (*__name) % _M_hash_size;
    pp_macro *it = _M_base [h];

    while (it && it->name && it->hash_code == h && (*it->name != *__name || it->hidden))
      it = it->next;

    return it;
  }

  inline pp_macro *resolve (char const *__data, std::size_t __size) const
  {
    pp_fast_string const __tmp (__data, __size);
    return resolve (&__tmp);
  }

  std::string current_file;
  int current_line;

private:
  inline std::size_t hash_code (pp_fast_string const &s) const
  {
    std::size_t hash_value = 0;

    for (std::size_t i = 0; i < s.size (); ++i)
      hash_value = (hash_value << 5) - hash_value + s.at (i);

    return hash_value;
  }

  void rehash()
  {
    delete[] _M_base;

    _M_hash_size <<= 1;

    _M_base = (pp_macro **) memset (new pp_macro* [_M_hash_size], 0, _M_hash_size * sizeof(pp_macro*));
    for (std::size_t index = 0; index < _M_macros.size (); ++index)
      {
        pp_macro *elt = _M_macros [index];
        std::size_t h = hash_code (*elt->name) % _M_hash_size;
        elt->next = _M_base [h];
        elt->hash_code = h;
        _M_base [h] = elt;
      }
  }

private:
  std::vector<pp_macro*> _M_macros;
  pp_macro **_M_base;
  std::size_t _M_hash_size;
};

} // namespace rpp

#endif // PP_ENVIRONMENT_H

// kate: space-indent on; indent-width 2; replace-tabs on;
