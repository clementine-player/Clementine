DELETE FROM %allsongstables_fts;

DROP TABLE %allsongstables_fts;

CREATE VIRTUAL TABLE %allsongstables_fts USING fts3( ftstitle, ftsalbum, ftsartist, ftsalbumartist,
  ftscomposer, ftsperformer, ftsgrouping, ftsgenre, ftscomment, ftsyear,
  tokenize=unicode
);

INSERT INTO %allsongstables_fts ( ROWID, ftstitle, ftsalbum, ftsartist, ftsalbumartist,
    ftscomposer, ftsperformer, ftsgrouping, ftsgenre, ftscomment, ftsyear)
  SELECT ROWID, title, album, artist, albumartist, composer, performer, grouping, genre, comment, year
  FROM %allsongstables;

UPDATE schema_version SET version=51;