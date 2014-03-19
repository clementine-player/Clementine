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
#include "friendrequest.h"
#include <QSharedData>

namespace Vreen {

class FriendRequestData : public QSharedData {
public:
    FriendRequestData(int uid) : QSharedData(),
        uid(uid)
    {}
    FriendRequestData(const FriendRequestData &o) : QSharedData(o),
        uid(o.uid),
        message(o.message),
        mutual(o.mutual)
    {}

    int uid;
    QString message;
    IdList mutual;
};

FriendRequest::FriendRequest(int uid) : data(new FriendRequestData(uid))
{
}

FriendRequest::FriendRequest(const FriendRequest &rhs) : data(rhs.data)
{
}

FriendRequest &FriendRequest::operator=(const FriendRequest &rhs)
{
    if (this != &rhs)
        data.operator=(rhs.data);
    return *this;
}

FriendRequest::~FriendRequest()
{
}

int FriendRequest::uid() const
{
    return data->uid;
}

void FriendRequest::setUid(int uid)
{
    data->uid = uid;
}

IdList FriendRequest::mutualFriends() const
{
    return data->mutual;
}

void FriendRequest::setMutualFriends(const IdList &mutual)
{
    data->mutual = mutual;
}

QString FriendRequest::message() const
{
    return data->message;
}

void FriendRequest::setMessage(const QString &message)
{
    data->message = message;
}

} // namespace Vreen
