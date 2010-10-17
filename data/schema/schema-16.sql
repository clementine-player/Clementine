UPDATE songs SET title = "" WHERE title IS NULL;

UPDATE songs SET album = "" WHERE album IS NULL;

UPDATE songs SET artist = "" WHERE artist IS NULL;

UPDATE songs SET albumartist = "" WHERE albumartist IS NULL;

UPDATE songs SET composer = "" WHERE composer IS NULL;

UPDATE songs SET genre = "" WHERE genre IS NULL;

UPDATE songs SET track = -1 WHERE track IS NULL;

UPDATE songs SET disc = -1 WHERE disc IS NULL;

UPDATE songs SET bpm = -1 WHERE bpm IS NULL;

UPDATE songs SET year = -1 WHERE year IS NULL;

UPDATE schema_version SET version=16;

