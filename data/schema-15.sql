ALTER TABLE devices ADD COLUMN schema_version INTEGER NOT NULL DEFAULT 0;

UPDATE schema_version SET version=15;

