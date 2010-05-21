ALTER TABLE playlists ADD COLUMN ui_order INTEGER NOT NULL DEFAULT 0;

UPDATE schema_version SET version=11;

