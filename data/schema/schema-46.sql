ALTER TABLE playlists ADD COLUMN is_favorite INTEGER NOT NULL DEFAULT 0;

UPDATE schema_version SET version=46;
