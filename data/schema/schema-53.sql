/* Jamendo's tracks used to come from a local copy of its catalogue, which is
   gone, so playlist items that pointed into it can't be played. */
DELETE FROM playlist_items WHERE type = 'Jamendo';

UPDATE playlists SET dynamic_playlist_type = NULL, dynamic_playlist_data = NULL,
  dynamic_playlist_backend = NULL
  WHERE dynamic_playlist_type = 'Jamendo' OR dynamic_playlist_backend = 'jamendo.songs';

UPDATE schema_version SET version=53;
