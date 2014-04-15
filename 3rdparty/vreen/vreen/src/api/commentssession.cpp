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
#include "commentssession.h"
#include "contact.h"
#include "client.h"
#include "reply.h"

namespace Vreen {

class CommentSession;
class CommentSessionPrivate
{
    Q_DECLARE_PUBLIC(CommentSession)
public:
    CommentSessionPrivate(CommentSession *q, Contact *contact) :
        q_ptr(q), contact(contact), postId(0),
        sort(Qt::AscendingOrder),
        needLikes(true),
        previewLenght(0)

    {}
    CommentSession *q_ptr;
    Contact *contact;
    int postId;
    Qt::SortOrder sort;
    bool needLikes;
    int previewLenght;

    void _q_comments_received(const QVariant &response)
    {
        auto list = response.toList();
        if (!list.isEmpty()) {
            list.takeFirst();
            foreach (auto item, list)
                emit q_func()->commentAdded(item.toMap());
        }
    }
};


/*!
 * \brief CommentsSession::CommentsSession
 * \param client
 */
CommentSession::CommentSession(Contact *contact) :
    QObject(contact),
    d_ptr(new CommentSessionPrivate(this, contact))
{
}

void CommentSession::setPostId(int postId)
{
    Q_D(CommentSession);
    d->postId = postId;
}

int CommentSession::postId() const
{
    return d_func()->postId;
}

CommentSession::~CommentSession()
{
}

Reply *CommentSession::getComments(int offset, int count)
{
    Q_D(CommentSession);
    QVariantMap args;
    args.insert("owner_id", (d->contact->type() == Contact::GroupType ? -1 : 1) * d->contact->id());
    args.insert("post_id", d->postId);
    args.insert("offset", offset);
    args.insert("count", count);
    args.insert("need_likes", d->needLikes);
    args.insert("preview_lenght", d->previewLenght);
    args.insert("sort", d->sort == Qt::AscendingOrder ? "asc" : "desc");
    auto reply = d->contact->client()->request("wall.getComments", args);
    connect(reply, SIGNAL(resultReady(QVariant)), SLOT(_q_comments_received(QVariant)));
    return reply;
}

} // namespace Vreen

#include "moc_commentssession.cpp"

