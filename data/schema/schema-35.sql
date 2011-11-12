CREATE INDEX idx_filename ON songs (filename);

UPDATE schema_version SET version=35;
