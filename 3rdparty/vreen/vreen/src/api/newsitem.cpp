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
#include "newsitem.h"
#include "utils.h"
#include "utils_p.h"
#include <QSharedData>
#include <QStringList>
#include <QDateTime>
#include <QDataStream>

namespace Vreen {

class NewsItemData : public QSharedData {
public:
    NewsItemData() : QSharedData() {}
    NewsItemData(const NewsItemData &o) : QSharedData(o),
        body(o.body),
        data(o.data),
        likes(o.likes),
        reposts(o.reposts),
        attachmentHash(o.attachmentHash)
    {}

    QString body;

    QVariantMap data;
    QVariantMap likes;
    QVariantMap reposts;
    Attachment::Hash attachmentHash;
};

QDataStream &operator <<(QDataStream &out, const NewsItem &item)
{
    auto &d = item.d;
    return out << d->body
               << d->data
               << d->likes
               << d->reposts
               << d->attachmentHash.values();
}

QDataStream &operator >>(QDataStream &out, NewsItem &item)
{
    auto &d = item.d;
    Attachment::List list;
    out >> d->body
        >> d->data
        >> d->likes
        >> d->reposts
        >> list;
    d->attachmentHash = Attachment::toHash(list);
    return out;
}

const static QStringList types = QStringList()
        << "post"
        << "photo"
        << "photo_tag"
        << "friend"
        << "note";

/*!
 * \brief The NewsItem class
 * Api reference: \link http://vk.com/developers.php?oid=-1&p=newsfeed.get
 */

/*!
 * \brief NewsItem::NewsItem
 */

NewsItem::NewsItem() : d(new NewsItemData)
{
}

NewsItem::NewsItem(const QVariantMap &data) : d(new NewsItemData)
{
    setData(data);
}

NewsItem::NewsItem(const NewsItem &rhs) : d(rhs.d)
{
}

NewsItem &NewsItem::operator=(const NewsItem &rhs)
{
    if (this != &rhs)
        d.operator=(rhs.d);
    return *this;
}

NewsItem::~NewsItem()
{
}

NewsItem NewsItem::fromData(const QVariant &data)
{
    return NewsItem(data.toMap());
}

void NewsItem::setData(const QVariantMap &data)
{
    d->data = data;
    d->body = fromHtmlEntities(d->data.take("text").toString());
    d->likes = d->data.take("likes").toMap();
    d->reposts = d->data.take("reposts").toMap();
    auto attachmentList = Attachment::fromVariantList(d->data.take("attachments").toList());
    setAttachments(attachmentList);

}

Attachment::Hash NewsItem::attachments() const
{
    return d->attachmentHash;
}

Attachment::List NewsItem::attachments(Attachment::Type type) const
{
    return d->attachmentHash.values(type);
}

void NewsItem::setAttachments(const Attachment::List &attachmentList)
{
    d->attachmentHash = Attachment::toHash(attachmentList);
}

NewsItem::Type NewsItem::type() const
{
    return static_cast<Type>(types.indexOf(d->data.value("type").toString()));
}

void NewsItem::setType(NewsItem::Type type)
{
    d->data.insert("type", types.value(type));
}

int NewsItem::postId() const
{
    return d->data.value("post_id").toInt();
}

void NewsItem::setPostId(int postId)
{
    d->data.insert("post_id", postId);
}

int NewsItem::sourceId() const
{
    return d->data.value("source_id").toInt();
}

void NewsItem::setSourceId(int sourceId)
{
    d->data.insert("source_id", sourceId);
}

QString NewsItem::body() const
{
    return d->body;
}

void NewsItem::setBody(const QString &body)
{
    d->body = body;
}

QDateTime NewsItem::date() const
{
    return QDateTime::fromTime_t(d->data.value("date").toInt());
}

void NewsItem::setDate(const QDateTime &date)
{
    d->data.insert("date", date.toTime_t());
}

QVariant NewsItem::property(const QString &name, const QVariant &def) const
{
    return d->data.value(name, def);
}

QStringList NewsItem::dynamicPropertyNames() const
{
    return d->data.keys();
}

void NewsItem::setProperty(const QString &name, const QVariant &value)
{
    d->data.insert(name, value);
}

QVariantMap NewsItem::likes() const
{
    return d->likes;
}

void NewsItem::setLikes(const QVariantMap &likes)
{
    d->likes = likes;
}

QVariantMap NewsItem::reposts() const
{
    return d->reposts;
}

void NewsItem::setReposts(const QVariantMap &reposts)
{
    d->reposts = reposts;
}

} // namespace Vreen

#include "moc_newsitem.cpp"
