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
#ifndef WALLPOST_H
#define WALLPOST_H

#include <QSharedDataPointer>
#include <QVariant>
#include "vk_global.h"
#include "attachment.h"

namespace Vreen {

class WallPostData;
class Client;
class Contact;

class VK_SHARED_EXPORT WallPost
{
public:  
    WallPost();
    WallPost(const WallPost &);
    WallPost &operator=(const WallPost &);
    ~WallPost();

    void setId(int id);
    int id() const;
    void setBody(const QString &body);
    QString body() const;
    void setFromId(int id);
    int fromId() const;
    void setToId(int id);
    int toId() const;
    int ownerId() const;
    void setOwnerId(int ownerId);
    void setDate(const QDateTime &date);
    QDateTime date() const;
    int signerId() const;
    void setSignerId(int signerId);
    QString copyText() const;
    void setCopyText(const QString &copyText);
    Attachment::Hash attachments() const;
    Attachment::List attachments(Attachment::Type type) const;
    void setAttachments(const Attachment::List &attachmentList);
    QVariantMap likes() const;
    void setLikes(const QVariantMap &likes);
    QVariantMap reposts() const;
    void setReposts(const QVariantMap &reposts);

    static WallPost fromData(const QVariant data);

    QVariant property(const QString &name, const QVariant &def = QVariant()) const;
    template<typename T>
    T property(const char *name, const T &def) const
    { return QVariant::fromValue(property(name, QVariant::fromValue(def))); }

    void setProperty(const QString &name, const QVariant &value);
    QStringList dynamicPropertyNames() const;
protected:
    WallPost(QVariantMap data);
private:
    QSharedDataPointer<WallPostData> d;
};
typedef QList<WallPost> WallPostList;

} //namespace Vreen

#endif // WALLPOST_H

