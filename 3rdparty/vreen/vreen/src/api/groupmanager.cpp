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
#include "client.h"
#include "contact.h"
#include "utils_p.h"
#include <QTimer>
#include "groupmanager_p.h"

namespace Vreen {

GroupManager::GroupManager(Client *client) :
    QObject(client),
    d_ptr(new GroupManagerPrivate(this, client))
{
}

GroupManager::~GroupManager()
{
}

Client *GroupManager::client() const
{
    return d_func()->client;
}

Group *GroupManager::group(int gid)
{
    Q_D(GroupManager);
    auto group = d->groupHash.value(gid);
    if (!group) {
        group = new Group(gid, client());
        d->groupHash.insert(gid, group);
    }
    return group;
}

Group *GroupManager::group(int gid) const
{
    return d_func()->groupHash.value(gid);
}

Reply *GroupManager::update(const IdList &ids, const QStringList &fields)
{
    Q_D(GroupManager);
    QVariantMap args;
    args.insert("gids", join(ids));
    args.insert("fields", fields.join(","));
    auto reply = d->client->request("groups.getById", args);
    reply->connect(reply, SIGNAL(resultReady(const QVariant&)),
                   this, SLOT(_q_update_finished(const QVariant&)));
    return reply;
}

Reply *GroupManager::update(const GroupList &groups, const QStringList &fields)
{
    IdList ids;
    foreach (auto group, groups)
        ids.append(group->id());
    return update(ids, fields);
}

void GroupManagerPrivate::_q_update_finished(const QVariant &response)
{
    Q_Q(GroupManager);
    auto list = response.toList();
    foreach (auto data, list) {
        auto map = data.toMap();
        int id = -map.value("gid").toInt();
        Contact::fill(q->group(id), map);
    }
}

void GroupManagerPrivate::_q_updater_handle()
{
    Q_Q(GroupManager);
    q->update(updaterQueue);
    updaterQueue.clear();
}

void GroupManagerPrivate::appendToUpdaterQueue(Group *contact)
{
    if (!updaterQueue.contains(-contact->id()))
        updaterQueue.append(-contact->id());
    if (!updaterTimer.isActive())
        updaterTimer.start();
}

} // namespace Vreen

#include "moc_groupmanager.cpp"
