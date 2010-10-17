CREATE VIRTUAL TABLE songs_fts USING fts3(
  ftstitle, ftsalbum, ftsartist, ftsalbumartist, ftscomposer, ftsgenre, ftscomment,
  tokenize=unicode
);

CREATE VIRTUAL TABLE magnatune_songs_fts USING fts3(
  ftstitle, ftsalbum, ftsartist, ftsalbumartist, ftscomposer, ftsgenre, ftscomment,
  tokenize=unicode
);

INSERT INTO songs_fts (ROWID, ftstitle, ftsalbum, ftsartist, ftsalbumartist, ftscomposer, ftsgenre, ftscomment)
    SELECT ROWID, title, album, artist, albumartist, composer, genre, comment
    FROM songs;

INSERT INTO magnatune_songs_fts (ROWID, ftstitle, ftsalbum, ftsartist, ftsalbumartist, ftscomposer, ftsgenre, ftscomment)
    SELECT ROWID, title, album, artist, albumartist, composer, genre, comment
    FROM magnatune_songs;

CREATE INDEX idx_album ON songs (album);

UPDATE schema_version SET version=13;

