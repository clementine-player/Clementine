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
#ifndef VK_MESSAGESESSION_H
#define VK_MESSAGESESSION_H

#include <QObject>
#include "message.h"
#include "client.h"

namespace Vreen {

class Message;
class Client;
class MessageSessionPrivate;

class VK_SHARED_EXPORT MessageSession : public QObject
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(MessageSession)

    Q_PROPERTY(int uid READ uid CONSTANT)
    Q_PROPERTY(Client* client READ client CONSTANT)
    Q_PROPERTY(QString title READ title WRITE setTitle NOTIFY titleChanged)
public:
    explicit MessageSession(MessageSessionPrivate *data);
    virtual ~MessageSession();
    Client *client() const;
    int uid() const;
    QString title() const;
public slots:
	ReplyBase<MessageList> *getHistory(int count = 16, int offset = 0);
    SendMessageReply *sendMessage(const QString &body, const QString &subject = QString());
    SendMessageReply *sendMessage(const Message &message);
    Reply *markMessagesAsRead(IdList ids, bool set = true);
    void setTitle(const QString &title);
signals:
    void messageAdded(const Vreen::Message &message);
    void messageDeleted(int id);
    void messageReadStateChanged(int mid, bool isRead);
    void titleChanged(const QString &title);
protected:
    virtual SendMessageReply *doSendMessage(const Vreen::Message &message) = 0;
	virtual ReplyBase<MessageList> *doGetHistory(int count, int offset) = 0;
    QScopedPointer<MessageSessionPrivate> d_ptr;
private:
	Q_PRIVATE_SLOT(d_func(), void _q_history_received(const QVariant &))
};

} // namespace Vreen

#endif // VK_MESSAGESESSION_H

