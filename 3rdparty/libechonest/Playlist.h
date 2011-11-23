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


#ifndef ECHONEST_PLAYLIST_H
#define ECHONEST_PLAYLIST_H

#include "echonest_export.h"
#include "Song.h"

#include <QSharedData>
#include <QDebug>
#include "Artist.h"
#include <QtCore/QString>
#include "Catalog.h"

class QNetworkReply;
class DynamicPlaylistData;

namespace Echonest{

    typedef struct {
        qreal served_time;
        QByteArray artist_id;
        QByteArray id;
        QString artist_name;
        QString title;
        int rating;
    } SessionItem;

    typedef struct {
        TermList terms;
        SongList seed_songs;
//         description .. what data is in here?
        Artists banned_artists;
        QVector< QString > rules;
        QByteArray session_id;
        Artists seeds;
        QVector< SessionItem > skipped_songs;
        QVector< SessionItem > banned_songs;
        QString playlist_type;
        Catalogs seed_catalogs;
        QVector< SessionItem > rated_songs;
        QVector< SessionItem > history;
    } SessionInfo;

    /**
     * This encapsulates an Echo Nest dynamic playlist. It contains a playlist ID and
     *  the current song, and can fetch the next song.
     *
     *  See http://developer.echonest.com/docs/v4/playlist.html#dynamic
     *   for more information
     */
    class ECHONEST_EXPORT DynamicPlaylist
    {
    public:
        /**
         * The types of playlist that can be generated. Artist plays songs for the given artist,
         *  ArtistRadio takes into account similar artists, and ArtistDescription plays songs matching
         *  the given description.
         */
        enum ArtistTypeEnum {
            ArtistType,
            ArtistRadioType,
            ArtistDescriptionType,
            CatalogType,
            CatalogRadioType,
            SongRadioType
        };

        /**
         * Different ways to sort a generated playlist
         */
        enum SortingType {
            SortTempoAscending,
            SortTempoDescending,
            SortDurationAscending,
            SortDurationDescending,
            SortArtistFamiliarityAscending,
            SortArtistFamiliarityDescending,
            SortArtistHotttnessAscending,
            SortArtistHotttnessDescending,
            SortSongHotttnesssAscending,
            SortSongHotttnesssDescending,
            SortLatitudeAscending,
            SortLatitudeDescending,
            SortLongitudeAscending,
            SortLongitudeDescending,
            SortModeAscending,
            SortModeDescending,
            SortKeyAscending,
            SortKeyDescending,
            SortLoudnessAscending,
            SortLoudnessDescending,
            SortEnergyAscending,
            SortEnergyDescending,
            SortDanceabilityAscending,
            SortDanceabilityDescending
        };

        /**
         * Different ways of picking artists in Artist radios.
         */
        enum ArtistPick {
            PickSongHotttnesssAscending,
            PickTempoAscending,
            PickDurationAscending,
            PickLoudnessAscending,
            PickModeAscending,
            PickKeyAscending,
            PickSongHotttnesssDescending,
            PickTempoDescending,
            PickDurationDescending,
            PickLoudnessDescending,
            PickModeDescending,
            PickKeyDescending
        };

        /**
         * The various parameters that can be passed to the playlist building
         *  functions.
         */
        enum PlaylistParam {
            Type, /// The type of playlist to generate. Value is the DynamicPlaylist::ArtistTypeEnum enum
            Format, /// Either xml (default) or xspf. If the result is xspf, the raw xspf playlist is returned, else the xml is parsed and exposed programmatically. If using XSPF, you must specify a catalog, the tracks bucket, and limit = true
            Pick,   /// How the artists are picked for each artist in ArtistType playlists. Value is Playlist::ArtistPick enum value.
            Variety, /// 0 < variety < 1        The maximum variety of artists to be represented in the playlist. A higher number will allow for more variety in the artists.
            ArtistId, ///  ID(s) of seed artist(s) for the playlist
            Artist, /// Artist names of seeds for playlist
            ArtistSeedCatalog, /// ID of seed artist catalog for the playlist
            SourceCatalog, /// ID of catalog (artist or song) for catalog type playlists
            SongId, /// IDs of seed songs for the playlist
            Description, /// Textual description for sort of songs that can be included in the playlist
            Results, /// 0-100, how many sonsg to include in the playlist, default 15
            MaxTempo, /// 0.0 < tempo < 500.0 (BPM)       The maximum tempo for any included songs
            MinTempo, /// 0.0 < tempo < 500.0 (BPM)       the minimum tempo for any included songs
            MaxDuration, /// 0.0 < duration < 3600.0 (seconds)       the maximum duration of any song on the playlist
            MinDuration, /// 0.0 < duration < 3600.0 (seconds)       the minimum duration of any song on the playlist
            MaxLoudness, /// -100.0 < loudness < 100.0 (dB)  the maximum loudness of any song on the playlist
            MinLoudness, /// -100.0 < loudness < 100.0 (dB)  the minimum loudness of any song on the playlist
            MinDanceability, /// 0 < danceability < 1  a measure of the minimum danceability of the song
            MaxDanceability, /// 0 < danceability < 1  a measure of the maximum danceability of the song
            MinEnergy, /// 0 < danceability < 1  a measure of the maximum energy of the song
            MaxEnergy, /// 0 < danceability < 1  a measure of the maximum energy of the song
            ArtistMaxFamiliarity, ///  0.0 < familiarity < 1.0 the maximum artist familiarity for songs in the playlist
            ArtistMinFamiliarity, ///  0.0 < familiarity < 1.0 the minimum artist familiarity for songs in the playlist
            ArtistMaxHotttnesss, ///  0.0 < hotttnesss < 1.0  the maximum hotttnesss for artists in the playlist
            ArtistMinHotttnesss, ///  0.0 < hotttnesss < 1.0  the maximum hotttnesss for artists in the playlist
            SongMaxHotttnesss, ///  0.0 < hotttnesss < 1.0  the maximum hotttnesss for songs in the playlist
            SongMinHotttnesss, ///  0.0 < hotttnesss < 1.0  the maximum hotttnesss for songs in the playlist
            ArtistMinLongitude, /// -180.0 < longitude < 180.0      the minimum longitude for the location of artists in the playlist
            ArtistMaxLongitude, /// -180.0 < longitude < 180.0      the maximum longitude for the location of artists in the playlist
            ArtistMinLatitude,  /// -90.0 < latitude < 90.0 the minimum latitude for the location of artists in the playlist
            ArtistMaxLatitude, /// -90.0 < latitude < 90.0 the maximum latitude for the location of artists in the playlist
            Mode, /// (minor, major) 0, 1     the mode of songs in the playlist
            Key, /// (c, c-sharp, d, e-flat, e, f, f-sharp, g, a-flat, a, b-flat, b) 0 - 11  the key of songs in the playlist
            SongInformation, /// what sort of song information should be returned. Should be an Echonest::SongInformation object
            Sort, /// SortingType enum, the type of sorting to use,
            Limit, /// true, false    if true songs will be limited to those that appear in the catalog specified by the id: bucket
            Audio, /// true, false,  if true songs will be limited to those that have associated audio
            DMCA, /// true, false    Only valid for dynamic playlists. Sets if playlist will follow DMCA rules (see web api doc for details)
            ChainXSPF, /// true, false    If true, returns an xspf for this dynamic playlist with 2 items. The second item will be a link to the API call for the next track in the chain. Please note that this sidesteps libechonest's handling of the tracks.
            Mood, /// A mood to limit this playlist to, for example "happy" or "sad". Multiples of this param are okay. See the method Artist::listTerms for details on what moods are currently available
            Style, /// A style to limit this playlist to, for example "happy" or "sad". Multiples of this param are okay. See the method Artist::listTerms for details on what styles are currently available
            Adventurousness
        };

        typedef QPair< PlaylistParam, QVariant > PlaylistParamData;
        typedef QVector< PlaylistParamData > PlaylistParams;

        /**
         * The various controls for a dynamic playlist.
         *
         * Please see The Echo Nest API documentation for more information
         */
        enum DynamicControlItem {
          Steer = 0,
          SteerDescription,
          Rating,
          Ban
        };
        typedef QPair< DynamicControlItem, QString > DynamicControl;
        typedef QVector< DynamicControl > DynamicControls;

        DynamicPlaylist();
        virtual ~DynamicPlaylist();
        DynamicPlaylist( const DynamicPlaylist& other );
        DynamicPlaylist& operator=( const DynamicPlaylist& playlist );

        /**
         * Start a dynamic playlist with the given parameters.
         *  Once the QNetworkReply has finished, pass it to parseStart()
         *  and the inital song will be populated and returned. The sessionId(), currentSong(),
         *  and fetchNextSong() methods will then be useful.
         */
        QNetworkReply* start( const PlaylistParams& params ) const;
        Song parseStart( QNetworkReply* ) throw( ParseError );

        /**
         * The session id of this dynamic playlist. If the playlist has ended, or has not been started,
         *  the result is empty.
         *
         */
        QByteArray sessionId() const;
        void setSessionId( const QByteArray& id );

        /**
         * The current song of this dynamic playlist. Once this song has been played,
         *  or whenever is desired, call fetchNextSong() to get the next song.
         */
        Song currentSong() const;
        void setCurrentSong( const Song& song );

        /**
         * Queries The Echo Nest for the next playable song in this
         *  dynamic playlist.
         *
         * Once the query has emitted the finished() signal, pass it to parseNextSong(), which will
         *  return the new song to play. It will also set the current song to the newly parsed song.
         *
         * If the playlist has no more songs, the returned song object will be have no name nor id.
         *
         * @param rating The rating for the song that was just played. Ranges from 1 (lowest) to 5 (highest)
         * @param controls The controls to apply when fetching the next track.
         *
         */
        QNetworkReply* fetchNextSong( int rating = -1 ) const;
        QNetworkReply* fetchNextSong( const DynamicControls& controls ) const;
        Song parseNextSong( QNetworkReply* reply );

        /**
         * Returns a description of this dynamic playlist session
         */
        QNetworkReply* fetchSessionInfo() const;
        SessionInfo parseSessionInfo( QNetworkReply* reply ) throw( ParseError );

        /**
         * Generate a static playlist, according to the desired criteria. Use parseXSPFPlaylist if
         *  you pass format=xspf to \c staticPlaylist().
         */
        static QNetworkReply* staticPlaylist( const PlaylistParams& params );
        static SongList parseStaticPlaylist( QNetworkReply* reply ) throw( ParseError );

        /**
         * Parse an xspf playlist. Returns the full xspf content with no modifications.
         */
        static QByteArray parseXSPFPlaylist( QNetworkReply* reply ) throw( ParseError );

    private:
        static QByteArray playlistParamToString( PlaylistParam param );
        static QNetworkReply* generateInternal( const PlaylistParams& params, const QByteArray& type );
        static QByteArray playlistSortToString(SortingType sorting);
        static QByteArray playlistArtistPickToString(ArtistPick pick);
        static QByteArray dynamicControlToString(DynamicControlItem control);

        QSharedDataPointer<DynamicPlaylistData> d;
    };

    ECHONEST_EXPORT QDebug operator<<(QDebug d, const Echonest::DynamicPlaylist& playlist);


} // namespace


Q_DECLARE_METATYPE( Echonest::DynamicPlaylist )

#endif
