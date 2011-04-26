ALTER TABLE playlist_items ADD COLUMN albumartist TEXT;

ALTER TABLE playlist_items ADD COLUMN composer TEXT;

ALTER TABLE playlist_items ADD COLUMN track INTEGER;

ALTER TABLE playlist_items ADD COLUMN disc INTEGER;

ALTER TABLE playlist_items ADD COLUMN bpm REAL;

ALTER TABLE playlist_items ADD COLUMN year INTEGER;

ALTER TABLE playlist_items ADD COLUMN genre TEXT;

ALTER TABLE playlist_items ADD COLUMN comment TEXT;

ALTER TABLE playlist_items ADD COLUMN compilation INTEGER;

ALTER TABLE playlist_items ADD COLUMN bitrate INTEGER;

ALTER TABLE playlist_items ADD COLUMN samplerate INTEGER;

ALTER TABLE playlist_items ADD COLUMN directory INTEGER;

ALTER TABLE playlist_items ADD COLUMN filename TEXT;

ALTER TABLE playlist_items ADD COLUMN mtime INTEGER;

ALTER TABLE playlist_items ADD COLUMN ctime INTEGER;

ALTER TABLE playlist_items ADD COLUMN filesize INTEGER;

ALTER TABLE playlist_items ADD COLUMN sampler INTEGER NOT NULL DEFAULT 0;

ALTER TABLE playlist_items ADD COLUMN art_automatic TEXT;

ALTER TABLE playlist_items ADD COLUMN art_manual TEXT;

ALTER TABLE playlist_items ADD COLUMN filetype INTEGER NOT NULL DEFAULT 0;

ALTER TABLE playlist_items ADD COLUMN playcount INTEGER NOT NULL DEFAULT 0;

ALTER TABLE playlist_items ADD COLUMN lastplayed INTEGER;

ALTER TABLE playlist_items ADD COLUMN rating INTEGER;

ALTER TABLE playlist_items ADD COLUMN forced_compilation_on INTEGER NOT NULL DEFAULT 0;

ALTER TABLE playlist_items ADD COLUMN forced_compilation_off INTEGER NOT NULL DEFAULT 0;

ALTER TABLE playlist_items ADD COLUMN effective_compilation NOT NULL DEFAULT 0;

ALTER TABLE playlist_items ADD COLUMN skipcount INTEGER NOT NULL DEFAULT 0;

ALTER TABLE playlist_items ADD COLUMN score INTEGER NOT NULL DEFAULT 0;

UPDATE playlist_items SET filename = url;

UPDATE schema_version SET version=29;
