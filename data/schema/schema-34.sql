ALTER TABLE %allsongstables ADD COLUMN unavailable INTEGER DEFAULT 0;

DROP VIEW duplicated_songs;

CREATE VIEW duplicated_songs as
select artist dup_artist, album dup_album, title dup_title
  from songs as inner_songs
 where artist != ''
   and album != ''
   and title != ''
   and unavailable = 0
 group by artist, album , title
having count(*) > 1;

UPDATE schema_version SET version=34;
