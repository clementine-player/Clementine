CREATE TABLE subdirectories (
  directory INTEGER NOT NULL,
  path TEXT NOT NULL,
  mtime INTEGER NOT NULL
);

UPDATE schema_version SET version=6;

