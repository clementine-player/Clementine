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
#include <QDebug>
#include "contact_p.h"
#include "message.h"
#include "roster_p.h"
#include "groupmanager_p.h"

namespace Vreen {

Contact::Contact(int id, Client *client) :
    QObject(client),
    d_ptr(new ContactPrivate(this, id, client))
{
}

Contact::Contact(ContactPrivate *data) :
    QObject(data->client),
    d_ptr(data)
{
}

Contact::~Contact()
{
}

Contact::Type Contact::type()
{
    return d_func()->type;
}

int Contact::id() const
{
    return d_func()->id;
}

Client *Contact::client() const
{
	return d_func()->client;
}

QString Contact::photoSource(Contact::PhotoSize size) const
{
    Q_D(const Contact);
    return d->sources.value(size);
}

void Contact::setPhotoSource(const QString &source, Contact::PhotoSize size)
{
    d_func()->sources[size] = source;
    emit photoSourceChanged(source, size);
}

void Contact::fill(Contact *contact, const QVariantMap &data)
{
    auto it = data.constBegin();
    for (; it != data.constEnd(); it++) {
        QByteArray property = "_q_" + it.key().toLatin1();
        contact->setProperty(property.data(), it.value());
    }
}

Buddy::Buddy(int id, Client *client) :
    Contact(new BuddyPrivate(this, id, client))
{
    if (id < 0)
        qWarning() << "Buddy's id must be positive";
}

QString Buddy::firstName() const
{
    return d_func()->firstName;
}

void Buddy::setFirstName(const QString &firstName)
{
    Q_D(Buddy);
    if (d->firstName != firstName) {
        d->firstName = firstName;
        emit firstNameChanged(firstName);
        emit nameChanged(name());
    }
}

QString Buddy::lastName() const
{
    return d_func()->lastName;
}

void Buddy::setLastName(const QString &lastName)
{
    Q_D(Buddy);
    if (d->lastName != lastName) {
        d->lastName = lastName;
        emit lastNameChanged(lastName);
        emit nameChanged(name());
    }
}

bool Buddy::isOnline() const
{
    return d_func()->status != Offline;
}

void Buddy::setOnline(bool set)
{
    setStatus(set ? Online : Offline);
    emit onlineChanged(set);
}

QString Buddy::name() const
{
    Q_D(const Buddy);
    if (d->firstName.isEmpty() && d->lastName.isEmpty())
        return tr("id%1").arg(id());
    else if (d->lastName.isEmpty())
        return d->firstName;
    else if (d->firstName.isEmpty())
        return d->lastName;
    else
        return d->firstName + ' ' + d->lastName;

}

QStringList Buddy::tags() const
{
    Q_D(const Buddy);
    QStringList tags;
    foreach (auto data, d->tagIdList) {
        int id = data.toInt();
        tags.append(d->client->roster()->tags().value(id, tr("Unknown tag id %1").arg(id)));
    }
    return tags;
}

QString Buddy::activity() const
{
    return d_func()->activity;
}

Buddy::Status Buddy::status() const
{
    return d_func()->status;
}

void Buddy::setStatus(Buddy::Status status)
{
    Q_D(Buddy);
    //hack for delayed replies recieve
    if (!d->client->isOnline())
        status = Offline;

    if (d->status != status) {
        d_func()->status = status;
        emit statusChanged(status);
    }
}

bool Buddy::isFriend() const
{
    return d_func()->isFriend;
}

void Buddy::setIsFriend(bool set)
{
    Q_D(Buddy);
    if (d->isFriend != set) {
        d->isFriend = set;
        emit isFriendChanged(set);
    }
}

/*!
 * \brief Buddy::update
 * \param fields - some fields need to update.
 * \note This request will be called immediately.
 * \sa update
 * api reference \link http://vk.com/developers.php?oid=-1&p=users.get
 */
void Buddy::update(const QStringList &fields)
{
    IdList ids;
    ids.append(id());
    d_func()->client->roster()->update(ids, fields);
}

/*!
 * \brief Buddy::update
 * Add contact to update queue and it will be updated as soon as posible in near future.
 * Use this method if you know that it takes more than one update
 * \sa update(fields)
 */
void Buddy::update()
{
    Q_D(Buddy);
    d->client->roster()->d_func()->appendToUpdaterQueue(this);
}

SendMessageReply *Buddy::sendMessage(const QString &body, const QString &subject)
{
    Q_D(Buddy);
    Message message(d->client);
    message.setBody(body);
    message.setSubject(subject);
	message.setToId(id());
	return d->client->sendMessage(message);
}

Reply *Buddy::addToFriends(const QString &reason)
{
	Q_D(Buddy);
	QVariantMap args;
	args.insert("uid", d->id);
	args.insert("text", reason);
	auto reply = d->client->request("friends.add", args);
	connect(reply, SIGNAL(resultReady(QVariant)), this, SLOT(_q_friends_add_finished(QVariant)));
	return reply;
}

Reply *Buddy::removeFromFriends()
{
	Q_D(Buddy);
	QVariantMap args;
	args.insert("uid", d->id);
	auto reply = d->client->request("friends.delete", args);
	connect(reply, SIGNAL(resultReady(QVariant)), this, SLOT(_q_friends_delete_finished(QVariant)));
	return reply;
}

Group::Group(int id, Client *client) :
	Contact(new GroupPrivate(this, id, client))
{
    Q_D(Group);
    if (id > 0)
        qWarning() << "Group's id must be negative";
    d->type = GroupType;
}

QString Group::name() const
{
    Q_D(const Group);
    if (!d->name.isEmpty())
        return d->name;
    return tr("group-%1").arg(id());
}

void Group::setName(const QString &name)
{
    d_func()->name = name;
    emit nameChanged(name);
}

void Group::update()
{
    Q_D(Group);
    d->client->groupManager()->d_func()->appendToUpdaterQueue(this);
}

void BuddyPrivate::_q_friends_add_finished(const QVariant &response)
{
	Q_Q(Buddy);
	int answer = response.toInt();
	switch (answer) {
	case 1:
		//TODO
		break;
	case 2:
		q->setIsFriend(true);
	case 4:
		//TODO
		break;
	default:
		break;
	}
}

void BuddyPrivate::_q_friends_delete_finished(const QVariant &response)
{
	Q_Q(Buddy);
	int answer = response.toInt();
	switch (answer) {
	case 1:
		q->setIsFriend(false);
		break;
	case 2:
		//TODO
	default:
		break;
	}
}

} // namespace Vreen

#include "moc_contact.cpp"

