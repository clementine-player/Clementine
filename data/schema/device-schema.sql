CREATE TABLE device_%deviceid_directories (
  path TEXT NOT NULL,
  subdirs INTEGER NOT NULL
);

CREATE TABLE device_%deviceid_subdirectories (
  directory INTEGER NOT NULL,
  path TEXT NOT NULL,
  mtime INTEGER NOT NULL
);

CREATE TABLE device_%deviceid_songs (
  /* Metadata from taglib */
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

  /* Information about the file on disk */
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
  beginning NOT NULL DEFAULT 0,

  cue_path TEXT,
  unavailable INTEGER DEFAULT 0,

  effective_albumartist TEXT,
  etag TEXT,

  performer TEXT,
  grouping TEXT,
  lyrics TEXT
);

CREATE INDEX idx_device_%deviceid_songs_album ON device_%deviceid_songs (album);

CREATE INDEX idx_device_%deviceid_songs_comp_artist ON device_%deviceid_songs (effective_compilation, artist);

CREATE VIRTUAL TABLE device_%deviceid_fts USING fts3(
  ftstitle, ftsalbum, ftsartist, ftsalbumartist, ftscomposer, ftsperformer, ftsgrouping, ftsgenre, ftscomment,
  tokenize=unicode
);

UPDATE devices SET schema_version=0 WHERE ROWID=%deviceid;

