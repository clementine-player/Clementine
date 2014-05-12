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
#ifndef VK_GROUPMANAGER_H
#define VK_GROUPMANAGER_H

#include "contact.h"
#include <QObject>

namespace Vreen {

class Client;
class Group;
class GroupManagerPrivate;

class VK_SHARED_EXPORT GroupManager : public QObject
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(GroupManager)
public:
    explicit GroupManager(Client *client);
    virtual ~GroupManager();
    Client *client() const;
    Group *group(int gid) const;
    Group *group(int gid);
public slots:
    Reply *update(const IdList &ids, const QStringList &fields = QStringList() << VK_GROUP_FIELDS);
    Reply *update(const GroupList &groups, const QStringList &fields = QStringList() << VK_GROUP_FIELDS);
signals:
    void groupCreated(Group *group);
protected:
    QScopedPointer<GroupManagerPrivate> d_ptr;
private:

    Q_PRIVATE_SLOT(d_func(), void _q_update_finished(const QVariant &response))
    Q_PRIVATE_SLOT(d_func(), void _q_updater_handle())

    friend class Group;
    friend class GroupPrivate;
};

} // namespace Vreen

#endif // VK_GROUPMANAGER_H

