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
#ifndef POLLITEM_H
#define POLLITEM_H

#include <QSharedDataPointer>
#include <QVariant>
#include "attachment.h"

namespace Vreen {

class PollItemData;

class VK_SHARED_EXPORT PollItem
{
public:
    struct Answer {
        int id;
        QString text;
        int votes;
        qreal rate;
    };
    typedef QList<Answer> AnswerList;

    PollItem(int pollId = 0);
    PollItem(const PollItem &);
    PollItem &operator=(const PollItem &);
    ~PollItem();
    
    int ownerId() const;
    void setOwnerId(int ownerId);
    int pollId() const;
    void setPollId(int pollId);
    QDateTime created() const;
    void setCreated(const QDateTime &created);
    QString question() const;
    void setQuestion(const QString &question);
    int votes() const;
    void setVotes(int votes);
    int answerId() const;
    void setAnswerId(int answerId);
    AnswerList answers() const;
    void setAnswers(const AnswerList &answers);
private:
    QSharedDataPointer<PollItemData> data;
};

template<>
PollItem Attachment::to(const Attachment &data);

} //namespace Vreen

Q_DECLARE_METATYPE(Vreen::PollItem)

#endif // POLLITEM_H
