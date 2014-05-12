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
#include "pollitem.h"
#include <QSharedData>
#include <QDateTime>

namespace Vreen {

template<>
PollItem Attachment::to(const Attachment &data)
{
    PollItem item;
    item.setPollId(data.property("poll_id").toInt());
    item.setQuestion(data.property("question").toString());
    return item;
}

class PollItemData : public QSharedData {
public:
    PollItemData() :
        ownerId(0), pollId(0), votes(0), answerId(0) {}
    PollItemData(const PollItemData &o) : QSharedData(o),
        ownerId(o.ownerId), pollId(o.pollId), created(o.created),
        question(o.question), votes(o.votes), answerId(o.answerId),
        answers(o.answers)
    {}

    int ownerId;
    int pollId;
    QDateTime created;
    QString question;
    int votes;
    int answerId;
    PollItem::AnswerList answers;
};

PollItem::PollItem(int pollId) : data(new PollItemData)
{
    data->pollId = pollId;
}

PollItem::PollItem(const PollItem &rhs) : data(rhs.data)
{
}

PollItem &PollItem::operator=(const PollItem &rhs)
{
    if (this != &rhs)
        data.operator=(rhs.data);
    return *this;
}

PollItem::~PollItem()
{
}

int PollItem::ownerId() const
{
    return data->ownerId;
}

void PollItem::setOwnerId(int ownerId)
{
    data->ownerId = ownerId;
}

int PollItem::pollId() const
{
    return data->pollId;
}

void PollItem::setPollId(int pollId)
{
    data->pollId = pollId;
}

QDateTime PollItem::created() const
{
    return data->created;
}

void PollItem::setCreated(const QDateTime &created)
{
    data->created = created;
}

QString PollItem::question() const
{
    return data->question;
}

void PollItem::setQuestion(const QString &question)
{
    data->question = question;
}

int PollItem::votes() const
{
    return data->votes;
}

void PollItem::setVotes(int votes)
{
    data->votes = votes;
}

int PollItem::answerId() const
{
    return data->answerId;
}

void PollItem::setAnswerId(int answerId)
{
    data->answerId = answerId;
}

PollItem::AnswerList PollItem::answers() const
{
    return data->answers;
}

void PollItem::setAnswers(const PollItem::AnswerList &answers)
{
    data->answers = answers;
}

} //namespace Vreen
