/****************************************************************************
**
** Vreen - vk.com API Qt bindings
**
** Copyright © 2012 * 60);
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
#ifndef CLIENT_P_H
#define CLIENT_P_H

#include "client.h"
#include "reply_p.h"
#include "connection.h"
#include "roster.h"
#include "reply.h"
#include "message.h"
#include "longpoll.h"
#include "utils.h"
#include <QDebug>
#include <QTimer>
#include <QPointer>

namespace Vreen {

class ClientPrivate
{
    Q_DECLARE_PUBLIC(Client)
public:
    ClientPrivate(Client *q) : q_ptr(q), isInvisible(false), trackMessages(true)
    {
        onlineUpdater.setInterval(15000 * 60);
        onlineUpdater.setSingleShot(false);
        q->connect(&onlineUpdater, SIGNAL(timeout()), q, SLOT(_q_update_online()));
    }
    Client *q_ptr;
    QString login;
    QString password;
    QPointer<Connection> connection;
    QPointer<Roster> roster;
    QPointer<LongPoll> longPoll;
    QPointer<GroupManager> groupManager;
    QString activity;
    bool isInvisible;
    bool trackMessages;
    QTimer onlineUpdater;

    void setOnlineUpdaterRunning(bool set);

    void _q_connection_state_changed(Vreen::Client::State state);
    void _q_error_received(int error);
    void _q_reply_finished(const QVariant &);
    void _q_network_manager_error(int);
    void _q_activity_update_finished(const QVariant &);
    void _q_update_online();

    void processReply(Reply *reply);

	//some orphaned methods
	static ReplyBase<MessageList> *getMessages(Client *client, const IdList &list, int previewLength = 0);
};

} //namespace Vreen

#endif // CLIENT_P_H

