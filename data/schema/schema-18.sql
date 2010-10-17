ALTER TABLE %allsongstables ADD COLUMN skipcount INTEGER NOT NULL DEFAULT 0;

UPDATE schema_version SET version=18;

