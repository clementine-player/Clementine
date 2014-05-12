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
#ifndef VREEN_FRIENDREQUEST_H
#define VREEN_FRIENDREQUEST_H

#include "vk_global.h"
#include <QVariant>
#include <QSharedDataPointer>

namespace Vreen {

class FriendRequestData;

class VK_SHARED_EXPORT FriendRequest
{
public:
    explicit FriendRequest(int uid = 0);
    FriendRequest(const FriendRequest &);
    FriendRequest &operator=(const FriendRequest &);
    ~FriendRequest();
    int uid() const;
    void setUid(int uid);
    QString message() const;
    void setMessage(const QString &message);
    IdList mutualFriends() const;
    void setMutualFriends(const IdList &mutualFriends);
private:
    QSharedDataPointer<FriendRequestData> data;
};
typedef QList<FriendRequest> FriendRequestList;

} // namespace Vreen

Q_DECLARE_METATYPE(Vreen::FriendRequest)
Q_DECLARE_METATYPE(Vreen::FriendRequestList)

#endif // VREEN_FRIENDREQUEST_H
