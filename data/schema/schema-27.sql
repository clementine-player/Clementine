CREATE INDEX idx_title ON songs (title);

CREATE VIEW duplicated_songs as 
select artist dup_artist, album dup_album, title dup_title 
  from songs as inner_songs 
 where artist != '' 
   and album != '' 
   and title != '' 
 group by artist, album , title 
having count(*) > 1;

UPDATE schema_version SET version=27;
