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
#include "reply_p.h"
#include <QNetworkReply>
#include "client.h"
#include "message.h"

namespace Vreen {

Reply::Reply(QNetworkReply *reply) :
    QObject(reply),
    d_ptr(new ReplyPrivate(this))
{
    setReply(reply);

    //qDebug() << "--Send reply:" << reply->url();
}

Reply::~Reply()
{
    //FIXME maybee it's never been used
    if (auto networkReply = d_func()->networkReply.data())
        networkReply->deleteLater();
}

QNetworkReply *Reply::networkReply() const
{
    return d_func()->networkReply.data();
}

QVariant Reply::response() const
{
    return d_func()->response;
}

QVariant Reply::error() const
{
    return d_func()->error;
}

QVariant Reply::result() const
{
    Q_D(const Reply);
    return d->result;
}

void Reply::setReply(QNetworkReply *reply)
{
    Q_D(Reply);
    if (!d->networkReply.isNull())
        d->networkReply.data()->deleteLater();
    d->networkReply = reply;
    setParent(reply);

    connect(reply, SIGNAL(finished()), SLOT(_q_reply_finished()));
    connect(reply, SIGNAL(error(QNetworkReply::NetworkError)), SLOT(_q_network_reply_error(QNetworkReply::NetworkError)));
}

void Reply::setHandlerImpl(Reply::ResultHandlerBase *handler)
{
    Q_D(Reply);
    d->resultHandler.reset(handler);
}

void ReplyPrivate::_q_reply_finished()
{
    Q_Q(Reply);
    auto reply = static_cast<QNetworkReply*>(q->sender());
    QVariantMap map = JSON::parse(reply->readAll()).toMap();
    //TODO error and captcha handler

    //qDebug() << "--Reply finished: " << reply->url().encodedPath();

    response = map.value("response");
    if (!response.isNull()) {
        if (resultHandler)
            result = resultHandler->handle(response);
        emit q->resultReady(response);
        return;
    } else {
        error = map.value("error");
        int errorCode = error.toMap().value("error_code").toInt();
        if (errorCode) {
            emit q->error(errorCode);
            emit q->resultReady(response); //emit blank response to mark reply as finished
            return;
        }
    }
    if (!map.isEmpty()) {
        response = map;
        emit q->resultReady(response);
    }
}

void ReplyPrivate::_q_network_reply_error(QNetworkReply::NetworkError code)
{
    Q_Q(Reply);
    error = code;
    emit q->error(Client::ErrorNetworkReply);
    emit q->resultReady(response);
}


QVariant MessageListHandler::operator()(const QVariant &response)
{
    MessageList msgList;
    auto list = response.toList();
    if (list.count()) {
        list.removeFirst(); //remove count
        msgList = Message::fromVariantList(list, clientId);
    }
    return QVariant::fromValue(msgList);
}

} // namespace Vreen

#include "moc_reply.cpp"

