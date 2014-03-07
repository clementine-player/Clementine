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
#ifndef VK_ROSTER_H
#define VK_ROSTER_H

#include "contact.h"
#include "message.h"
#include "reply.h"
#include "friendrequest.h"
#include <QVariant>
#include <QStringList>

namespace Vreen {
class Client;

class RosterPrivate;
class VK_SHARED_EXPORT Roster : public QObject
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(Roster)
	Q_FLAGS(FriendRequestFlags)
public:

    enum NameCase {
        NomCase,
        GenCase,
        DatCase,
        AccCase,
        InsCase,
        AblCase
    };  

	enum FriendRequestFlag {
		NeedMutualFriends,
		NeedMessages,
		GetOutRequests
	};
	Q_DECLARE_FLAGS(FriendRequestFlags, FriendRequestFlag)

    Roster(Client *client, int uid = 0);
    virtual ~Roster();
    void setUid(int uid);
    int uid() const;

    Buddy *owner() const;
    Buddy *buddy(int id);
    Buddy *buddy(int id) const;
    BuddyList buddies() const;

    QMap<int, QString> tags() const;
    void setTags(const QMap<int, QString> &list);
    Reply *getDialogs(int offset = 0, int count = 16, int previewLength = -1);
    Reply *getMessages(int offset = 0, int count = 50, Message::Filter filter = Message::FilterNone);
public slots:
    void sync(const QStringList &fields = QStringList()
            << VK_COMMON_FIELDS
            );
    Reply *update(const IdList &ids, const QStringList &fields = QStringList()
            << VK_ALL_FIELDS
            );
    Reply *update(const BuddyList &buddies, const QStringList &fields = QStringList()
            << VK_ALL_FIELDS
            );
	ReplyBase<FriendRequestList> *getFriendRequests(int count = 100, int offset = 0, FriendRequestFlags flags = NeedMessages);
signals:
    void buddyAdded(Vreen::Buddy *buddy);
    void buddyUpdated(Vreen::Buddy *buddy);
	void buddyRemoved(int id);
    void tagsChanged(const QMap<int, QString> &);
    void syncFinished(bool success);
    void uidChanged(int uid);
protected:
    QScopedPointer<RosterPrivate> d_ptr;

    //friend class Contact;
    friend class Buddy;
    //friend class Group;

    Q_PRIVATE_SLOT(d_func(), void _q_tags_received(const QVariant &response))
    Q_PRIVATE_SLOT(d_func(), void _q_friends_received(const QVariant &response))
    Q_PRIVATE_SLOT(d_func(), void _q_status_changed(int userId, Vreen::Contact::Status status))
    Q_PRIVATE_SLOT(d_func(), void _q_online_changed(bool))
    Q_PRIVATE_SLOT(d_func(), void _q_updater_handle())
};

} // namespace Vreen

Q_DECLARE_METATYPE(Vreen::Roster*)
Q_DECLARE_OPERATORS_FOR_FLAGS(Vreen::Roster::FriendRequestFlags)

#endif // VK_ROSTER_H

