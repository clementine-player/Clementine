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
#ifndef VK_AUDIOITEM_H
#define VK_AUDIOITEM_H

#include <QSharedDataPointer>
#include "attachment.h"
#include <QVariant>

class QUrl;

namespace Vreen {

class Client;
class AudioItemData;

class VK_SHARED_EXPORT AudioItem
{
public:
    AudioItem();
    AudioItem(const AudioItem &);
    AudioItem &operator=(const AudioItem &);
    ~AudioItem();

    int id() const;
    void setId(int aid);
    int ownerId() const;
    void setOwnerId(int ownerId);
    QString artist() const;
    void setArtist(const QString &artist);
    QString title() const;
    void setTitle(const QString &title);
    qreal duration() const;
    void setDuration(qreal duration);
    QUrl url() const;
    void setUrl(const QUrl &url);
    int lyricsId() const;
    void setLyricsId(int lyricsId);
    int albumId() const;
    void setAlbumId(int albumId);
private:
    QSharedDataPointer<AudioItemData> data;
};
typedef QList<AudioItem> AudioItemList;

class AudioAlbumItemData;
class VK_SHARED_EXPORT AudioAlbumItem
{
public:
    AudioAlbumItem();
    AudioAlbumItem(const AudioAlbumItem &other);
    AudioAlbumItem &operator=(const AudioAlbumItem &other);
    ~AudioAlbumItem();

    int id() const;
    void setId(int id);
    int ownerId() const;
    void setOwnerId(int ownerId);
    QString title() const;
    void setTitle(const QString &title);
private:
    QSharedDataPointer<AudioAlbumItemData> data;
};
typedef QList<AudioAlbumItem> AudioAlbumItemList;

template<>
AudioItem Attachment::to(const Attachment &data);

} // namespace Vreen

Q_DECLARE_METATYPE(Vreen::AudioItem)
Q_DECLARE_METATYPE(Vreen::AudioItemList)
Q_DECLARE_METATYPE(Vreen::AudioAlbumItem)
Q_DECLARE_METATYPE(Vreen::AudioAlbumItemList)

#endif // VK_AUDIOITEM_H

