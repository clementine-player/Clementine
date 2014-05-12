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
#ifndef NEWSFEED_H
#define NEWSFEED_H
#include <QObject>
#include "newsitem.h"
#include <QVariantMap>

namespace Vreen {

class NewsFeedPrivate;
class Client;
class Reply;

class VK_SHARED_EXPORT NewsFeed : public QObject
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(NewsFeed)
    Q_ENUMS(Filter)
public:

    enum Filter {
        FilterNone      =   0,
        FilterPost      =   0x01,
        FilterPhoto     =   0x02,
        FilterPhotoTag  =   0x04,
        FilterFriend    =   0x08,
        FilterNote      =   0x10
    };
    Q_DECLARE_FLAGS(Filters, Filter)

    NewsFeed(Client *client);
    virtual ~NewsFeed();
public slots:
    Reply *getNews(Filters filters = FilterNone, quint8 count = 25, int offset = 0);
signals:
    void newsReceived(const Vreen::NewsItemList &list);
private:
    QScopedPointer<NewsFeedPrivate> d_ptr;

    Q_PRIVATE_SLOT(d_func(), void _q_news_received(QVariant))
};

} //namespace Vreen

Q_DECLARE_OPERATORS_FOR_FLAGS(Vreen::NewsFeed::Filters)

#endif // NEWSFEED_H

