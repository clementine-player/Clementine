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
#include "messagesession_p.h"
#include "client.h"
#include "utils_p.h"

namespace Vreen {

MessageSession::MessageSession(MessageSessionPrivate *data) :
    QObject(data->client),
    d_ptr(data)
{
}
MessageSession::~MessageSession()
{
}

Client *MessageSession::client() const
{
    return d_func()->client;
}

int MessageSession::uid() const
{
    return d_func()->uid;
}

QString MessageSession::title() const
{
    return d_func()->title;
}

void MessageSession::setTitle(const QString &title)
{
    Q_D(MessageSession);
    if (d->title != title) {
        d->title = title;
        emit titleChanged(title);
    }
}

ReplyBase<MessageList> *MessageSession::getHistory(int count, int offset)
{
	auto reply = doGetHistory(count, offset);
    connect(reply, SIGNAL(resultReady(QVariant)), SLOT(_q_history_received(QVariant)));
	return reply;
}

SendMessageReply *MessageSession::sendMessage(const QString &body, const QString &subject)
{
    Q_D(MessageSession);
    Message msg(d->client);
    msg.setToId(d->uid);
    msg.setBody(body);
    msg.setSubject(subject);
    return sendMessage(msg);
}

SendMessageReply *MessageSession::sendMessage(const Message &message)
{
    return doSendMessage(message);
}

Reply *MessageSession::markMessagesAsRead(IdList ids, bool set)
{
    Q_D(MessageSession);
    QString request = set ? "messages.markAsRead"
                          : "messages.markAsNew";
    QVariantMap args;
    args.insert("mids", join(ids));
    auto reply = d->client->request(request, args);
    reply->setProperty("mids", qVariantFromValue(ids));
    reply->setProperty("set", set);
    return reply;
}

void MessageSessionPrivate::_q_history_received(const QVariant &)
{
	Q_Q(MessageSession);
	auto reply = static_cast<ReplyBase<MessageList>*>(q->sender());
	foreach (auto message, reply->result())
		emit q->messageAdded(message);
}

} // namespace Vreen

#include "moc_messagesession.cpp"

