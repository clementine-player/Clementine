/****************************************************************************************
 * Copyright (c) 2010 Leo Franchi <lfranchi@kde.org>                                    *
 *                                                                                      *
 * This program is free software; you can redistribute it and/or modify it under        *
 * the terms of the GNU General Public License as published by the Free Software        *
 * Foundation; either version 2 of the License, or (at your option) any later           *
 * version.                                                                             *
 *                                                                                      *
 * This program is distributed in the hope that it will be useful, but WITHOUT ANY      *
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A      *
 * PARTICULAR PURPOSE. See the GNU General Public License for more details.             *
 *                                                                                      *
 * You should have received a copy of the GNU General Public License along with         *
 * this program.  If not, see <http://www.gnu.org/licenses/>.                           *
 ****************************************************************************************/

#include "ArtistTypes.h"

#include "ArtistTypes_p.h"

#include <QUrl>
#include <QDateTime>

Echonest::ArtistImage::ArtistImage() : d( new ArtistImageData )
{

}

Echonest::ArtistImage::ArtistImage(const Echonest::ArtistImage& other)
{
    d = other.d;
}

Echonest::ArtistImage& Echonest::ArtistImage::operator=(const Echonest::ArtistImage& img)
{
    d = img.d;
    return *this;
}

Echonest::ArtistImage::~ArtistImage()
{

}


QUrl Echonest::ArtistImage::url() const
{
    return d->url;
}

void Echonest::ArtistImage::setUrl(const QUrl& url)
{
    d->url = url;
}

Echonest::License Echonest::ArtistImage::license() const
{
    return d->license;
}

void Echonest::ArtistImage::setLicense(const Echonest::License& license)
{
    d->license = license;
}

Echonest::AudioFile::AudioFile() : d( new AudioFileData )
{

}

Echonest::AudioFile::AudioFile(const Echonest::AudioFile& other)
{
    d = other.d;
}

Echonest::AudioFile& Echonest::AudioFile::operator=(const Echonest::AudioFile& artist)
{
    d = artist.d;
    return *this;
}

Echonest::AudioFile::~AudioFile()
{

}

void Echonest::AudioFile::setTitle(const QString& title)
{
    d->title = title;
}

QString Echonest::AudioFile::title() const
{
    return d->title;
}


QString Echonest::AudioFile::artist() const
{
    return d->artist;
}

void Echonest::AudioFile::setArtist(const QString& artist)
{
    d->artist = artist;
}

QDateTime Echonest::AudioFile::date() const
{
    return d->date;
}

void Echonest::AudioFile::setDate(const QDateTime& date)
{
    d->date = date;
}

QString Echonest::AudioFile::release() const
{
    return d->release;
}

void Echonest::AudioFile::setRelease(const QString& release)
{
    d->release = release;
}

QByteArray Echonest::AudioFile::id() const
{
    return d->id;
}

void Echonest::AudioFile::setId(const QByteArray& id)
{
    d->id = id;
}

qreal Echonest::AudioFile::length() const
{
    return d->length;
}

void Echonest::AudioFile::setLength(qreal length)
{
    d->length = length;
}

QUrl Echonest::AudioFile::link() const
{
    return d->link;
}

void Echonest::AudioFile::setLink(const QUrl& url)
{
    d->link = url;
}

QUrl Echonest::AudioFile::url() const
{
    return d->url;
}

void Echonest::AudioFile::setUrl(const QUrl& url)
{
    d->url = url;
}

Echonest::Biography::Biography() : d( new BiographyData )
{

}

Echonest::Biography::Biography(const Echonest::Biography& other)
{
    d = other.d;
}

Echonest::Biography& Echonest::Biography::operator=(const Echonest::Biography& biblio)
{
    d = biblio.d;
    return *this;
}

Echonest::Biography::~Biography()
{

}

Echonest::License Echonest::Biography::license() const
{
    return d->license;
}

void Echonest::Biography::setLicense(const Echonest::License& license)
{
    d->license = license;
}

QString Echonest::Biography::site() const
{
    return d->site;
}

void Echonest::Biography::setSite(const QString& site)
{
    d->site = site;
}

QString Echonest::Biography::text() const
{
    return d->text;
}

void Echonest::Biography::setText(const QString& text)
{
    d->text = text;
}

QUrl Echonest::Biography::url() const
{
    return d->url;
}

void Echonest::Biography::setUrl(const QUrl& url)
{
    d->url = url;
}

Echonest::Blog::Blog() : d( new BlogData )
{

}

Echonest::Blog::Blog(const Echonest::Blog& other) : d( other.d )
{

}

Echonest::Blog& Echonest::Blog::operator=(const Echonest::Blog& other)
{
    d = other.d;
    return *this;
}

Echonest::Blog::~Blog()
{

}


QDateTime Echonest::Blog::dateFound() const
{
    return d->date_found;
}

void Echonest::Blog::setDateFound(const QDateTime& date)
{
    d->date_found = date;
}

QDateTime Echonest::Blog::datePosted() const
{
    return d->date_posted;
}

void Echonest::Blog::setDatePosted(const QDateTime& date)
{
    d->date_posted = date;
}

QByteArray Echonest::Blog::id() const
{
    return d->id;
}

void Echonest::Blog::setId(const QByteArray& id)
{
    d->id = id;
}

QString Echonest::Blog::name() const
{
    return d->name;
}

void Echonest::Blog::setName(const QString& name)
{
    d->name = name;
}

QString Echonest::Blog::summary() const
{
    return d->summary;
}

void Echonest::Blog::setSummary(const QString& text)
{
    d->summary = text;
}

QUrl Echonest::Blog::url() const
{
    return d->url;
}

void Echonest::Blog::setUrl(const QUrl& url)
{
    d->url = url;
}

Echonest::Review::Review() : d( new ReviewData )
{
    
}

Echonest::Review::Review(const Echonest::Review& other) : d( other.d )
{

}

Echonest::Review& Echonest::Review::operator=(const Echonest::Review& other)
{
    d = other.d;
    return *this;
}

Echonest::Review::~Review()
{

}


QDateTime Echonest::Review::dateFound() const
{
    return d->date_found;
}

void Echonest::Review::setDateFound(const QDateTime& date)
{
    d->date_found = date;
}

QDateTime Echonest::Review::dateReviewed() const
{
    return d->date_reviewed;
}

void Echonest::Review::setDateReviewed(const QDateTime& date)
{
    d->date_reviewed = date;
}

QByteArray Echonest::Review::id() const
{
    return d->id;
}

void Echonest::Review::setId(const QByteArray& id)
{
    d->id = id;
}

QUrl Echonest::Review::imageUrl() const
{
    return d->image_url;
}

void Echonest::Review::setImageUrl(const QUrl& imageUrl)
{
    d->image_url = imageUrl;
}

QString Echonest::Review::name() const
{
    return d->name;
}

void Echonest::Review::setName(const QString& name)
{
    d->name = name;
}

QString Echonest::Review::release() const
{
    return d->release;
}

void Echonest::Review::setRelease(const QString& release)
{
    d->release = release;
}

QString Echonest::Review::summary() const
{
    return d->summary;
}

void Echonest::Review::setSummary(const QString& text)
{
    d->summary = text;
}

QUrl Echonest::Review::url() const
{
    return d->url;
}

void Echonest::Review::setUrl(const QUrl& url)
{
    d->url = url;
}

Echonest::Term::Term() : d( new TermData )
{

}

Echonest::Term::Term(const Echonest::Term& other) : d( other.d )
{

}

Echonest::Term& Echonest::Term::operator=(const Echonest::Term& other)
{
    d = other.d;
    return *this;
}

Echonest::Term::~Term()
{

}

qreal Echonest::Term::frequency() const
{
    return d->frequency;
}

void Echonest::Term::setFrequency(qreal freq)
{
    d->frequency = freq;
}

QString Echonest::Term::name() const
{
    return d->name;
}

void Echonest::Term::setName(const QString& name)
{
    d->name = name;
}

qreal Echonest::Term::weight() const
{
    return d->weight;
}

void Echonest::Term::setWeight(qreal weight)
{
    d->weight = weight;
}

Echonest::Video::Video() : d( new VideoData )
{

}

Echonest::Video::Video(const Echonest::Video& other) : d( other.d )
{

}

Echonest::Video& Echonest::Video::operator=(const Echonest::Video& other)
{
    d = other.d;
    return *this;
}

Echonest::Video::~Video()
{

}

QDateTime Echonest::Video::dateFound() const
{
    return d->date_found;
}

void Echonest::Video::setDateFound(const QDateTime& date)
{
    d->date_found = date;
}

QByteArray Echonest::Video::id() const
{
    return d->id;
}

void Echonest::Video::setId(const QByteArray& id)
{
    d->id = id;
}

QUrl Echonest::Video::imageUrl() const
{
    return d->image_url;
}

void Echonest::Video::setImageUrl(const QUrl& imageUrl)
{
    d->image_url = imageUrl;
}

QString Echonest::Video::site() const
{
    return d->site;
}

void Echonest::Video::setSite(const QString& site)
{
    d->site = site;
}

QString Echonest::Video::title() const
{
    return d->title;
}

void Echonest::Video::setTitle(const QString& title)
{
    d->title = title;
}

QUrl Echonest::Video::url() const
{
    return d->url;
}

void Echonest::Video::setUrl(const QUrl& url)
{
    d->url = url;
}

QDebug Echonest::operator<<(QDebug d, const Echonest::AudioFile& audio)
{
    return d.maybeSpace() << QString::fromLatin1( "AudioFile [%1, %2, %3, %4]" ).arg( audio.title() ) .arg( audio.artist() ).arg( audio.release() ).arg( audio.url().toString() );
}
QDebug Echonest::operator<<(QDebug d, const Echonest::Biography& biblio)
{
    return d.maybeSpace() << QString::fromLatin1( "Bibliography [%1, %2, %3, %4]" ).arg( biblio.site() ).arg( biblio.url().toString() ).arg( biblio.license().type ).arg( biblio.text().left( 100 ) );
}
QDebug Echonest::operator<<(QDebug d, const Echonest::Blog& blog)
{
    return d.maybeSpace() << QString::fromLatin1( "Blog [%1, %2, %3, %4, %5, %6]" ).arg( blog.name() ).arg( blog.datePosted().toString() ).arg( blog.dateFound().toString() ).arg( blog.url().toString() ).arg( QLatin1String( blog.id() ) ).arg( blog.summary().left( 100 ) );
}
QDebug Echonest::operator<<(QDebug d, const Echonest::ArtistImage& img)
{
    return d.maybeSpace() << QString::fromLatin1( "ArtistImage [%1, %2]" ).arg( img.url().toString() ).arg( img.license().type );
}
QDebug Echonest::operator<<(QDebug d, const Echonest::Review& review)
{
    return d.maybeSpace() << QString::fromLatin1( "Review [%1, %2, %3]" ).arg( review.name() ).arg( review.release() ).arg( review.summary().left( 100 ) );
}
QDebug Echonest::operator<<(QDebug d, const Echonest::Term& term)
{
    return d.maybeSpace() << QString::fromLatin1( "Term [%1, %2, %3]" ).arg( term.name() ).arg( term.frequency() ).arg( term.weight() );
}
QDebug Echonest::operator<<(QDebug d, const Echonest::Video& video)
{
    return d.maybeSpace() << QString::fromLatin1( "Video [%1, %2, %3]" ).arg( video.title() ).arg( video.site() ).arg( video.url().toString() );
}
