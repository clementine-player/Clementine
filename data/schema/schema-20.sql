ALTER TABLE playlists ADD COLUMN dynamic_playlist_type TEXT;

ALTER TABLE playlists ADD COLUMN dynamic_playlist_data BLOB;

UPDATE schema_version SET version=20;

