ALTER TABLE %allsongstables ADD COLUMN originalyear INTEGER;

ALTER TABLE %allsongstables ADD COLUMN effective_originalyear INTEGER;

UPDATE songs SET originalyear = -1;

UPDATE songs SET effective_originalyear = -1;

UPDATE schema_version SET version=50;
