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
#include "attachment.h"
#include <QSharedData>
#include <QStringList>

namespace Vreen {

QDataStream &operator <<(QDataStream &out, const Vreen::Attachment &item)
{
    return out << item.data();
}

QDataStream &operator >>(QDataStream &out, Vreen::Attachment &item)
{
    QVariantMap data;
    out >> data;
    item.setData(data);
    return out;
}

const static QStringList types = QStringList()
        << "photo"
        << "posted_photo"
        << "video"
        << "audio"
        << "doc"
        << "graffiti"
        << "link"
        << "note"
        << "app"
        << "poll"
        << "page";

class AttachmentData : public QSharedData {
public:
    AttachmentData() : QSharedData(),
        type(Attachment::Other),
        ownerId(0),
        mediaId(0) {}
    AttachmentData(const AttachmentData &o) : QSharedData(o),
        type(o.type),
        ownerId(o.ownerId),
        mediaId(o.mediaId),
        data(o.data) {}
    Attachment::Type type;
    int ownerId;
    int mediaId;
    QVariantMap data;

    static Attachment::Type getType(const QString &type)
    {
        return static_cast<Attachment::Type>(types.indexOf(type));
    }
};

/*!
 * \brief The Attachment class
 * Api reference: \link http://vk.com/developers.php?oid=-1&p=Описание_поля_attachments
 */

/*!
 * \brief Attachment::Attachment
 */
Attachment::Attachment() : d(new AttachmentData)
{
}

Attachment::Attachment(const QVariantMap &data) : d(new AttachmentData)
{
    setData(data);
}

Attachment::Attachment(const QString &string) : d(new AttachmentData)
{
    QRegExp regex("(\\w+)(\\d+)_(\\d+)");
    regex.indexIn(string);
    //convert type to enum
    d->data.insert("type", d->type = AttachmentData::getType(regex.cap(1)));
    d->ownerId = regex.cap(2).toInt();
    d->mediaId = regex.cap(3).toInt();
}

Attachment::Attachment(const Attachment &rhs) : d(rhs.d)
{
}

Attachment &Attachment::operator=(const Attachment &rhs)
{
    if (this != &rhs)
        d.operator=(rhs.d);
    return *this;
}

Attachment::~Attachment()
{
}

void Attachment::setData(const QVariantMap &data)
{
    d->data.clear();
    QString type = data.value("type").toString();

    //move properties to top level
    auto map = data.value(type).toMap();
    for (auto it = map.constBegin(); it != map.constEnd(); it++)
        d->data.insert(it.key(), it.value());
    //convert type to enum
    d->data.insert("type", d->type = AttachmentData::getType(type));
}

QVariantMap Attachment::data() const
{
    return d->data;
}

Attachment::Type Attachment::type() const
{
    return d->type;
}

void Attachment::setType(Attachment::Type type)
{
    d->type = type;
}

void Attachment::setType(const QString &type)
{
    d->type = d->getType(type);
}

int Attachment::ownerId() const
{
    return d->ownerId;
}

void Attachment::setOwnerId(int ownerId)
{
    d->ownerId = ownerId;
}

int Attachment::mediaId() const
{
    return d->mediaId;
}

void Attachment::setMediaId(int id)
{
    d->mediaId = id;
}

Attachment Attachment::fromData(const QVariant &data)
{
    return Attachment(data.toMap());
}

Attachment::List Attachment::fromVariantList(const QVariantList &list)
{
    Attachment::List attachments;
    foreach (auto item, list)
        attachments.append(Attachment::fromData(item.toMap()));
    return attachments;
}

QVariantList Attachment::toVariantList(const Attachment::List &list)
{
    QVariantList variantList;
    foreach (auto item, list)
        variantList.append(item.data());
    return variantList;
}

Attachment::Hash Attachment::toHash(const Attachment::List &list)
{
    Hash hash;
    foreach (auto attachment, list)
        hash.insert(attachment.type(), attachment);
    return hash;
}

QVariantMap Attachment::toVariantMap(const Attachment::Hash &hash)
{
    //FIXME i want to Qt5
    QVariantMap map;
    foreach (auto key, hash.keys())
        map.insert(QString::number(key), toVariantList(hash.values(key)));
    return map;
}

QVariant Attachment::property(const QString &name, const QVariant &def) const
{
    return d->data.value(name, def);
}

QStringList Attachment::dynamicPropertyNames() const
{
    return d->data.keys();
}

void Attachment::setProperty(const QString &name, const QVariant &value)
{
    d->data.insert(name, value);
}

bool Attachment::isFetched() const
{
    return !d->data.isEmpty();
}

} // namespace Vreen

#include "moc_attachment.cpp"
