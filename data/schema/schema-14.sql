CREATE TABLE devices (
  unique_id TEXT NOT NULL,
  friendly_name TEXT,
  size INTEGER,
  icon TEXT
);

UPDATE schema_version SET version=14;
