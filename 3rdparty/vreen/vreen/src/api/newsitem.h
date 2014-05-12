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
#ifndef VK_NEWSITEM_H
#define VK_NEWSITEM_H

#include <QSharedDataPointer>
#include <QVariant>
#include "attachment.h"

namespace Vreen {

class NewsItemData;

class VK_SHARED_EXPORT NewsItem
{
    Q_GADGET
    Q_ENUMS(Type)
public:

    enum Type {
        Post,
        Photo,
        PhotoTag,
        Note,
        Invalid = -1
    };

    NewsItem();
    NewsItem(const NewsItem &);
    NewsItem &operator=(const NewsItem &);
    ~NewsItem();

    static NewsItem fromData(const QVariant &data);

    Attachment::Hash attachments() const;
    Attachment::List attachments(Attachment::Type type) const;
    void setAttachments(const Attachment::List &attachmentList);
    Type type() const;
    void setType(Type type);
    int postId() const;
    void setPostId(int postId);
    int sourceId() const;
    void setSourceId(int sourceId);
    QString body() const;
    void setBody(const QString &body);
    QDateTime date() const;
    void setDate(const QDateTime &date);
    QVariantMap likes() const;
    void setLikes(const QVariantMap &likes);
    QVariantMap reposts() const;
    void setReposts(const QVariantMap &reposts);

    QVariant property(const QString &name, const QVariant &def = QVariant()) const;
    template<typename T>
    T property(const char *name, const T &def) const
    { return QVariant::fromValue(property(name, QVariant::fromValue(def))); }
    void setProperty(const QString &name, const QVariant &value);
    QStringList dynamicPropertyNames() const;

    VK_SHARED_EXPORT friend QDataStream &operator <<(QDataStream &out, const Vreen::NewsItem &item);
    VK_SHARED_EXPORT friend QDataStream &operator >>(QDataStream &out, Vreen::NewsItem &item);
protected:
    NewsItem(const QVariantMap &data);
    void setData(const QVariantMap &data);
private:
    QSharedDataPointer<NewsItemData> d;
};
typedef QList<NewsItem> NewsItemList;

} // namespace Vreen

Q_DECLARE_METATYPE(Vreen::NewsItem)
Q_DECLARE_METATYPE(Vreen::NewsItemList)

#endif // VK_NEWSITEM_H

