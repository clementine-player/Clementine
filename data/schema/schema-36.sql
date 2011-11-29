ALTER TABLE %allsongstables ADD COLUMN effective_albumartist TEXT;

UPDATE songs SET effective_albumartist = albumartist;

UPDATE songs SET effective_albumartist = artist WHERE effective_albumartist = "";

UPDATE schema_version SET version=36;
