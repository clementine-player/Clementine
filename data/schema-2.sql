ALTER TABLE songs ADD COLUMN art_automatic TEXT;

ALTER TABLE songs ADD COLUMN art_manual TEXT;

UPDATE schema_version SET version=2;
