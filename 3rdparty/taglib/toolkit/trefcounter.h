/***************************************************************************
    copyright            : (C) 2013 by Tsuda Kageyu
    email                : tsuda.kageyu@gmail.com
 ***************************************************************************/

/***************************************************************************
 *   This library is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU Lesser General Public License version   *
 *   2.1 as published by the Free Software Foundation.                     *
 *                                                                         *
 *   This library is distributed in the hope that it will be useful, but   *
 *   WITHOUT ANY WARRANTY; without even the implied warranty of            *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU     *
 *   Lesser General Public License for more details.                       *
 *                                                                         *
 *   You should have received a copy of the GNU Lesser General Public      *
 *   License along with this library; if not, write to the Free Software   *
 *   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA         *
 *   02110-1301  USA                                                       *
 *                                                                         *
 *   Alternatively, this file is available under the Mozilla Public        *
 *   License Version 1.1.  You may obtain a copy of the License at         *
 *   http://www.mozilla.org/MPL/                                           *
 ***************************************************************************/

#ifndef TAGLIB_REFCOUNTER_H
#define TAGLIB_REFCOUNTER_H

#include "taglib_export.h"
#include "taglib.h"

#ifndef DO_NOT_DOCUMENT // Tell Doxygen to skip this class.
/*!
  * \internal
  * This is just used as a base class for shared classes in TagLib.
  *
  * \warning This <b>is not</b> part of the TagLib public API!
  */
namespace TagLib
{
  class TAGLIB_EXPORT RefCounter
  {
  public:
    RefCounter();
    virtual ~RefCounter();

    void ref();
    bool deref();
    int count() const;

  private:
    class RefCounterPrivate;
    RefCounterPrivate *d;
  };
}

#endif // DO_NOT_DOCUMENT
#endif
