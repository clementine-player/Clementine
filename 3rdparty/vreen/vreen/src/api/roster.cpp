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
#include "roster_p.h"
#include "longpoll.h"
#include "utils_p.h"

#include <QDebug>
#include <QTimer>

namespace Vreen {

/*!
 * \brief The Roster class
 * Api reference: \link http://vk.com/developers.php?oid=-1&p=friends.get
 */

/*!
 * \brief Roster::Roster
 * \param client
 */
Roster::Roster(Client *client, int uid) :
    QObject(client),
    d_ptr(new RosterPrivate(this, client))
{
    Q_D(Roster);
    connect(d->client->longPoll(), SIGNAL(contactStatusChanged(int, Vreen::Contact::Status)),
            this, SLOT(_q_status_changed(int, Vreen::Contact::Status)));
    connect(d->client, SIGNAL(onlineStateChanged(bool)), SLOT(_q_online_changed(bool)));
    if (uid)
        setUid(uid);
}

Roster::~Roster()
{

}

void Roster::setUid(int uid)
{
    Q_D(Roster);
    if (uid) {
        if (d->owner) {
            if (uid == d->owner->id())
                return;
            else
                qDeleteAll(d->buddyHash);
        }
        d->owner = buddy(uid);
        emit uidChanged(uid);
    } else {
        qDeleteAll(d->buddyHash);
        emit uidChanged(uid);
    }
}

int Roster::uid() const
{
    Q_D(const Roster);
    if (d->owner)
        return d->owner->id();
    return 0;
}

Buddy *Roster::owner() const
{
    return d_func()->owner;
}

Buddy *Roster::buddy(int id)
{
    Q_D(Roster);
    if (!id) {
        qWarning("Contact id cannot be null!");
        return 0;
    }
    if (id < 0) {
        id = qAbs(id);
        qWarning("For groups use class GroupManager");
    }


    auto buddy = d->buddyHash.value(id);
    if (!buddy) {
        if (d->owner && d->owner->id() == id)
            return d->owner;
        buddy = new Buddy(id, d->client);
        d->addBuddy(buddy);
    }
    return buddy;
}

Buddy *Roster::buddy(int id) const
{
    return d_func()->buddyHash.value(id);
}

BuddyList Roster::buddies() const
{
    return d_func()->buddyHash.values();
}

QMap<int, QString> Roster::tags() const
{
    return d_func()->tags;
}

void Roster::setTags(const QMap<int, QString> &tags)
{
    d_func()->tags = tags;
    emit tagsChanged(tags);
}

/*!
 * \brief Roster::getDialogs
 * \param offset
 * \param count
 * \param previewLength
 * \return
 */
Reply *Roster::getDialogs(int offset, int count, int previewLength)
{
    QVariantMap args;
    args.insert("count", count);
    args.insert("offset", offset);
    if (previewLength != -1)
        args.insert("preview_length", previewLength);
    return d_func()->client->request("messages.getDialogs", args);
}

/*!
 * \brief Roster::getMessages
 * \param offset
 * \param count
 * \param filter
 * \return
 */
Reply *Roster::getMessages(int offset, int count, Message::Filter filter)
{
    QVariantMap args;
    args.insert("count", count);
    args.insert("offset", offset);
    args.insert("filter", filter);
    return d_func()->client->request("messages.get", args);
}

void Roster::sync(const QStringList &fields)
{
    Q_D(Roster);
    //TODO rewrite with method chains with lambdas in Qt5
    QVariantMap args;
    args.insert("fields", fields.join(","));
    args.insert("order", "hints");

    d->getTags();
    d->getFriends(args);
}

/*!
 * \brief Roster::update
 * \param ids
 * \param fields from \link http://vk.com/developers.php?oid=-1&p=Описание_полей_параметра_fields
 */
Reply *Roster::update(const IdList &ids, const QStringList &fields)
{
    Q_D(Roster);
    QVariantMap args;
    args.insert("uids", join(ids));
    args.insert("fields", fields.join(","));
    auto reply = d->client->request("users.get", args);
    reply->connect(reply, SIGNAL(resultReady(const QVariant&)),
                   this, SLOT(_q_friends_received(const QVariant&)));
    return reply;
}

Reply *Roster::update(const BuddyList &buddies, const QStringList &fields)
{
    IdList ids;
    foreach (auto buddy, buddies)
        ids.append(buddy->id());
	return update(ids, fields);
}

ReplyBase<FriendRequestList> *Roster::getFriendRequests(int count, int offset, FriendRequestFlags flags)
{
	Q_D(Roster);
	QVariantMap args;
	args.insert("count", count);
	args.insert("offset", offset);
	if (flags & NeedMutualFriends)
		args.insert("need_mutual", true);
	if (flags & NeedMessages)
		args.insert("need_messages", true);
	if (flags & GetOutRequests)
		args.insert("out", 1);
	auto reply = d->client->request<ReplyBase<FriendRequestList>>("friends.getRequests",
																  args,
																  RosterPrivate::handleGetRequests);
	return reply;
}

void RosterPrivate::getTags()
{
    Q_Q(Roster);
    auto reply = client->request("friends.getLists");
    reply->connect(reply, SIGNAL(resultReady(const QVariant&)),
                   q, SLOT(_q_tags_received(const QVariant&)));
}

void RosterPrivate::getOnline()
{
}

void RosterPrivate::getFriends(const QVariantMap &args)
{
    Q_Q(Roster);
    auto reply = client->request("friends.get", args);
    reply->setProperty("friend", true);
    reply->setProperty("sync", true);
    reply->connect(reply, SIGNAL(resultReady(const QVariant&)),
                   q, SLOT(_q_friends_received(const QVariant&)));
}

void RosterPrivate::addBuddy(Buddy *buddy)
{
    Q_Q(Roster);
    if (!buddy->isFriend()) {
        IdList ids;
        ids.append(buddy->id());
        //q->update(ids, QStringList() << VK_COMMON_FIELDS); //TODO move!
    }
    buddyHash.insert(buddy->id(), buddy);
    emit q->buddyAdded(buddy);
}

void RosterPrivate::appendToUpdaterQueue(Buddy *contact)
{
    if (!updaterQueue.contains(contact->id()))
        updaterQueue.append(contact->id());
    if (!updaterTimer.isActive())
        updaterTimer.start();
}

QVariant RosterPrivate::handleGetRequests(const QVariant &response)
{
	FriendRequestList list;
	foreach (auto item, response.toList()) {
		QVariantMap map = item.toMap();
		FriendRequest request(map.value("uid").toInt());

		request.setMessage(map.value("message").toString());
		IdList ids;
		QVariantMap mutuals = map.value("mutual").toMap();
		foreach (auto user, mutuals.value("users").toList())
			ids.append(user.toInt());
		request.setMutualFriends(ids);
		list.append(request);
	}
	return QVariant::fromValue(list);
}

void RosterPrivate::_q_tags_received(const QVariant &response)
{
    Q_Q(Roster);
    auto list = response.toList();
    QMap<int, QString> tags;
    foreach (auto item, list) {
        tags.insert(item.toMap().value("lid").toInt(),item.toMap().value("name").toString());
    }
    q->setTags(tags);
}

void RosterPrivate::_q_friends_received(const QVariant &response)
{
    Q_Q(Roster);
    bool isFriend = q->sender()->property("friend").toBool();
    bool isSync = q->sender()->property("sync").toBool();
    auto list = response.toList();
    foreach (auto data, list) {
        auto map = data.toMap();
        int id = map.value("uid").toInt();
        auto buddy = buddyHash.value(id);
        if (!buddy) {
            buddy = new Buddy(id, client);
            Contact::fill(buddy, map);
            buddy->setIsFriend(isFriend);
            buddyHash[id] = buddy;
            if (!isSync)
                emit q->buddyAdded(buddy);
        } else {
            buddy->setIsFriend(isFriend);
            Contact::fill(buddy, map);
            if (!isSync)
                emit q->buddyUpdated(buddy);
        }
    }
    emit q->syncFinished(true);
}

void RosterPrivate::_q_status_changed(int userId, Buddy::Status status)
{
    Q_Q(Roster);
    auto buddy = q->buddy(userId);
    buddy->setStatus(status);
}

void RosterPrivate::_q_online_changed(bool set)
{
    if (!set)
        foreach(auto buddy, buddyHash)
            buddy->setOnline(false);
}

void RosterPrivate::_q_updater_handle()
{
    Q_Q(Roster);
    q->update(updaterQueue);
    updaterQueue.clear();
}


} // namespace Vreen

#include "moc_roster.cpp"

