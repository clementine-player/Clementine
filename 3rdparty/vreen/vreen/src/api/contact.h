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
#ifndef VK_USER_H
#define VK_USER_H

#include "client.h"
#include <QStringList>
#include <QVariant>

namespace Vreen {

#define VK_COMMON_FIELDS QLatin1String("first_name") \
    << QLatin1String("last_name") \
    << QLatin1String("online") \
    << QLatin1String("photo") \
    << QLatin1String("photo_medium") \
    << QLatin1String("photo_medium_rec") \
    << QLatin1String("photo_big") \
    << QLatin1String("photo_big_rec") \
    << QLatin1String("lists") \
    << QLatin1String("activity")

#define VK_EXTENDED_FIELDS QLatin1String("sex") \
    << QLatin1String("bdate") \
    << QLatin1String("city") \
    << QLatin1String("country") \
    << QLatin1String("education") \
    << QLatin1String("can_post") \
    << QLatin1String("contacts") \
    << QLatin1String("can_see_all_posts") \
    << QLatin1String("can_write_private_message") \
    << QLatin1String("last_seen") \
    << QLatin1String("relation") \
    << QLatin1String("nickname") \
    << QLatin1String("wall_comments") \

#define VK_GROUP_FIELDS QLatin1String("city") \
    << "country" \
    << "place" \
    << "description" \
    << "wiki_page" \
    << "members_count" \
    << "counters" \
    << "start_date" \
    << "end_date" \
    << "can_post" \
    << "activity"

#define VK_ALL_FIELDS VK_COMMON_FIELDS \
    << VK_EXTENDED_FIELDS

class Client;
class ContactPrivate;
class VK_SHARED_EXPORT Contact : public QObject
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(Contact)
    Q_ENUMS(Type)
    Q_ENUMS(Status)

    Q_PROPERTY(int id READ id CONSTANT)
    Q_PROPERTY(QString name READ name NOTIFY nameChanged)
    Q_PROPERTY(Type type READ type CONSTANT)
    Q_PRIVATE_PROPERTY(Contact::d_func(), QString photoSource READ defaultSource NOTIFY photoSourceChanged)
    Q_PRIVATE_PROPERTY(Contact::d_func(), QString photoSourceBig READ bigSource NOTIFY photoSourceChanged)

    Q_PRIVATE_PROPERTY(Contact::d_func(), QString _q_photo READ smallSource WRITE setSmallSource DESIGNABLE false)
    Q_PRIVATE_PROPERTY(Contact::d_func(), QString _q_photo_medium READ mediumSource WRITE setMediumSource DESIGNABLE false)
    Q_PRIVATE_PROPERTY(Contact::d_func(), QString _q_photo_medium_rec READ mediumSourceRec WRITE setMediumSourceRec DESIGNABLE false)
    Q_PRIVATE_PROPERTY(Contact::d_func(), QString _q_photo_big READ bigSource WRITE setBigSource DESIGNABLE false)
    Q_PRIVATE_PROPERTY(Contact::d_func(), QString _q_photo_big_rec READ bigSourceRec WRITE setBigSourceRec DESIGNABLE false)
public:

    enum PhotoSize {
        PhotoSizeSmall,
        PhotoSizeMedium,
        PhotoSizeBig,
        PhotoSizeMediumRec,
        PhotoSizeBigRec
    };

    enum Type {
        BuddyType,
        GroupType,
        ChatType
    };

    enum Status {
        Online,
        Away,
        Offline,
        Unknown
    };

    Contact(int id, Client *client);
    Contact(ContactPrivate *data);
    virtual ~Contact();
    virtual QString name() const = 0;
    Type type();
    int id() const;
    Client *client() const;
    Q_INVOKABLE QString photoSource(PhotoSize size = PhotoSizeSmall) const;
    void setPhotoSource(const QString &source, PhotoSize size = PhotoSizeSmall);    
    static void fill(Contact *contact, const QVariantMap &data);
signals:
    void nameChanged(const QString &name);
    void photoSourceChanged(const QString &source, Vreen::Contact::PhotoSize);
protected:
    QScopedPointer<ContactPrivate> d_ptr;
};

#define VK_CONTACT_TYPE(ContactType) \
    public: \
    static Contact::Type staticType() { return ContactType; } \
    virtual Contact::Type type() const { return staticType(); } \
    private:

class BuddyPrivate;
class VK_SHARED_EXPORT Buddy : public Contact
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(Buddy)
    VK_CONTACT_TYPE(BuddyType)

    Q_PROPERTY(QString fistName READ firstName NOTIFY firstNameChanged)
    Q_PROPERTY(QString lastName READ lastName NOTIFY lastNameChanged)
    Q_PROPERTY(bool online READ isOnline NOTIFY onlineChanged)
    Q_PROPERTY(QStringList tags READ tags NOTIFY tagsChanged)
    Q_PROPERTY(QString activity READ activity NOTIFY activityChanged)
    Q_PROPERTY(Status status READ status NOTIFY statusChanged)
    Q_PROPERTY(bool isFriend READ isFriend NOTIFY isFriendChanged)

    //private properties
    Q_PROPERTY(QString _q_first_name READ firstName WRITE setFirstName DESIGNABLE false)
    Q_PROPERTY(QString _q_last_name READ lastName WRITE setLastName DESIGNABLE false)
    Q_PROPERTY(bool _q_online READ isOnline WRITE setOnline DESIGNABLE false STORED false)
    Q_PRIVATE_PROPERTY(d_func(), QVariantList _q_lists READ lists WRITE setLists DESIGNABLE false)
    Q_PRIVATE_PROPERTY(d_func(), QString _q_activity READ getActivity WRITE setActivity DESIGNABLE false)
public:
    //TODO name case support maybe needed
    QString firstName() const;
    void setFirstName(const QString &firstName);
    QString lastName() const;
    void setLastName(const QString &lastName);
    bool isOnline() const;
    void setOnline(bool set);
    virtual QString name() const;
    QStringList tags() const;
    QString activity() const;
    Status status() const;
    void setStatus(Status status);
    bool isFriend() const;
    void setIsFriend(bool set);
public slots:
    void update(const QStringList &fields);
    void update();
    SendMessageReply *sendMessage(const QString &body, const QString &subject = QString());
	Reply *addToFriends(const QString &reason = QString());
	Reply *removeFromFriends();
signals:
    void firstNameChanged(const QString &name);
    void lastNameChanged(const QString &name);
    void onlineChanged(bool isOnline);
    void tagsChanged(const QStringList &tags);
    void activityChanged(const QString &activity);
    void statusChanged(Vreen::Contact::Status);
    void isFriendChanged(bool isFriend);
protected:
    Buddy(int id, Client *client);

    friend class Roster;
    friend class RosterPrivate;

	Q_PRIVATE_SLOT(d_func(), void _q_friends_add_finished(const QVariant &response))
	Q_PRIVATE_SLOT(d_func(), void _q_friends_delete_finished(const QVariant &response))
};

class GroupPrivate;
class VK_SHARED_EXPORT Group : public Contact
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(Group)
    VK_CONTACT_TYPE(GroupType)

    Q_PROPERTY(QString _q_name READ name WRITE setName DESIGNABLE false)
public:
    virtual QString name() const;
    void setName(const QString &name);
public slots:
    void update();
protected:
    Group(int id, Client *client);

    friend class GroupManager;
};

//TODO group chats
class GroupChat;

typedef QList<Contact*> ContactList;
typedef QList<Buddy*> BuddyList;
typedef QList<Group*> GroupList;

//contact's casts
template <typename T>
Q_INLINE_TEMPLATE T contact_cast(Contact *contact)
{
    //T t = reinterpret_cast<T>(0);
    //if (t->staticType() == contact->type())
    //    return static_cast<T>(contact);
    return qobject_cast<T>(contact);
}

} // namespace Vreen

Q_DECLARE_METATYPE(Vreen::Contact*)
Q_DECLARE_METATYPE(Vreen::Buddy*)
Q_DECLARE_METATYPE(Vreen::Group*)

#endif // VK_USER_H

