CREATE TABLE playlists (
  name TEXT NOT NULL
);

CREATE TABLE playlist_items (
  playlist INTEGER NOT NULL,
  type TEXT NOT NULL, /* Library, Stream, File, or Radio */

  /* Library */
  library_id INTEGER,

  /* Stream, File or Radio */
  url TEXT,

  /* Stream or Radio */
  title TEXT,
  artist TEXT,
  album TEXT,
  length INTEGER,

  /* Radio */
  radio_service TEXT
);

UPDATE schema_version SET version=7;

