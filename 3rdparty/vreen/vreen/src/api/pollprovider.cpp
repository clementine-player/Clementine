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
#include <QDateTime>
#include "client.h"
#include "pollprovider.h"
#include "pollitem.h"

namespace Vreen {

class PollProvider;
class PollProviderPrivate
{
    Q_DECLARE_PUBLIC(PollProvider)
public:
    PollProviderPrivate(PollProvider *q, Client *client) : q_ptr(q), client(client) {}
    PollProvider *q_ptr;
    Client *client;
    
    static QVariant handlePoll(const QVariant& response) {
        auto map = response.toMap();
        PollItem poll;
        poll.setOwnerId(map.value("owner_id").toInt());
        poll.setPollId(map.value("poll_id").toInt());
        poll.setCreated(map.value("created").toDateTime());
        poll.setQuestion(map.value("question").toString());
        poll.setVotes(map.value("votes").toInt());
        poll.setAnswerId(map.value("answer_id").toInt());
        
        PollItem::AnswerList answerList;
        auto answers = map.value("answers").toList();
        foreach (auto item, answers) {
            auto map = item.toMap();
            PollItem::Answer answer;
            answer.id = map.value("id").toInt();
            answer.text = map.value("text").toString();
            answer.votes = map.value("votes").toInt();
            answer.rate = map.value("rate").toReal();
            answerList.append(answer);
        }
        poll.setAnswers(answerList);
        
        return QVariant::fromValue(poll);
    }
    
};

PollProvider::PollProvider(Client *client) :
    QObject(client),
    d_ptr(new PollProviderPrivate(this, client))
{
}

PollProvider::~PollProvider()
{
}

PollItemReply *PollProvider::getPollById(int ownerId, int pollId)
{
    Q_D(PollProvider);
    QVariantMap args;
    args.insert("owner_id", ownerId);
    args.insert("poll_id", pollId);
    
    auto reply = d->client->request<PollItemReply>("polls.getById", args, PollProviderPrivate::handlePoll);
    return reply;
}

Reply *PollProvider::addVote(int pollId, int answerId, int ownerId)
{
    Q_D(PollProvider);
    QVariantMap args;
    args.insert("poll_id", pollId);
    args.insert("answer_id", answerId);
    if (ownerId)
        args.insert("owner_id", ownerId);
    
    auto reply = d->client->request("polls.addVote", args);
    return reply;
}

Reply *PollProvider::deleteVote(int pollId, int answerId, int ownerId)
{
    Q_D(PollProvider);
    QVariantMap args;
    args.insert("poll_id", pollId);
    args.insert("answer_id", answerId);
    if (ownerId)
        args.insert("owner_id", ownerId);
    
    auto reply = d->client->request("polls.deleteVote", args);
    return reply;
}

} // namespace Vreen
