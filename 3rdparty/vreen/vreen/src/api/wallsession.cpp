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
#include "wallsession.h"
#include "contact.h"
#include "wallpost.h"
#include "utils.h"
#include "client_p.h"
#include <QNetworkReply>

namespace Vreen {

static const char *filters[] = {
    "owner",
    "others",
    "all"
};

class WallSession;
class WallSessionPrivate
{
    Q_DECLARE_PUBLIC(WallSession)
public:
    WallSessionPrivate(WallSession *q, Contact *contact) : q_ptr(q), contact(contact) {}
    WallSession *q_ptr;
    Contact *contact;

    void _q_posts_received(const QVariant &response)
    {
        auto list = response.toMap().value("wall").toList();
        if (!list.isEmpty()) {
            list.takeFirst();
            foreach (auto item, list) {
                auto post = WallPost::fromData(item);
                emit q_func()->postAdded(post);
            }
        }
    }

    void _q_like_added(const QVariant &response)
    {
        //FIXME error handler
        auto reply = sender_cast<Reply*>(q_func()->sender());
        auto url = reply->networkReply()->url();
        int id = url.queryItemValue("post_id").toInt();
        int retweet = url.queryItemValue("repost").toInt();
        auto map = response.toMap();

        emit q_func()->postLikeAdded(id,
                                     map.value("likes").toInt(),
                                     map.value("reposts").toInt(),
                                     retweet);
    }

    void _q_like_deleted(const QVariant &response)
    {
        auto reply = sender_cast<Reply*>(q_func()->sender());
        auto url = reply->networkReply()->url();
        int id = url.queryItemValue("post_id").toInt();
        int likesCount = response.toMap().value("likes").toInt();

        emit q_func()->postLikeDeleted(id, likesCount);
    }
};

WallSession::WallSession(Contact *contact) :
    QObject(contact),
    d_ptr(new WallSessionPrivate(this, contact))
{

}

WallSession::~WallSession()
{

}

/*!
 * \brief WallSession::getPosts. A wrapper on API method wall.get, \link http://vk.com/developers.php?oid=-1&p=wall.get
 * \param filter determine what types of messages on the wall to get. The following parameter values​​:
 * Owner - messages on the wall by its owner
 * Others - posts on the wall, not on its owner
 * All - all the messages on the wall
 * \param count
 * \param offset
 * \param extended flag: true - three arrays will be returned to wall, profiles, and groups. By default, additional fields will not be returned.
 * \return
 */
Reply *WallSession::getPosts(WallSession::Filter filter, quint8 count, int offset, bool extended)
{
    Q_D(WallSession);
    QVariantMap args;
    args.insert("owner_id", QString::number(d->contact->id()));
    args.insert("offset", offset);
    args.insert("count", count);
    args.insert("filter", filters[filter-1]);
    args.insert("extended", extended);
    auto reply = d->contact->client()->request("wall.get", args);
    connect(reply, SIGNAL(resultReady(QVariant)), SLOT(_q_posts_received(QVariant)));

    return reply;
}

Contact *WallSession::contact() const
{
    return d_func()->contact;
}

/*!
 * \brief Vreen::WallSession::like A wrapper on API method wall.addLike \link http://vk.com/developers.php?oid=-1&p=wall.addLike
 * \param postId
 * \param retweet
 * \return
 */
Vreen::Reply *Vreen::WallSession::addLike(int postId, bool retweet, const QString &message)
{
    Q_D(WallSession);
    auto reply = d->contact->client()->addLike(d->contact->id(),
                                               postId,
                                               retweet,
                                               message);
    connect(reply, SIGNAL(resultReady(QVariant)), SLOT(_q_like_added(QVariant)));
    return reply;
}

/*!
 * \brief WallSession::deleteLike a wrapper on API method wall.deleteLike \link http://vk.com/developers.php?oid=-1&p=wall.deleteLike
 * \param postId
 * \return
 */
Reply *WallSession::deleteLike(int postId)
{
    Q_D(WallSession);
    auto reply = d->contact->client()->deleteLike(d->contact->id(), postId);
    connect(reply, SIGNAL(resultReady(QVariant)), SLOT(_q_like_deleted(QVariant)));
    return reply;
}

} // namespace Vreen

#include "moc_wallsession.cpp"

