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
#include "newsfeed.h"
#include "utils.h"
#include "client.h"
#include "roster.h"
#include "groupmanager.h"
#include <QVariantList>

namespace Vreen {

static const char *filters_str[] = {
    "post",
    "photo",
    "photo_tag",
    "friend",
    "note"
};

class NewsFeed;
class NewsFeedPrivate
{
    Q_DECLARE_PUBLIC(NewsFeed)
public:
    NewsFeedPrivate(NewsFeed *q, Client *client) : q_ptr(q), client(client) {}
    NewsFeed *q_ptr;
    Client *client;

    void updateProfiles(const QVariantList &profiles)
    {
        foreach (auto item, profiles) {
            auto map = item.toMap();
            int uid = map.value("uid").toInt();

            if (uid > 0) {
                auto roster = client->roster();
                auto buddy = roster->buddy(uid);
                Contact::fill(buddy, map);
            } else {
                auto manager = client->groupManager();
                auto group = manager->group(-uid);
                Contact::fill(group, map);
            }
        }
    }

    void updateGroups(const QVariantList &groups)
    {
        foreach (auto item, groups) {
            auto map = item.toMap();
            auto manager = client->groupManager();
            int gid = -map.value("gid").toInt();
            auto contact = manager->group(gid);
            Contact::fill(contact, map);
        }
    }

    void _q_news_received(const QVariant &response)
    {
        Q_Q(NewsFeed);
        auto map = response.toMap();
        updateProfiles(map.value("profiles").toList());
        updateGroups(map.value("groups").toList());

        auto items = map.value("items").toList();
        NewsItemList news;
        foreach (auto item, items) {
            auto newsItem = NewsItem::fromData(item);
            auto map = item.toMap();
            auto itemCount = strCount(filters_str);
            for (size_t i = 0; i != itemCount; i++) {
                auto list = map.value(filters_str[i]).toList();
                if (list.count()) {
                    auto count = list.takeFirst().toInt();
                    Q_UNUSED(count);
                    newsItem.setAttachments(Attachment::fromVariantList(list));
                    break;
                }
            }
            news.append(newsItem);
        }
        emit q->newsReceived(news);
    }
};

/*!
 * \brief The NewsFeed class
 * Api reference: \link http://vk.com/developers.php?oid=-1&p=Расширенные_методы_API
 */

/*!
 * \brief NewsFeed::NewsFeed
 * \param client
 */
NewsFeed::NewsFeed(Client *client) :
    QObject(client),
    d_ptr(new NewsFeedPrivate(this, client))
{

}

NewsFeed::~NewsFeed()
{

}

/*!
 * \brief NewsFeed::getNews API reference is \link http://vk.com/developers.php?oid=-1&p=newsfeed.get
 * \return
 */
Reply *NewsFeed::getNews(Filters filters, quint8 count, int offset)
{
    QVariantMap args;
    args.insert("count", count);
    args.insert("filters", flagsToStrList(filters, filters_str).join(","));
    args.insert("offset", offset);

    auto reply = d_func()->client->request("newsfeed.get", args);
    connect(reply, SIGNAL(resultReady(QVariant)), SLOT(_q_news_received(QVariant)));
    return reply;
}

} //namespace Vreen

#include "moc_newsfeed.cpp"

