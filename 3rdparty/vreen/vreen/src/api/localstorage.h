/****************************************************************************
**
** Vreen - vk.com API Qt bindings
**
** Copyright © 2012 Aleksey Sidorov <gorthauer87@ya.ru>
**
*****************************************************************************
**
** $VREEN_BEGIN_LICENSE$
** This program is free software: you can redistribute it and/or modify
** it under the terms of the GNU Lesser General Public License as published by
** the Free Software Foundation, either version 3 of the License, or
** (at your option) any later version.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
** See the GNU Lesser General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with this program.  If not, see http://www.gnu.org/licees/.
** $VREEN_END_LICENSE$
**
****************************************************************************/
#ifndef VREEN_LOCALSTORAGE_H
#define VREEN_LOCALSTORAGE_H
#include "contact.h"

namespace Vreen {

class AbstractLocalStorage
{
public:
    AbstractLocalStorage();
    virtual ~AbstractLocalStorage() {}
protected:
    virtual void loadBuddies(Roster *roster) = 0;
    virtual void storeBuddies(Roster *roster) = 0;
    //TODO group managers
    //key value storage
    virtual void store(const QString &key, const QVariant &value) = 0;
    virtual QVariant load(const QString &key) = 0;
    //TODO messages history async get and set
};

} // namespace Vreen

#endif // VREEN_LOCALSTORAGE_H
