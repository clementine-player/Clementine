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
#include "groupchatsession.h"
#include "messagesession_p.h"
#include "client_p.h"
#include "reply_p.h"
#include "roster.h"
#include <QSet>

namespace Vreen {

class GroupChatSession;
class GroupChatSessionPrivate : public MessageSessionPrivate
{
    Q_DECLARE_PUBLIC(GroupChatSession)
public:
    GroupChatSessionPrivate(MessageSession *q, Client *client, int uid) :
        MessageSessionPrivate(q, client, uid),
        adminId(0)
    {}
    QString title;
    QHash<int, Buddy*> buddies;
    int adminId;

    Buddy *addContact(int id);
    void removeContact(int id);

    void _q_message_sent(const QVariant &response);
    void _q_info_received(const QVariant &response);
    void _q_participant_added(const QVariant &response);
    void _q_participant_removed(const QVariant &response);
    void _q_title_updated(const QVariant &response);
    void _q_online_changed(bool set);
    void _q_message_added(const Vreen::Message &);
    void _q_group_chat_updated(int chatId, bool self);
};

/*!
 * \brief The GroupChatSession class, based on
 * \link http://vk.com/developers.php?oid=-1&p=messages.getChat
 * \link http://vk.com/developers.php?o=-1&p=messages.createChat
 * etc
 */

/*!
 * \brief GroupChatSession::GroupChatSession
 * \param chatId
 * \param client
 */

GroupChatSession::GroupChatSession(int chatId, Client *client) :
    MessageSession(new GroupChatSessionPrivate(this, client, chatId))
{
    connect(client, SIGNAL(onlineStateChanged(bool)), SLOT(_q_online_changed(bool)));
    connect(client->longPoll(), SIGNAL(groupChatUpdated(int,bool)), SLOT(_q_group_chat_updated(int,bool)));
}

BuddyList GroupChatSession::participants() const
{
    return d_func()->buddies.values();
}

Buddy *GroupChatSession::admin() const
{
    return static_cast<Buddy*>(findParticipant(d_func()->adminId));
}

QString GroupChatSession::title() const
{
    return d_func()->title;
}

Buddy *GroupChatSession::findParticipant(int uid) const
{
    foreach (auto buddy, d_func()->buddies)
        if (buddy->id() == uid)
            return buddy;
    return 0;
}

bool GroupChatSession::isJoined() const
{
    Q_D(const GroupChatSession);
    return d->buddies.contains(d->client->me()->id())
            && d->client->isOnline();
}

//Buddy *GroupChatSession::participant(int uid)
//{
//    auto p = findParticipant(uid);
//    if (!p)
//        p = d_func()->addContact(uid);
//    return p;
//}

Reply *GroupChatSession::create(Client *client, const IdList &uids, const QString &title)
{
    QStringList list;
    foreach (auto &uid, uids)
        list.append(QString::number(uid));
    QVariantMap args;
    args.insert("uids", list.join(","));
    args.insert("title", title);
    return client->request("messages.createChat", args);
}

void GroupChatSession::setTitle(const QString &title)
{
    Q_D(GroupChatSession);
    if (d->title != title) {
        d->title = title;
        emit titleChanged(title);
    }
}

ReplyBase<MessageList> *GroupChatSession::doGetHistory(int count, int offset)
{
    Q_D(GroupChatSession);
    QVariantMap args;
    args.insert("count", count);
    args.insert("offset", offset);
    args.insert("chat_id", d->uid);

	auto reply = d->client->request<ReplyBase<MessageList>>("messages.getHistory",
																	  args,
																	  MessageListHandler(d->client->id()));
	return reply;
}

SendMessageReply *GroupChatSession::doSendMessage(const Message &message)
{
    Q_D(GroupChatSession);
    QVariantMap args;
    //TODO move to client
    args.insert("chat_id", d->uid);
    args.insert("message", message.body());
    args.insert("title", message.subject());

	return d->client->request<SendMessageReply>("messages.send", args, ReplyPrivate::handleInt);
}

Reply *GroupChatSession::getInfo()
{
    Q_D(GroupChatSession);
    QVariantMap args;
    args.insert("chat_id", d->uid);

    auto reply = d->client->request("messages.getChat", args);
    connect(reply, SIGNAL(resultReady(QVariant)), SLOT(_q_info_received(QVariant)));
    return reply;
}

/*!
 * \brief GroupChatSession::inviteParticipant \link http://vk.com/developers.php?oid=-1&p=messages.addChatUser
 * \param buddy
 * \return
 */
Reply *GroupChatSession::inviteParticipant(Contact *buddy)
{
    Q_D(GroupChatSession);
    QVariantMap args;
    args.insert("chat_id", d->uid);
    args.insert("uid", buddy->id());

    auto reply = d->client->request("messages.addChatUser", args);
    reply->setProperty("uid", buddy->id());
    connect(reply, SIGNAL(resultReady(QVariant)), SLOT(_q_participant_added(QVariant)));
    return reply;
}

Reply *GroupChatSession::removeParticipant(Contact *buddy)
{
    Q_D(GroupChatSession);
    QVariantMap args;
    args.insert("chat_id", d->uid);
    args.insert("uid", buddy->id());

    auto reply = d->client->request("messages.removeChatUser", args);
    reply->setProperty("uid", buddy->id());
    connect(reply, SIGNAL(resultReady(QVariant)), SLOT(_q_participant_removed(QVariant)));
    return reply;
}

Reply *GroupChatSession::updateTitle(const QString &title)
{
    Q_D(GroupChatSession);
    QVariantMap args;
    args.insert("chat_id", d->uid);
    args.insert("title", title);

    auto reply = d->client->request("messages.editChat", args);
    connect(reply, SIGNAL(resultReady(QVariant)), SLOT(_q_title_updated(QVariant)));
    return reply;
}

void GroupChatSession::join()
{
    Q_D(GroupChatSession);
    if (!isJoined() && d->client->isOnline())
        inviteParticipant(d_func()->client->me());
}

void GroupChatSession::leave()
{
    if (isJoined())
        removeParticipant(d_func()->client->me());
}

void GroupChatSessionPrivate::_q_info_received(const QVariant &response)
{
    Q_Q(GroupChatSession);
    auto map = response.toMap();
    adminId = map.value("admin_id").toInt();
    q->setTitle(map.value("title").toString());

    QSet<int> uidsToAdd;
    foreach (auto item, map.value("users").toList())
        uidsToAdd.insert(item.toInt());

    QSet<int> uids = buddies.keys().toSet();
    QSet<int> uidsToRemove = uids  - uidsToAdd;
    uidsToAdd -= uids;
    foreach (auto uid, uidsToRemove)
        removeContact(uid);
    foreach (auto uid, uidsToAdd)
        addContact(uid);
}

void GroupChatSessionPrivate::_q_participant_added(const QVariant &response)
{
    if (response.toInt() == 1) {
        int id = q_func()->sender()->property("uid").toInt();
        addContact(id);
    }
}

void GroupChatSessionPrivate::_q_participant_removed(const QVariant &response)
{
    if (response.toInt() == 1) {
        int id = q_func()->sender()->property("uid").toInt();
        removeContact(id);
    }
}

void GroupChatSessionPrivate::_q_title_updated(const QVariant &response)
{
    Q_Q(GroupChatSession);
    auto map = response.toMap();
    q->setTitle(map.value("title").toString());
}

void GroupChatSessionPrivate::_q_online_changed(bool set)
{
    foreach (auto contact, buddies) {
        if (auto buddy = qobject_cast<Buddy*>(contact)) {
            if (set)
                buddy->update();
            else
                buddy->setOnline(false);
        }
    }
    if (!set)
        emit q_func()->isJoinedChanged(false);
}

void GroupChatSessionPrivate::_q_message_added(const Message &msg)
{
    if (msg.chatId() == uid) {
        emit q_func()->messageAdded(msg);
    }
}

void GroupChatSessionPrivate::_q_group_chat_updated(int chatId, bool)
{
    Q_Q(GroupChatSession);
    if (chatId == uid)
        q->getInfo();
}

Buddy *GroupChatSessionPrivate::addContact(int id)
{
    Q_Q(GroupChatSession);
    if (id) {
        if (!buddies.contains(id)) {
            auto contact = client->roster()->buddy(id);
            buddies.insert(id, contact);
            emit q->participantAdded(contact);
            if (contact == client->me()) {
                emit q->isJoinedChanged(true);
            }
            return contact;
        }
    }
    return 0;
}

void GroupChatSessionPrivate::removeContact(int id)
{
    Q_Q(GroupChatSession);
    if (id) {
        if (buddies.contains(id)) {
            buddies.remove(id);
            auto contact = client->roster()->buddy(id);
            emit q->participantRemoved(contact);
            if (contact == client->me())
                emit q->isJoinedChanged(false);
        }
    }
}

} // namespace Vreen

#include "moc_groupchatsession.cpp"

