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
#ifndef VK_CLIENT_H
#define VK_CLIENT_H

#include "vk_global.h"
#include "reply.h"
#include <QScopedPointer>
#include <QObject>
#include <QVariantMap>

class QUrl;
namespace Vreen {

class Message;
class Connection;
class ClientPrivate;
class Roster;
class GroupManager;
class LongPoll;
class Contact;
class Buddy;

typedef ReplyBase<int> SendMessageReply;

class VK_SHARED_EXPORT Client : public QObject
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(Client)

    Q_PROPERTY(QString password READ password WRITE setPassword NOTIFY passwordChanged DESIGNABLE true)
    Q_PROPERTY(QString login READ login WRITE setLogin NOTIFY loginChanged DESIGNABLE true)
    Q_PROPERTY(bool online READ isOnline NOTIFY onlineStateChanged DESIGNABLE true)
    Q_PROPERTY(State connectionState READ connectionState NOTIFY connectionStateChanged DESIGNABLE true)
    Q_PROPERTY(Vreen::Roster* roster READ roster NOTIFY rosterChanged DESIGNABLE true)
    Q_PROPERTY(Vreen::GroupManager* groupManager READ groupManager NOTIFY groupManagerChanged DESIGNABLE true)
    Q_PROPERTY(Vreen::LongPoll* longPoll READ longPoll NOTIFY longPollChanged DESIGNABLE true)
    Q_PROPERTY(Vreen::Buddy* me READ me NOTIFY meChanged DESIGNABLE true)
    Q_PROPERTY(Vreen::Connection* connection READ connection WRITE setConnection NOTIFY connectionChanged DESIGNABLE true)
    Q_PROPERTY(QString activity READ activity WRITE setActivity NOTIFY activityChanged DESIGNABLE true)
    Q_PROPERTY(bool invisible READ isInvisible WRITE setInvisible NOTIFY invisibleChanged)
    Q_PROPERTY(bool trackMessages READ trackMessages WRITE setTrackMessages NOTIFY trackMessagesChanged)

    Q_ENUMS(State)
    Q_ENUMS(Error)
public:

    enum State {
        StateOffline,
        StateConnecting,
        StateOnline,
        StateInvalid
    };
    enum Error {
        ErrorUnknown                    = 1,
        ErrorApplicationDisabled        = 2,
        ErrorIncorrectSignature         = 4,
        ErrorAuthorizationFailed        = 5,
        ErrorToManyRequests             = 6,
        ErrorPermissionDenied           = 7,
        ErrorCaptchaNeeded              = 14,
        ErrorMissingOrInvalidParameter  = 100,
        ErrorNetworkReply               = 4096
    };

    explicit Client(QObject *parent = 0);
    explicit Client(const QString &login, const QString &password, QObject *parent = 0);
    virtual ~Client();
    QString password() const;
    void setPassword(const QString &password);
    QString login() const;
    void setLogin(const QString &login);
    State connectionState() const;
    bool isOnline() const;
    QString activity() const;
    void setActivity(const QString &activity);
    bool isInvisible() const;
    void setInvisible(bool set);
    bool trackMessages() const;
    void setTrackMessages(bool set);

    Connection *connection() const;
    Connection *connection();
    void setConnection(Connection *connection);
    Roster *roster();
    Roster *roster() const;
    LongPoll *longPoll();
    LongPoll *longPoll() const;
    GroupManager *groupManager();
    GroupManager *groupManager() const;

    Reply *request(const QUrl &);
    Reply *request(const QString &method, const QVariantMap &args = QVariantMap());
    template <typename ReplyImpl, typename Handler>
    ReplyImpl *request(const QString &method, const QVariantMap &args, const Handler &handler);
    SendMessageReply *sendMessage(const Message &message);
    Reply *getMessage(int mid, int previewLength = 0);
    Reply *addLike(int ownerId, int postId, bool retweet = false, const QString &message = QString()); //TODO move method
    Reply *deleteLike(int ownerId, int postId); //TODO move method

    Q_INVOKABLE Buddy *me();
    Buddy *me() const;
    Q_INVOKABLE Contact *contact(int id) const;
    int id() const;
public slots:
    void connectToHost();
    void connectToHost(const QString &login, const QString &password);
    void disconnectFromHost();
signals:
    void loginChanged(const QString &login);
    void passwordChanged(const QString &password);
    void connectionChanged(Vreen::Connection *connection);
    void connectionStateChanged(Vreen::Client::State state);
    void replyCreated(Vreen::Reply*);
    void error(Vreen::Client::Error error);
    void onlineStateChanged(bool online);
    void rosterChanged(Vreen::Roster*);
    void groupManagerChanged(Vreen::GroupManager*);
    void longPollChanged(Vreen::LongPoll*);
    void meChanged(Vreen::Buddy *me);
    void activityChanged(const QString &activity);
    void invisibleChanged(bool set);
    void trackMessagesChanged(bool set);
protected:
    Reply *setStatus(const QString &text, int aid = 0);
    QScopedPointer<ClientPrivate> d_ptr;
private:
    void processReply(Reply *reply);
    QNetworkReply *requestHelper(const QString &method, const QVariantMap &args);

    Q_PRIVATE_SLOT(d_func(), void _q_connection_state_changed(Vreen::Client::State))
    Q_PRIVATE_SLOT(d_func(), void _q_error_received(int))
    Q_PRIVATE_SLOT(d_func(), void _q_reply_finished(const QVariant &))
    Q_PRIVATE_SLOT(d_func(), void _q_activity_update_finished(const QVariant &))
    Q_PRIVATE_SLOT(d_func(), void _q_update_online())
    Q_PRIVATE_SLOT(d_func(), void _q_network_manager_error(int))
};

template<typename ReplyImpl, typename Handler>
ReplyImpl *Client::request(const QString &method, const QVariantMap &args, const Handler &handler)
{
    ReplyImpl *reply = new ReplyImpl(handler, requestHelper(method, args));
    processReply(reply);
    return reply;
}

} // namespace Vreen

Q_DECLARE_METATYPE(Vreen::Client*)

#endif // VK_CLIENT_H

