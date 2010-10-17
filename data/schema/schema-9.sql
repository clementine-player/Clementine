CREATE INDEX idx_magnatune_comp_artist ON magnatune_songs (effective_compilation, artist);

UPDATE schema_version SET version=9;

