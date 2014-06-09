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
#ifndef VK_CHATSESSION_H
#define VK_CHATSESSION_H

#include "message.h"
#include "messagesession.h"

namespace Vreen {

class Reply;
class ChatSessionPrivate;
class VK_SHARED_EXPORT ChatSession : public MessageSession
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(ChatSession)

    Q_PROPERTY(Contact *contact READ contact CONSTANT)
public:
    ChatSession(Contact *contact);
    virtual ~ChatSession();

    Contact *contact() const;
    bool isActive() const;
    void setActive(bool set);
protected:
	virtual ReplyBase<MessageList> *doGetHistory(int count = 16, int offset = 0);
    virtual SendMessageReply *doSendMessage(const Vreen::Message &message);
private:

    Q_PRIVATE_SLOT(d_func(), void _q_message_added(const Vreen::Message &))
};

} // namespace Vreen

#endif // VK_CHATSESSION_H

