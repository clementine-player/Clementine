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
#ifndef VK_MESSAGE_H
#define VK_MESSAGE_H
#include <QVariant>
#include <QSharedData>
#include "attachment.h"

namespace Vreen {

class Contact;
class Message;
typedef QList<Message> MessageList;

class Client;
class MessageData;
class VK_SHARED_EXPORT Message
{
    Q_GADGET
    Q_ENUMS(ReadState)
    Q_ENUMS(Direction)
    Q_ENUMS(Flags)
public:
    enum Flag {
        FlagUnread   = 1,
        FlagOutbox   = 2,
        FlagReplied  = 4,
        FlagImportant= 8,
        FlagChat     = 16,
        FlagFriends  = 32,
        FlagSpam     = 64,
        FlagDeleted  = 128,
        FlagFixed    = 256,
        FlagMedia    = 512
    };
    Q_DECLARE_FLAGS(Flags, Flag)
    enum Filter {
        FilterNone          = 0,
        FilterUnread        = 1,
        FilterNotFromChat   = 2,
        FilterFromFriends   = 4
    };

	Message(int clientId);
	Message(const QVariantMap &data, int clientId);
	Message(Client *client = 0);
    Message(const QVariantMap &data, Client *client);
    Message(const Message &other);
    Message &operator =(const Message &other);
    bool operator ==(const Message &other);
    virtual ~Message();

	int id() const;
    void setId(int id);
    QDateTime date() const;
    void setDate(const QDateTime &date);
    int fromId() const;
    void setFromId(int id);
    int toId() const;
    void setToId(int id);
    int chatId() const;
    void setChatId(int chatId);
	QString subject() const;
    void setSubject(const QString &subject);
    QString body() const;
    void setBody(const QString &body);
    bool isUnread() const;
    void setUnread(bool set);
    bool isIncoming() const;
    void setIncoming(bool set);
    void setFlags(Flags flags);
    Flags flags() const;
    void setFlag(Flag flag, bool set = true);
    bool testFlag(Flag flag) const;
    Attachment::Hash attachments() const;
    Attachment::List attachments(Attachment::Type type) const;
    void setAttachments(const Attachment::List &attachmentList);

    static MessageList fromVariantList(const QVariantList &list, Client *client);
	static MessageList fromVariantList(const QVariantList &list, int clientId);
private:
    QSharedDataPointer<MessageData> d;
};

typedef QList<int> IdList;

} // namespace Vreen

Q_DECLARE_METATYPE(Vreen::Message)
Q_DECLARE_METATYPE(Vreen::MessageList)
Q_DECLARE_METATYPE(Vreen::IdList)
Q_DECLARE_OPERATORS_FOR_FLAGS(Vreen::Message::Flags)

#endif // VK_MESSAGE_H

