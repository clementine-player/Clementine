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
#include "audioitem.h"
#include <QSharedData>
#include <QUrl>
#include "client.h"

namespace Vreen {

template<>
AudioItem Attachment::to(const Attachment &data)
{
    AudioItem item;
    item.setId(data.property("aid").toInt());
    item.setOwnerId(data.property("owner_id").toInt());
    item.setArtist(data.property("performer").toString());
    item.setTitle(data.property("title").toString());
    item.setUrl(data.property("url").toUrl());
    item.setDuration(data.property("duration").toDouble());
    return item;
}

class AudioItemData : public QSharedData {
public:
    AudioItemData() :
        id(0), ownerId(0),
        duration(0),
        lyricsId(0),
        albumId(0)
    {}
    AudioItemData(AudioItemData &o) : QSharedData(),
        id(o.id), ownerId(o.ownerId),
        artist(o.artist),
        title(o.title),
        duration(o.duration),
        url(o.url),
        lyricsId(o.lyricsId),
        albumId(o.albumId)
    {}
    int id;
    int ownerId;
    QString artist;
    QString title;
    qreal duration;
    QUrl url;
    int lyricsId;
    int albumId;
};

AudioItem::AudioItem() : data(new AudioItemData)
{
}

AudioItem::AudioItem(const AudioItem &rhs) : data(rhs.data)
{
}

AudioItem &AudioItem::operator=(const AudioItem &rhs)
{
    if (this != &rhs)
        data.operator=(rhs.data);
    return *this;
}

AudioItem::~AudioItem()
{
}

int AudioItem::id() const
{
    return data->id;
}

void AudioItem::setId(int id)
{
    data->id = id;
}

int AudioItem::ownerId() const
{
    return data->ownerId;
}

void AudioItem::setOwnerId(int ownerId)
{
    data->ownerId = ownerId;
}

QString AudioItem::artist() const
{
    return data->artist;
}

void AudioItem::setArtist(const QString &artist)
{
    data->artist = artist;
}

QString AudioItem::title() const
{
    return data->title;
}

void AudioItem::setTitle(const QString &title)
{
    data->title = title;
}

qreal AudioItem::duration() const
{
    return data->duration;
}

void AudioItem::setDuration(qreal duration)
{
    data->duration = duration;
}

QUrl AudioItem::url() const
{
    return data->url;
}

void AudioItem::setUrl(const QUrl &url)
{
    data->url = url;
}

int AudioItem::lyricsId() const
{
    return data->lyricsId;
}

void AudioItem::setLyricsId(int lyricsId)
{
    data->lyricsId = lyricsId;
}

int AudioItem::albumId() const
{
    return data->albumId;
}

void AudioItem::setAlbumId(int albumId)
{
    data->albumId = albumId;
}

class AudioAlbumItemData : public QSharedData {
public:
    AudioAlbumItemData() :
        id(0),
        ownerId(0)
    {}
    AudioAlbumItemData(AudioAlbumItemData &o) : QSharedData(),
        id(o.id),
        ownerId(o.ownerId),
        title(o.title)
    {}
    int id;
    int ownerId;
    QString title;
};

AudioAlbumItem::AudioAlbumItem() : data(new AudioAlbumItemData)
{
}

AudioAlbumItem::AudioAlbumItem(const AudioAlbumItem &rhs) : data(rhs.data)
{
}

AudioAlbumItem &AudioAlbumItem::operator=(const AudioAlbumItem &rhs)
{
    if (this != &rhs)
        data.operator=(rhs.data);
    return *this;
}

AudioAlbumItem::~AudioAlbumItem()
{

}

int AudioAlbumItem::ownerId() const
{
    return data->ownerId;
}

void AudioAlbumItem::setOwnerId(int ownerId)
{
    data->ownerId = ownerId;
}

int AudioAlbumItem::id() const
{
    return data->id;
}

void AudioAlbumItem::setId(int id)
{
    data->id = id;
}

QString AudioAlbumItem::title() const
{
    return data->title;
}

void AudioAlbumItem::setTitle(const QString &title)
{
    data->title = title;
}

} // namespace Vreen

