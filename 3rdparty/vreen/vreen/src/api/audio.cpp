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
#include "audio.h"
#include "client.h"
#include "reply_p.h"
#include "utils_p.h"
#include <QUrl>
#include <QCoreApplication>
#include <QDebug>
#include <QtCore/qalgorithms.h>

namespace Vreen {

class AudioProvider;
class AudioProviderPrivate
{
    Q_DECLARE_PUBLIC(AudioProvider)
public:
    AudioProviderPrivate(AudioProvider *q, Client *client) : q_ptr(q), client(client) {}
    AudioProvider *q_ptr;
    Client *client;

    static QVariant handleAudio(const QVariant &response) {
        AudioItemList items;
        auto list = response.toList();
        if (list.count() && list.first().canConvert<int>())
            list.removeFirst(); //HACK For stupid API(((

        foreach (auto item, list) {
            auto map = item.toMap();
            AudioItem audio;
            audio.setId(map.value("aid").toInt());
            audio.setOwnerId(map.value("owner_id").toInt());
            audio.setArtist(fromHtmlEntities(map.value("artist").toString()));
            audio.setTitle(fromHtmlEntities(map.value("title").toString()));
            audio.setDuration(map.value("duration").toReal());
            audio.setAlbumId(map.value("album").toInt());
            audio.setLyricsId(map.value("lyrics_id").toInt());
            audio.setUrl(map.value("url").toUrl());
            items.append(audio);
        }
        return QVariant::fromValue(items);
    }

    static QVariant handleAudioAlbum(const QVariant &response) {
        AudioAlbumItemList items;
        auto list = response.toList();

        if (list.count() && list.first().canConvert<int>())
            list.removeFirst();

        foreach (auto item, list) {
            auto map = item.toMap();
            AudioAlbumItem audio;
            audio.setId(map.value("album_id").toInt());
            audio.setOwnerId(map.value("owner_id").toInt());
            audio.setTitle(fromHtmlEntities(map.value("title").toString()));
            items.append(audio);
        }
        return QVariant::fromValue(items);
    }
};

AudioProvider::AudioProvider(Client *client) :
    d_ptr(new AudioProviderPrivate(this, client))
{

}

AudioProvider::~AudioProvider()
{

}

/*!
 * \brief AudioProvider::get \link http://vk.com/developers.php?oid=-1&p=audio.get
 * \param uid
 * \param count
 * \param offset
 * \return reply
 */
AudioItemListReply *AudioProvider::getContactAudio(int uid, int count, int offset, int album_id)
{
    Q_D(AudioProvider);
    QVariantMap args;
    if (uid)
        args.insert(uid > 0 ? "uid" : "gid", qAbs(uid));
    if (album_id > 0)
        args.insert("album_id", album_id);
    args.insert("count", count);
    args.insert("offset", offset);

    auto reply = d->client->request<AudioItemListReply>("audio.get", args, AudioProviderPrivate::handleAudio);
    return reply;
}

/*!
 * \brief AudioProvider::searchAudio \link http://vk.com/developers.php?oid=-1&p=audio.search
 *
 * \param query
 * \param autocomplete
 * \param lyrics
 * \param count
 * \param offset
 * \return reply
 **/
AudioItemListReply *AudioProvider::searchAudio(const QString& query, int count, int offset, bool autoComplete, Vreen::AudioProvider::SortOrder sort, bool withLyrics)
{
    Q_D(AudioProvider);
    QVariantMap args;
    args.insert("q", query);
    args.insert("auto_complete", autoComplete);
    args.insert("sort", static_cast<int>(sort));
    args.insert("lyrics", withLyrics);
    args.insert("count", count);
    args.insert("offset", offset);

    auto reply = d->client->request<AudioItemListReply>("audio.search", args, AudioProviderPrivate::handleAudio);
    return reply;
}

AudioAlbumItemListReply *AudioProvider::getAlbums(int ownerId, int count, int offset)
{
    Q_D(AudioProvider);
    QVariantMap args;
    args.insert("owner_id", ownerId);
    args.insert("count", count);
    args.insert("offset", offset);

    auto reply = d->client->request<AudioAlbumItemListReply>("audio.getAlbums", args, AudioProviderPrivate::handleAudioAlbum);
    return reply;
}

AudioItemListReply *AudioProvider::getRecommendationsForUser(int uid, int count, int offset)
{
    Q_D(AudioProvider);
    QVariantMap args;
    if (uid < 0) {
        qDebug("Vreen::AudioProvider::getRecomendationForUser may not work with groups (uid < 0)");

    }
    args.insert("uid",uid);
    args.insert("count", count);
    args.insert("offset", offset);

    auto reply = d->client->request<AudioItemListReply>("audio.getRecommendations", args, AudioProviderPrivate::handleAudio);
    return reply;
}

IntReply *AudioProvider::getCount(int oid)
{
    Q_D(AudioProvider);

    oid = oid?oid:d->client->id();

    QVariantMap args;
    args.insert("oid", oid);

    auto reply = d->client->request<IntReply>("audio.getCount", args, ReplyPrivate::handleInt);
    return reply;
}

IntReply *AudioProvider::addToLibrary(int aid, int oid, int gid)
{
    Q_D(AudioProvider);

    QVariantMap args;
    args.insert("aid", aid);
    args.insert("oid", oid);

    if (gid) {
        args.insert("gid",gid);
    }

    auto reply = d->client->request<IntReply>("audio.add", args, ReplyPrivate::handleInt);
    return reply;
}

IntReply *AudioProvider::removeFromLibrary(int aid, int oid)
{
    Q_D(AudioProvider);

    QVariantMap args;
    args.insert("aid", aid);
    args.insert("oid", oid);

    auto reply = d->client->request<IntReply>("audio.delete", args, ReplyPrivate::handleInt);
    return reply;
}

IdListReply *AudioProvider::setBroadcast(int aid, int oid, const IdList &targetIds)
{
    Q_D(AudioProvider);

    QVariantMap args;
    args.insert("audio", QString("%1_%2").arg(oid).arg(aid));
    args.insert("target_ids", join(targetIds));

    auto reply = d->client->request<IdListReply>("audio.setBroadcast", args, ReplyPrivate::handleIdList);
    return reply;
}

IdListReply *AudioProvider::resetBroadcast(const IdList &targetIds)
{
    Q_D(AudioProvider);

    QVariantMap args;
    args.insert("audio","");
    args.insert("target_ids", join(targetIds));
    auto reply = d->client->request<IdListReply>("audio.setBroadcast", args, ReplyPrivate::handleIdList);
    return reply;
}

AudioItemListReply *AudioProvider::getAudiosByIds(const QString &ids)
{
    Q_D(AudioProvider);

    QVariantMap args;
    args.insert("audios", ids);

    auto reply = d->client->request<AudioItemListReply>("audio.getById", args, AudioProviderPrivate::handleAudio);
    return reply;
}

class AudioModel;
class AudioModelPrivate
{
    Q_DECLARE_PUBLIC(AudioModel)
public:
    AudioModelPrivate(AudioModel *q) : q_ptr(q) {}
    AudioModel *q_ptr;
    AudioItemList itemList;

    IdComparator<AudioItem> audioItemComparator;
};

AudioModel::AudioModel(QObject *parent) : AbstractListModel(parent),
    d_ptr(new AudioModelPrivate(this))
{  
    auto roles = roleNames();
    roles[IdRole] = "aid";
    roles[TitleRole] = "title";
    roles[ArtistRole] = "artist";
    roles[UrlRole] = "url";
    roles[DurationRole] = "duration";
    roles[AlbumIdRole] = "albumId";
    roles[LyricsIdRole] = "lyricsId";
    roles[OwnerIdRole] = "ownerId";
    setRoleNames(roles);
}

AudioModel::~AudioModel()
{
}

int AudioModel::count() const
{
    return d_func()->itemList.count();
}

void AudioModel::insertAudio(int index, const AudioItem &item)
{
    beginInsertRows(QModelIndex(), index, index);
    d_func()->itemList.insert(index, item);
    endInsertRows();
}

void AudioModel::replaceAudio(int i, const AudioItem &item)
{
    auto index = createIndex(i, 0);
    d_func()->itemList[i] = item;
    emit dataChanged(index, index);
}

void AudioModel::setAudio(const AudioItemList &items)
{
    Q_D(AudioModel);
    clear();
    beginInsertRows(QModelIndex(), 0, items.count());
    d->itemList = items;
    qSort(d->itemList.begin(), d->itemList.end(), d->audioItemComparator);
    endInsertRows();
}

void AudioModel::sort(int, Qt::SortOrder order)
{
    Q_D(AudioModel);
    d->audioItemComparator.sortOrder = order;
    setAudio(d->itemList);
}

void AudioModel::removeAudio(int aid)
{
    Q_D(AudioModel);
    int index = findAudio(aid);
    if (index == -1)
        return;
    beginRemoveRows(QModelIndex(), index, index);
    d->itemList.removeAt(index);
    endRemoveRows();
}

void AudioModel::addAudio(const AudioItem &item)
{
    Q_D(AudioModel);
    if (findAudio(item.id()) != -1)
        return;

    int index = d->itemList.count();
    beginInsertRows(QModelIndex(), index, index);
    d->itemList.append(item);
    endInsertRows();

    //int index = 0;
    //if (d->sortOrder == Qt::AscendingOrder)
    //    index = d->itemList.count();
    //insertAudio(index, item);
}

void AudioModel::clear()
{
    Q_D(AudioModel);
    beginRemoveRows(QModelIndex(), 0, d->itemList.count());
    d->itemList.clear();
    endRemoveRows();
}

void AudioModel::truncate(int count)
{
    Q_D(AudioModel);
    if (count > 0 && count > d->itemList.count()) {
        qWarning("Unable to truncate");
        return;
    }
    beginRemoveRows(QModelIndex(), 0, count);
    d->itemList.erase(d->itemList.begin() + count, d->itemList.end());
    endRemoveRows();
}

int AudioModel::rowCount(const QModelIndex &) const
{
    return count();
}

void AudioModel::setSortOrder(Qt::SortOrder order)
{
    Q_D(AudioModel);
    if (order != d->audioItemComparator.sortOrder) {
        d->audioItemComparator.sortOrder = order;
        emit sortOrderChanged(order);
        sort(0, order);
    }
}

Qt::SortOrder AudioModel::sortOrder() const
{
    Q_D(const AudioModel);
    return d->audioItemComparator.sortOrder;
}

QVariant AudioModel::data(const QModelIndex &index, int role) const
{
    Q_D(const AudioModel);
    int row = index.row();
    auto item = d->itemList.at(row);
    switch (role) {
    case IdRole:
        return item.id();
        break;
    case TitleRole:
        return item.title();
    case ArtistRole:
        return item.artist();
    case UrlRole:
        return item.url();
    case DurationRole:
        return item.duration();
    case AlbumIdRole:
        return item.albumId();
    case LyricsIdRole:
        return item.lyricsId();
    case OwnerIdRole:
        return item.ownerId();
    default:
        break;
    }
    return QVariant::Invalid;
}

int AudioModel::findAudio(int id) const
{
    Q_D(const AudioModel);
    //auto it = qBinaryFind(d->itemList.begin(), d->itemList.end(), id, d->audioItemComparator);
    //auto index = it - d->itemList.begin();
    //return index;

    for (int i = 0; i != d->itemList.count(); i++)
        if (d->itemList.at(i).id() == id)
            return id;
    return -1;
}

} // namespace Vreen

#include "moc_audio.cpp"

