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
#ifndef VK_GROUPCHATSESSION_H
#define VK_GROUPCHATSESSION_H
#include "messagesession.h"
#include "contact.h"

namespace Vreen {

class GroupChatSessionPrivate;

class VK_SHARED_EXPORT GroupChatSession : public Vreen::MessageSession
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(GroupChatSession)

    Q_PROPERTY(QString title READ title NOTIFY titleChanged)
    Q_PROPERTY(bool joined READ isJoined NOTIFY isJoinedChanged)
public:
    explicit GroupChatSession(int chatId, Client *parent);

    BuddyList participants() const;
    Buddy *admin() const;
    QString title() const;
    Buddy *findParticipant(int uid) const;
    //Buddy *participant(int uid);
    bool isJoined() const;

    static Reply *create(Client *client, const IdList &uids, const QString &title = QString());
public slots:
    Reply *getInfo();
    Reply *inviteParticipant(Contact *buddy);
    Reply *removeParticipant(Contact *buddy);
    Reply *updateTitle(const QString &title);
    void join();
    void leave();
signals:
    void participantAdded(Vreen::Buddy*);
    void participantRemoved(Vreen::Buddy*);
    void titleChanged(QString);
    void isJoinedChanged(bool);
protected:
    void setTitle(const QString &title);
    virtual SendMessageReply *doSendMessage(const Vreen::Message &message);
	virtual ReplyBase<MessageList> *doGetHistory(int count = 16, int offset = 0);
private:
    Q_PRIVATE_SLOT(d_func(), void _q_info_received(const QVariant &response))
    Q_PRIVATE_SLOT(d_func(), void _q_participant_added(const QVariant &response))
    Q_PRIVATE_SLOT(d_func(), void _q_participant_removed(const QVariant &response))
    Q_PRIVATE_SLOT(d_func(), void _q_title_updated(const QVariant &response))
    Q_PRIVATE_SLOT(d_func(), void _q_online_changed(bool))
    Q_PRIVATE_SLOT(d_func(), void _q_message_added(const Vreen::Message &))
    Q_PRIVATE_SLOT(d_func(), void _q_group_chat_updated(int chatId, bool self))
};

} // namespace Vreen

#endif // VK_GROUPCHATSESSION_H

