ALTER TABLE playlists ADD COLUMN ui_path TEXT;

UPDATE schema_version SET version=39;
