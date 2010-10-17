ALTER TABLE devices ADD COLUMN transcode_mode NOT NULL DEFAULT 3;

ALTER TABLE devices ADD COLUMN transcode_format NOT NULL DEFAULT 5;

UPDATE schema_version SET version=17;

