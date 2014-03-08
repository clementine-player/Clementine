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
** along with this program.  If not, see http://www.gnu.org/licees/.
** $VREEN_END_LICENSE$
**
****************************************************************************/
#ifndef VREEN_POLLPROVIDER_H
#define VREEN_POLLPROVIDER_H

#include "pollitem.h"
#include "reply.h"

namespace Vreen {
    
class Client;
typedef ReplyBase<Vreen::PollItem> PollItemReply;

class PollProviderPrivate;
class VK_SHARED_EXPORT PollProvider : public QObject
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(PollProvider)
public:
    explicit PollProvider(Client *client);
    ~PollProvider();
    
    PollItemReply *getPollById(int ownerId, int pollId);
    Reply *addVote(int pollId, int answerId, int ownerId = 0);
    Reply *deleteVote(int pollId, int answerId, int ownerId = 0);
protected:
    QScopedPointer<PollProviderPrivate> d_ptr;
};

} // namespace Vreen

#endif // VREEN_POLLPROVIDER_H
