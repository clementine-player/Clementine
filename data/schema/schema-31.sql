UPDATE songs SET filename = "file://" || filename;

UPDATE schema_version SET version=31;
