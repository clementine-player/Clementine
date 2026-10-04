/* Spotify's search results were kept here, until Spotify support was removed.
   Being songs tables, they'd otherwise go on being updated with all the rest. */
DROP TABLE IF EXISTS spotify_search_songs_fts;

DROP TABLE IF EXISTS spotify_search_songs;

UPDATE schema_version SET version=54;
