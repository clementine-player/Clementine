ALTER TABLE songs ADD COLUMN forced_compilation_on INTEGER NOT NULL DEFAULT 0;

ALTER TABLE songs ADD COLUMN forced_compilation_off INTEGER NOT NULL DEFAULT 0;

UPDATE schema_version SET version=4;
