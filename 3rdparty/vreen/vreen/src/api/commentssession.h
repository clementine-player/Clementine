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
#ifndef VK_COMMENTSSESSION_H
#define VK_COMMENTSSESSION_H

#include <QObject>
#include <QVariantMap>
#include "vk_global.h"

namespace Vreen {

class Reply;
class Contact;
class CommentSessionPrivate;

class VK_SHARED_EXPORT CommentSession : public QObject
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(CommentSession)
public:
    CommentSession(Vreen::Contact *contact);
    virtual ~CommentSession();
    void setPostId(int id);
    int postId() const;
public slots:
    Reply *getComments(int offset = 0, int count = 100);
signals:
    void commentAdded(const QVariantMap &item);
    void commentDeleted(int commentId);
private:
    QScopedPointer<CommentSessionPrivate> d_ptr;

    Q_PRIVATE_SLOT(d_func(), void _q_comments_received(QVariant))
};

typedef QList<QVariantMap> CommentList;

} // namespace Vreen

#endif // VK_COMMENTSSESSION_H

