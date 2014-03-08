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
#include "messagemodel.h"
#include "contact.h"
#include "longpoll.h"
#include "utils.h"
#include "utils_p.h"
#include "client.h"
#include <QCoreApplication>
#include <QDateTime>
#include <QPointer>

namespace Vreen {

class MessageListModel;
class MessageListModelPrivate
{
    Q_DECLARE_PUBLIC(MessageListModel)
public:
    MessageListModelPrivate(MessageListModel *q) : q_ptr(q),
        messageComparator(Qt::DescendingOrder) {}
    MessageListModel *q_ptr;
    QPointer<Client> client;

    MessageList messageList;
    IdComparator<Message> messageComparator;
};


MessageListModel::MessageListModel(QObject *parent) :
    QAbstractListModel(parent),
    d_ptr(new MessageListModelPrivate(this))
{
    auto roles = roleNames();
    roles[SubjectRole] = "subject";
    roles[BodyRole] = "body";
    roles[FromRole] = "from";
    roles[ToRole] = "to";
    roles[ReadStateRole] = "unread";
    roles[DirectionRole] = "incoming";
    roles[DateRole] = "date";
    roles[IdRole] = "mid";
    roles[AttachmentRole] = "attachments";
    roles[ChatIdRole] = "chatId";
    setRoleNames(roles);
}

MessageListModel::~MessageListModel()
{

}

int MessageListModel::count() const
{
    return d_func()->messageList.count();
}

Message MessageListModel::at(int index) const
{
    return d_func()->messageList.at(index);
}

int MessageListModel::findMessage(int id)
{
    Q_D(MessageListModel);
    for (int index = 0; index != d->messageList.count(); index++)
        if (d->messageList.at(index).id() == id)
            return index;
    return -1;
}


QVariant MessageListModel::data(const QModelIndex &index, int role) const
{
    Q_D(const MessageListModel);

    int row = index.row();
    auto message = d->messageList.at(row);
    switch (role) {
    case SubjectRole:
        return message.subject();
        break;
    case BodyRole:
        return fromHtmlEntities(message.body());
    case FromRole:
        return QVariant::fromValue(d->client->contact(message.fromId()));
    case ToRole:
        return QVariant::fromValue(d->client->contact(message.toId()));
    case ReadStateRole:
        return message.isUnread();
    case DirectionRole:
        return message.isIncoming();
    case DateRole:
        return message.date();
    case IdRole:
        return message.id();
    case AttachmentRole:
        return Attachment::toVariantMap(message.attachments());
    case ChatIdRole:
        return message.chatId();
    default:
        break;
    }
    return QVariant::Invalid;
}

int MessageListModel::rowCount(const QModelIndex &) const
{
    return count();
}

void MessageListModel::setSortOrder(Qt::SortOrder order)
{
    Q_D(MessageListModel);
    if (d->messageComparator.sortOrder != order) {
        sort(0, order);
        emit sortOrderChanged(order);
    }
}

Qt::SortOrder MessageListModel::sortOrder() const
{
    return d_func()->messageComparator.sortOrder;
}

void MessageListModel::setClient(Client *client)
{
    Q_D(MessageListModel);
    if (d->client != client) {
        d->client = client;
        emit clientChanged(client);

        if (d->client) {
            auto longPoll = d->client.data()->longPoll();
            connect(longPoll, SIGNAL(messageFlagsReplaced(int, int, int)), SLOT(replaceMessageFlags(int, int, int)));
            connect(longPoll, SIGNAL(messageFlagsReseted(int,int,int)), SLOT(resetMessageFlags(int,int)));
        }
        if (client)
            client->disconnect(this);
    }
}

Client *MessageListModel::client() const
{
    return d_func()->client.data();
}

void MessageListModel::addMessage(const Message &message)
{
    Q_D(MessageListModel);
    int index = findMessage(message.id());
    if (index != -1)
        return;

    index = lowerBound(d->messageList, message, d->messageComparator);
    doInsertMessage(index, message);
}

void MessageListModel::removeMessage(const Message &message)
{
    Q_D(MessageListModel);
    int index = d->messageList.indexOf(message);
    if (index == -1)
        return;
    doRemoveMessage(index);
}

void MessageListModel::removeMessage(int id)
{
    int index = findMessage(id);
    if (index != -1)
        doRemoveMessage(index);
}

void MessageListModel::setMessages(const MessageList &messages)
{
    clear();
    foreach (auto message, messages) {
        addMessage(message);
        qApp->processEvents(QEventLoop::ExcludeUserInputEvents);
    }
}

void MessageListModel::clear()
{
    Q_D(MessageListModel);
    beginRemoveRows(QModelIndex(), 0, d->messageList.count());
    d->messageList.clear();
    endRemoveRows();
}

void MessageListModel::doReplaceMessage(int i, const Message &message)
{
    auto index = createIndex(i, 0);
    d_func()->messageList[i] = message;
    emit dataChanged(index, index);
}

void MessageListModel::doInsertMessage(int index, const Message &message)
{
    Q_D(MessageListModel);
    beginInsertRows(QModelIndex(), index, index);
    d->messageList.insert(index, message);
    endInsertRows();
}

void MessageListModel::doRemoveMessage(int index)
{
    Q_D(MessageListModel);
    beginRemoveRows(QModelIndex(), index, index);
    d->messageList.removeAt(index);
    endRemoveRows();
}

void MessageListModel::moveMessage(int sourceIndex, int destinationIndex)
{
    Q_D(MessageListModel);
    beginMoveRows(QModelIndex(), sourceIndex, sourceIndex,
                  QModelIndex(), destinationIndex);
    d->messageList.move(sourceIndex, destinationIndex);
    endMoveRows();
}

void MessageListModel::sort(int column, Qt::SortOrder order)
{
    Q_D(MessageListModel);
    Q_UNUSED(column);
    d->messageComparator.sortOrder = order;
    qStableSort(d->messageList.begin(), d->messageList.end(), d->messageComparator);
    emit dataChanged(createIndex(0, 0), createIndex(d->messageList.count(), 0));
}

void MessageListModel::replaceMessageFlags(int id, int mask, int userId)
{
    Q_UNUSED(userId);
    int index = findMessage(id);
    if (index == -1)
        return;

    auto message = at(index);
    Message::Flags flags = message.flags();
    flags |= static_cast<Message::Flags>(mask);
    message.setFlags(flags);
    doReplaceMessage(index, message);
}

void MessageListModel::resetMessageFlags(int id, int mask, int userId)
{
    Q_UNUSED(userId);
    int index = findMessage(id);
    if (index == -1)
        return;

    auto message = at(index);
    auto flags = message.flags();
    flags &= ~mask;
    message.setFlags(flags);
    doReplaceMessage(index, message);
}

} //namespace Vreen

#include "moc_messagemodel.cpp"

