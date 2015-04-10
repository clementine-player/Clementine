ALTER TABLE %allsongstables ADD COLUMN lyrics TEXT;

UPDATE schema_version SET version=49;
