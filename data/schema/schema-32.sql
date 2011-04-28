UPDATE magnatune_songs SET filename = "magnatune://" || substr(filename, 8);

UPDATE schema_version SET version=32;
