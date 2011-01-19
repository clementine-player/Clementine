ALTER TABLE playlist_items ADD COLUMN beginning INTEGER;

ALTER TABLE playlist_items ADD COLUMN cue_path TEXT;

UPDATE schema_version SET version=26;
