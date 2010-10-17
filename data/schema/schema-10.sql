ALTER TABLE playlists ADD COLUMN last_played INTEGER NOT NULL DEFAULT -1;

UPDATE schema_version SET version=10;

