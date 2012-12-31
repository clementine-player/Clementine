#ifndef REMOTEXMLTAGS_H
#define REMOTEXMLTAGS_H

#include <QString>

class RemoteXmlTags {
public:
    RemoteXmlTags();
    ~RemoteXmlTags();

    // Nodes
    const static QString ROOT; // The root node of every xml
    const static QString ACTION; // The Node where the action is specified
    const static QString SONG; // The Node for song metadata
    const static QString INFOS; // Info Node
    const static QString VERSION; // Holds the Version of Clementine

    // Playlist
    const static QString PLAYLIST; // Node for Playlist
    const static QString PLAYLIST_NAME;
    const static QString PLAYLIST_ITEMS; // How may items are in the playlist?
    const static QString PLAYLIST_ACTIVE; // Is the playlist active?

    // Attributes
    const static QString ID;
    const static QString INDEX; // Index on the Playlist
    const static QString CURRENT_ROW;

    // Actions
    const static QString CLIENT_CONNTECTED; // A client wants to connect
    const static QString CLIENT_DISCONNECT; // A Client wants to disconnect
    const static QString SEND_INFOS; // Sending general Infos to client (e.g. Version)
    const static QString SEND_METAINFOS; // Sends the metainfos of the current song
    const static QString SEND_PLAYLISTS; // Send the List of Playlists to client
    const static QString SEND_PLAYLIST_SONGS; // Send the list of Songs to client
    const static QString REQUEST_PLAYLISTS; // A client request for playlists
    const static QString REQUEST_PLAYLIST_SONGS; // A clients wants to get all the songs in a playlist
    const static QString SET_VOLUME;
    const static QString PLAY;
    const static QString PLAYPAUSE;
    const static QString PAUSE;
    const static QString STOP;
    const static QString NEXT;
    const static QString PREV;
    const static QString CHANGE_SONG;
    const static QString KEEP_ALIVE;


    // Tags in Song
    const static QString SONG_TITLE;
    const static QString SONG_ARTIST;
    const static QString SONG_ALBUM;
    const static QString SONG_ALBUMARTIST;
    const static QString SONG_LENGTH;
    const static QString SONG_GENRE;
    const static QString SONG_YEAR;
    const static QString SONG_TRACK;
    const static QString SONG_DISC;
    const static QString SONG_PLAYCOUNT;
    const static QString SONG_ART;
    const static QString VOLUME;

};

#endif // REMOTEXMLTAGS_H
