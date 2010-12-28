ALTER TABLE %allsongstables ADD COLUMN beginning INTEGER NOT NULL DEFAULT 0;

UPDATE schema_version SET version=24;
