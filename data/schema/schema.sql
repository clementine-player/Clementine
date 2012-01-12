CREATE TABLE directories (
  path TEXT NOT NULL,
  subdirs INTEGER NOT NULL
);


CREATE TABLE songs (
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
  filesize INTEGER NOT NULL
);


CREATE TABLE schema_version (
  version INTEGER NOT NULL
);

INSERT INTO schema_version (version) VALUES (0);
