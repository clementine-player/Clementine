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
#ifndef VK_WALLSESSION_H
#define VK_WALLSESSION_H

#include "client.h"
#include "wallpost.h"

namespace Vreen {

class Reply;
class WallSessionPrivate;
class VK_SHARED_EXPORT WallSession : public QObject
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(WallSession)
    Q_ENUMS(Filter)
public:
    enum Filter {
        Owner   = 0x1,
        Others   = 0x2,
        All     = Owner | Others
    };

    explicit WallSession(Contact *contact);
    Contact *contact() const;
    virtual ~WallSession();

public slots:
    Reply *getPosts(Filter filter = All, quint8 count = 16, int offset = 0, bool extended = false);
    Reply *addLike(int postId, bool retweet = false, const QString &message = QString());
    Reply *deleteLike(int postId);
signals:
    void postAdded(const Vreen::WallPost &post);
    void postDeleted(int postId);
    void postLikeAdded(int postId, int likesCount, int repostsCount, bool isRetweeted);
    void postLikeDeleted(int postId, int likesCount);
protected:
    QScopedPointer<WallSessionPrivate> d_ptr;

    Q_PRIVATE_SLOT(d_func(), void _q_posts_received(QVariant))
    Q_PRIVATE_SLOT(d_func(), void _q_like_added(QVariant))
    Q_PRIVATE_SLOT(d_func(), void _q_like_deleted(QVariant))
};

} // namespace Vreen

#endif // VK_WALLSESSION_H

