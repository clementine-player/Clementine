ALTER TABLE %allsongstables ADD COLUMN cue_path TEXT;

UPDATE schema_version SET version=25;
