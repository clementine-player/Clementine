ALTER TABLE playlists ADD COLUMN special_type TEXT;

UPDATE schema_version SET version=33;
