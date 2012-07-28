CREATE TABLE podcasts (
  url TEXT,
  title TEXT,
  description TEXT,
  copyright TEXT,
  link TEXT,
  image_url_large TEXT,
  image_url_small TEXT,
  author TEXT,
  owner_name TEXT,
  owner_email TEXT,

  last_updated INTEGER,
  last_update_error TEXT,

  extra BLOB
);

CREATE TABLE podcast_episodes (
  podcast_id INTEGER,

  title TEXT,
  description TEXT,
  author TEXT,
  publication_date INTEGER,
  duration_secs INTEGER,
  url TEXT,

  listened BOOLEAN,
  listened_date INTEGER,
  downloaded BOOLEAN,
  local_url TEXT,

  extra BLOB
);

CREATE INDEX podcast_idx_url ON podcasts(url);

CREATE INDEX podcast_episodes_idx_podcast_id ON podcast_episodes(podcast_id);

CREATE INDEX podcast_episodes_idx_url ON podcast_episodes(url);

CREATE INDEX podcast_episodes_idx_local_url ON podcast_episodes(local_url);

UPDATE schema_version SET version=37;
