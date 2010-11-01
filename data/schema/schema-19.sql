ALTER TABLE %allsongstables ADD COLUMN score INTEGER NOT NULL DEFAULT 0;

UPDATE schema_version SET version=19;

