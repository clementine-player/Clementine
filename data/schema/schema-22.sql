CREATE TABLE icecast_stations (
  name TEXT,
  url TEXT,
  mime_type TEXT,
  bitrate INTEGER,
  channels INTEGER,
  samplerate INTEGER,
  genre TEXT
);

CREATE INDEX idx_icecast_genres ON icecast_stations(genre);

CREATE INDEX idx_icecast_name ON icecast_stations(name);

UPDATE schema_version SET version=22;
