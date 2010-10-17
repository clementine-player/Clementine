ALTER TABLE songs ADD COLUMN sampler INTEGER NOT NULL DEFAULT 0;

INSERT INTO schema_version (version) VALUES (0);

UPDATE schema_version SET version=1;
