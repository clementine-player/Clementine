CREATE VIRTUAL TABLE playlist_items_fts USING fts3(
  ftstitle, ftsalbum, ftsartist, ftsalbumartist, ftscomposer, ftsgenre, ftscomment,
  tokenize=unicode
);

DELETE FROM %allsongstables_fts;

DROP TABLE %allsongstables_fts;

ALTER TABLE %allsongstables ADD COLUMN performer TEXT;

ALTER TABLE %allsongstables ADD COLUMN grouping TEXT;

CREATE VIRTUAL TABLE %allsongstables_fts USING fts3(
  ftstitle, ftsalbum, ftsartist, ftsalbumartist, ftscomposer, ftsperformer, ftsgrouping, ftsgenre, ftscomment,
  tokenize=unicode
);

INSERT INTO %allsongstables_fts (ROWID, ftstitle, ftsalbum, ftsartist, ftsalbumartist, ftscomposer, ftsperformer, ftsgrouping, ftsgenre, ftscomment)
    SELECT ROWID, title, album, artist, albumartist, composer, performer, grouping, genre, comment
    FROM %allsongstables;

UPDATE schema_version SET version=45;

