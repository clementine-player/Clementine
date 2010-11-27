ALTER TABLE playlists ADD COLUMN dynamic_playlist_backend TEXT DEFAULT "songs";

UPDATE schema_version SET version=23;
