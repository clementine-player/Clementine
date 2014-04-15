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
#include "message.h"
#include "contact.h"
#include "client.h"
#include "roster.h"
#include <QDateTime>
#include "dynamicpropertydata_p.h"
#include "utils_p.h"
#include <QDebug>

namespace Vreen {

class MessageData : public DynamicPropertyData
{
public:
	MessageData(int clientId) :
		clientId(clientId),
        messageId(0),
        fromId(0),
        toId(0),
        chatId(0),
		userCount(0),
		adminId(0),
        latitude(-1),
        longitude(-1)
    {}
    MessageData(const MessageData &o) :
        DynamicPropertyData(o),
		clientId(o.clientId),
        messageId(o.messageId),
        fromId(o.fromId),
        toId(o.toId),
        date(o.date),
        flags(o.flags),
        subject(o.subject),
        body(o.body),
        forwardMsgIds(o.forwardMsgIds),
        chatId(o.chatId),
        chatActive(o.chatActive),
        userCount(o.userCount),
		adminId(o.adminId),
        latitude(o.latitude),
        longitude(o.longitude),
        attachmentHash(o.attachmentHash)
    {}
    ~MessageData() {}

	int clientId;
    int messageId;
    int fromId;
    int toId;
    QDateTime date;
    Message::Flags flags;
    QString subject;
    QString body;
    QList<int> forwardMsgIds;
    int chatId;
    QList<int> chatActive;
    int userCount;
	int adminId;
    qreal latitude;
    qreal longitude;
    Attachment::Hash attachmentHash;

    void fill(const QVariantMap &data)
    {
        messageId = data.value("mid").toInt();

		int contactId = data.value("from_id").toInt();
		if (contactId) {
			bool isIncoming = (contactId == clientId);
            setFlag(Message::FlagOutbox, !isIncoming);
            if (isIncoming) {
				fromId = clientId;
                toId = 0;
            } else {
				fromId = contactId;
				toId = clientId;
            }
        } else {
            setFlag(Message::FlagOutbox, data.value("out").toBool());
			contactId = data.value("uid").toInt();
            if (!flags.testFlag(Message::FlagOutbox)) {
				fromId = contactId;
				toId = clientId;
            } else {
				toId = contactId;
				fromId = clientId;
            }
        }

        date = QDateTime::fromTime_t(data.value("date").toInt());
        setFlag(Message::FlagUnread, !data.value("read_state").toBool());
        subject = fromHtmlEntities(data.value("title").toString());
        body = fromHtmlEntities(data.value("body").toString());
        attachmentHash = Attachment::toHash(Attachment::fromVariantList(data.value("attachments").toList()));
        //TODO forward messages
        chatId = data.value("chat_id").toInt();
    }

    void setFlag(Message::Flag flag, bool set = true)
    {
        if (set)
            flags |= flag;
        else
            flags &= ~flag;
    }

	int getId(Contact *contact) const
	{
		return contact ? contact->id() : 0;
	}
};


/*!
 * \brief The Message class
 * Api reference: \link http://vk.com/developers.php?oid=-1&p=Формат_описания_личных_сообщений */

Message::Message(Client *client) :
	d(new MessageData(client->id()))
{
}

Message::Message(int clientId) :
	d(new MessageData(clientId))
{
}

Message::Message(const QVariantMap &data, int clientId) :
	d(new MessageData(clientId))
{
	d->fill(data);
}

Message::Message(const QVariantMap &data, Client *client) :
	d(new MessageData(client->id()))
{
    d->fill(data);
}

Message::Message(const Message &other) : d(other.d)
{
}

Message &Message::operator =(const Message &other)
{
    if (this != &other)
        d.operator=(other.d);
    return *this;
}

bool Message::operator ==(const Message &other)
{
    return id() == other.id();
}

Message::~Message()
{
}

int Message::id() const
{
    return d->messageId;
}

void Message::setId(int id)
{
    d->messageId = id;
}

QDateTime Message::date() const
{
    return d->date;
}

void Message::setDate(const QDateTime &date)
{
    d->date = date;
}

int Message::fromId() const
{
    return d->fromId;
}

void Message::setFromId(int id)
{
    d->fromId = id;
}

int Message::toId() const
{
    return d->toId;
}

void Message::setToId(int id)
{
    d->toId = id;
}

int Message::chatId() const
{
    return d->chatId;
}

void Message::setChatId(int chatId)
{
    d->chatId = chatId;
}

QString Message::subject() const
{
    return d->subject;
}

void Message::setSubject(const QString &title)
{
    d->subject = title;
}

QString Message::body() const
{
    return d->body;
}

void Message::setBody(const QString &body)
{
	d->body = body;
}

bool Message::isUnread() const
{
    return testFlag(FlagUnread);
}

void Message::setUnread(bool set)
{
    setFlag(FlagUnread, set);
}

bool Message::isIncoming() const
{
    return !testFlag(FlagOutbox);
}

void Message::setIncoming(bool set)
{
    setFlag(FlagOutbox, !set);
}

void Message::setFlags(Message::Flags flags)
{
    d->flags = flags;
}

Message::Flags Message::flags() const
{
    return d->flags;
}

void Message::setFlag(Flag flag, bool set)
{
    d->setFlag(flag, set);
}

bool Message::testFlag(Flag flag) const
{
    return d->flags.testFlag(flag);
}

Attachment::Hash Message::attachments() const
{
    return d->attachmentHash;
}

Attachment::List Message::attachments(Attachment::Type type) const
{
    return d->attachmentHash.values(type);
}

void Message::setAttachments(const Attachment::List &attachmentList)
{
    d->attachmentHash = Attachment::toHash(attachmentList);
}

MessageList Message::fromVariantList(const QVariantList &list, Vreen::Client *client)
{
	return fromVariantList(list, client->id());
}

MessageList Message::fromVariantList(const QVariantList &list, int clientId)
{
	MessageList messageList;
	foreach (auto item, list) {
		Vreen::Message message(item.toMap(), clientId);
		messageList.append(message);
	}
	return messageList;
}

} // namespace Vreen

#include "moc_message.cpp"
