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
#ifndef REPLY_P_H
#define REPLY_P_H
#include "reply.h"
#include "json.h"
#include "reply.h"
#include "QNetworkReply"
#include <QDebug>
#include <QPointer>
#include <QSharedPointer>

namespace Vreen {

class ReplyPrivate
{
    Q_DECLARE_PUBLIC(Reply)
public:
    ReplyPrivate(Reply *q) : q_ptr(q), resultHandler(0) {}
    Reply *q_ptr;

    QPointer<QNetworkReply> networkReply;
    QVariant response;
    QVariant error;
    QScopedPointer<Reply::ResultHandlerBase> resultHandler;
    QVariant result;

    void _q_reply_finished();
    void _q_network_reply_error(QNetworkReply::NetworkError);

    static QVariant handleInt(const QVariant &response) { return response.toInt(); }
    static QVariant handleIdList(const QVariant& response);
};


struct MessageListHandler {
    MessageListHandler(int clientId) : clientId(clientId) {}
    QVariant operator()(const QVariant &response);

    int clientId;
};

} //namespace Vreen

#endif // REPLY_P_H

