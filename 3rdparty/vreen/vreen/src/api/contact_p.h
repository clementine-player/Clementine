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
#ifndef USER_P_H
#define USER_P_H
#include "contact.h"
#include "client.h"
#include <QStringList>
#include <QVector>

namespace Vreen {

class Contact;
class ContactPrivate
{
    Q_DECLARE_PUBLIC(Contact)
public:
    ContactPrivate(Contact *q, int id, Client *client) : q_ptr(q),
        client(client), id(id), type(Contact::BuddyType),
        sources(Contact::PhotoSizeBigRec + 1),
        preferedSize(Contact::PhotoSizeMediumRec)
    {

    }
    Contact *q_ptr;
    Client *client;
    int id;
    Contact::Type type;
    QVector<QString> sources;
    Contact::PhotoSize preferedSize;

    QString defaultSource() const
    {
        //return sources[preferedSize];
        for (int index = preferedSize; index != -1; index--) {
            auto photo = sources.value(index);
            if (!photo.isNull())
                return photo;
        }
        return QString();
    }
    QString smallSource() { return sources[Contact::PhotoSizeSmall]; }
    void setSmallSource(const QString &source)
    {
        q_func()->setPhotoSource(source, Contact::PhotoSizeSmall);
    }
    QString mediumSource() { return sources[Contact::PhotoSizeMedium]; }
    void setMediumSource(const QString &source)
    {
        q_func()->setPhotoSource(source, Contact::PhotoSizeMedium);
    }
    QString mediumSourceRec() { return sources[Contact::PhotoSizeMediumRec]; }
    void setMediumSourceRec(const QString &source)
    {
        q_func()->setPhotoSource(source, Contact::PhotoSizeMediumRec);
    }
    QString bigSource() { return sources[Contact::PhotoSizeBig]; }
    void setBigSource(const QString &source)
    {
        q_func()->setPhotoSource(source, Contact::PhotoSizeBig);
    }
    QString bigSourceRec() { return sources[Contact::PhotoSizeBigRec]; }
    void setBigSourceRec(const QString &source)
    {
        q_func()->setPhotoSource(source, Contact::PhotoSizeBigRec);
    }
};

class BuddyPrivate : public ContactPrivate
{
    Q_DECLARE_PUBLIC(Buddy)
public:
    BuddyPrivate(Contact *q, int id, Client *client) :
        ContactPrivate(q, id, client),
        status(Buddy::Offline),
        isFriend(false)
    {
        type = Contact::BuddyType;
    }
    QString firstName;
    QString lastName;
    Buddy::Status status;
    QVariantList tagIdList;
    QString activity;
    bool isFriend;

    QVariantList lists() const { return QVariantList(); }
    void setLists(const QVariantList &list)
    {
        Q_Q(Buddy);
        tagIdList.clear();
        foreach (auto value, list)
            tagIdList.append(value);
        emit q->tagsChanged(q->tags());
    }
    QString getActivity() const { return activity; }
    void setActivity(const QString &now)
    {
        if (activity != now) {
            activity = now;
            emit q_func()->activityChanged(now);
        }
    }

	void _q_friends_add_finished(const QVariant &response);
	void _q_friends_delete_finished(const QVariant &response);
};

class GroupPrivate : public ContactPrivate
{
    Q_DECLARE_PUBLIC(Group)
public:
    GroupPrivate(Contact *q, int id, Client *client) : ContactPrivate(q, id, client) {}
    QString name;
};

} //namespace Vreen

#endif // USER_P_H

