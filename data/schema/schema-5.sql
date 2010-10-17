ALTER TABLE songs ADD COLUMN effective_compilation NOT NULL DEFAULT 0;

UPDATE songs SET effective_compilation = ((compilation OR sampler OR forced_compilation_on) AND NOT forced_compilation_off) + 0;

CREATE INDEX idx_comp_artist ON songs (effective_compilation, artist);

UPDATE schema_version SET version=5;

