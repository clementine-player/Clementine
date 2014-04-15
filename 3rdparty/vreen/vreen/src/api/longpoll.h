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
#ifndef VK_LONGPOLL_H
#define VK_LONGPOLL_H

#include <QObject>
#include "contact.h"

namespace Vreen {

class Message;
class Client;
class LongPollPrivate;
class VK_SHARED_EXPORT LongPoll : public QObject
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(LongPoll)
    Q_PROPERTY(int pollInterval READ pollInterval WRITE setPollInterval NOTIFY pollIntervalChanged)
public:

    enum ServerAnswer {
        MessageDeleted      = 0,
        MessageFlagsReplaced= 1,
        MessageFlagsSet     = 2,
        MessageFlagsReseted = 3,
        MessageAdded        = 4,
        UserOnline          = 8,
        UserOffline         = 9,
        GroupChatUpdated    = 51,
        ChatTyping          = 61,
        GroupChatTyping     = 62,
        UserCall            = 70

    };

    enum OfflineFlag {
        OfflineTimeout = 1
    };
    Q_DECLARE_FLAGS(OfflineFlags, OfflineFlag)

    enum Mode {
        NoRecieveAttachments = 0,
        RecieveAttachments = 2
    };

    LongPoll(Client *client);
    virtual ~LongPoll();
    void setMode(Mode mode);
    Mode mode() const;
    int pollInterval() const;
    void setPollInterval(int interval);
signals:
    void messageAdded(const Vreen::Message &msg);
    void messageDeleted(int mid);
    void messageFlagsReplaced(int mid, int mask, int userId = 0);
    void messageFlagsReseted(int mid, int mask, int userId = 0);
    void contactStatusChanged(int userId, Vreen::Contact::Status status);
    void contactTyping(int userId, int chatId = 0);
    void contactCall(int userId, int callId);
    void groupChatUpdated(int chatId, bool self);
    void pollIntervalChanged(int);
public slots:
    void setRunning(bool set);
protected slots:
    void requestServer();
    void requestData(const QByteArray &timeStamp);
protected:
    QScopedPointer<LongPollPrivate> d_ptr;

    Q_PRIVATE_SLOT(d_func(), void _q_request_server_finished(const QVariant &))
    Q_PRIVATE_SLOT(d_func(), void _q_on_data_recieved(const QVariant &))
    Q_PRIVATE_SLOT(d_func(), void _q_update_running());
};

} // namespace Vreen

Q_DECLARE_METATYPE(Vreen::LongPoll*)

#endif // VK_LONGPOLL_H

