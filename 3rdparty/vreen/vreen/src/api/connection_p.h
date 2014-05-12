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
** along with this program.  If not, see http://www.gnu.org/licenses/.
** $VREEN_END_LICENSE$
**
****************************************************************************/
#ifndef CONNECTION_P_H
#define CONNECTION_P_H
#include "connection.h"

namespace Vreen {

class Connection;
class VK_SHARED_EXPORT ConnectionPrivate
{
    Q_DECLARE_PUBLIC(Connection)
public:
    ConnectionPrivate(Connection *q) : q_ptr(q) {}
    Connection *q_ptr;
    QMap<Connection::ConnectionOption, QVariant> options;
};

}


#endif // CONNECTION_P_H
