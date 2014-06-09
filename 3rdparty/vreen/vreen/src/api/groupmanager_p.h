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

#ifndef GROUPMANAGER_P_H
#define GROUPMANAGER_P_H

#include "groupmanager.h"
#include "client.h"
#include "contact.h"
#include <QTimer>

namespace Vreen {

class GroupManager;
class GroupManagerPrivate
{
    Q_DECLARE_PUBLIC(GroupManager)
public:
    GroupManagerPrivate(GroupManager *q, Client *client) : q_ptr(q), client(client)
    {
        updaterTimer.setInterval(5000);
        updaterTimer.setSingleShot(true);
        updaterTimer.connect(&updaterTimer, SIGNAL(timeout()),
                             q, SLOT(_q_updater_handle()));
    }
    GroupManager *q_ptr;
    Client *client;
    QHash<int, Group*> groupHash;

    //updater
    QTimer updaterTimer;
    IdList updaterQueue;

    void _q_update_finished(const QVariant &response);
    void _q_updater_handle();
    void appendToUpdaterQueue(Group *contact);
};

} //namespace Vreen

#endif // GROUPMANAGER_P_H
