/* Schema should be kept identical to the "songs" table, even though most of
   it isn't used by jamendo */
CREATE TABLE jamendo.songs (
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
  skipcount NOT NULL DEFAULT 0,
  score NOT NULL DEFAULT 0,
  beginning NOT NULL DEFAULT 0,

  cue_path TEXT,
  unavailable INTEGER DEFAULT 0,

  effective_albumartist TEXT,
  etag TEXT,

  performer TEXT,
  grouping TEXT,
  lyrics TEXT,

  originalyear INTEGER,
  effective_originalyear INTEGER
);

CREATE VIRTUAL TABLE jamendo.songs_fts USING fts3(
  ftstitle, ftsalbum, ftsartist, ftsalbumartist, ftscomposer, ftsperformer, ftsgrouping, ftsgenre, ftscomment, ftsyear,
  tokenize=unicode
);

CREATE INDEX jamendo.idx_jamendo_comp_artist ON songs (effective_compilation, artist);

CREATE TABLE jamendo.track_ids (
  songs_row_id INTEGER PRIMARY KEY,
  track_id INTEGER
);

CREATE INDEX jamendo.idx_jamendo_track_id ON track_ids(track_id);

