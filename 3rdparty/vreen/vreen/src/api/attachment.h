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
#ifndef VK_ATTACHMENT_H
#define VK_ATTACHMENT_H

#include <QSharedDataPointer>
#include <QVariantMap>
#include "vk_global.h"

namespace Vreen {

class AttachmentData;

class VK_SHARED_EXPORT Attachment
{
    Q_GADGET
    Q_ENUMS(Type)
public:    
    enum Type {
        Photo,
        PostedPhoto,
        Video,
        Audio,
        Document,
        Graffiti,
        Link,
        Note,
        ApplicationImage,
        Poll,
        Page,
        Other = -1
    };
    typedef QList<Attachment> List;
    typedef QMultiHash<Attachment::Type, Attachment> Hash;

    Attachment();
    Attachment(const Attachment &);
    Attachment &operator=(const Attachment &);
    ~Attachment();

    void setData(const QVariantMap &data);
    QVariantMap data() const;
    Type type() const;
    void setType(Type);
    void setType(const QString &type);
    int ownerId() const;
    void setOwnerId(int ownerId);
    int mediaId() const;
    void setMediaId(int mediaId);
    bool isFetched() const;

    static Attachment fromData(const QVariant &data);
    static List fromVariantList(const QVariantList &list);
    static QVariantList toVariantList(const List &list);
    static Hash toHash(const List &list);
    static QVariantMap toVariantMap(const Hash &hash);

    QVariant property(const QString &name, const QVariant &def = QVariant()) const;
    template<typename T>
    T property(const char *name, const T &def) const
    { return QVariant::fromValue<T>(property(name, QVariant::fromValue(def))); }
    void setProperty(const QString &name, const QVariant &value);
    QStringList dynamicPropertyNames() const;
    template <typename T>
    static T to(const Attachment &attachment);
    template <typename T>
    static Attachment from(const T &item);

    friend QDataStream &operator <<(QDataStream &out, const Vreen::Attachment &item);
    friend QDataStream &operator >>(QDataStream &out, Vreen::Attachment &item);
protected:
    Attachment(const QVariantMap &data);
    Attachment(const QString &string);
private:
    QSharedDataPointer<AttachmentData> d;
};

} // namespace Vreen

Q_DECLARE_METATYPE(Vreen::Attachment)
Q_DECLARE_METATYPE(Vreen::Attachment::List)
Q_DECLARE_METATYPE(Vreen::Attachment::Hash)
Q_DECLARE_METATYPE(Vreen::Attachment::Type)


#endif // VK_ATTACHMENT_H

