CREATE TABLE seafile_songs(
  title TEXT,
  album TEXT,
  artist TEXT,
  albumartist TEXT,
  composer TEXT,
  track INTEGER,
  disc INTEGER,
  bpm REAL,
  year INTEGER,
  genre TEXT,
  comment TEXT,
  compilation INTEGER,

  length INTEGER,
  bitrate INTEGER,
  samplerate INTEGER,

  directory INTEGER NOT NULL,
  filename TEXT NOT NULL,
  mtime INTEGER NOT NULL,
  ctime INTEGER NOT NULL,
  filesize INTEGER NOT NULL,
  sampler INTEGER NOT NULL DEFAULT 0,
  art_automatic TEXT,
  art_manual TEXT,
  filetype INTEGER NOT NULL DEFAULT 0,
  playcount INTEGER NOT NULL DEFAULT 0,
  lastplayed INTEGER,
  rating INTEGER,
  forced_compilation_on INTEGER NOT NULL DEFAULT 0,
  forced_compilation_off INTEGER NOT NULL DEFAULT 0,
  effective_compilation NOT NULL DEFAULT 0,
  skipcount INTEGER NOT NULL DEFAULT 0,
  score INTEGER NOT NULL DEFAULT 0,
  beginning INTEGER NOT NULL DEFAULT 0,
  cue_path TEXT,
  unavailable INTEGER DEFAULT 0,
  effective_albumartist TEXT,
  etag TEXT
);

CREATE VIRTUAL TABLE seafile_songs_fts USING fts3 (
  ftstitle, ftsalbum, ftsartist, ftsalbumartist, ftscomposer, ftsgenre, ftscomment,
  tokenize=unicode
);

UPDATE schema_version SET version=45;
