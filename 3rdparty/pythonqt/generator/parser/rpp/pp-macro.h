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

#ifndef PP_MACRO_H
#define PP_MACRO_H

namespace rpp {

struct pp_macro
{
#if defined (PP_WITH_MACRO_POSITION)
  pp_fast_string const *file;
#endif
  pp_fast_string const *name;
  pp_fast_string const *definition;
  std::vector<pp_fast_string const *> formals;

  union
  {
    int unsigned state;

    struct
    {
      int unsigned hidden: 1;
      int unsigned function_like: 1;
      int unsigned variadics: 1;
    };
  };

  int lines;
  pp_macro *next;
  std::size_t hash_code;

  inline pp_macro():
#if defined (PP_WITH_MACRO_POSITION)
    file (0),
#endif
    name (0),
    definition (0),
    state (0),
    lines (0),
    next (0),
    hash_code (0)
    {}
};

} // namespace rpp

#endif // PP_MACRO_H

// kate: space-indent on; indent-width 2; replace-tabs on;
