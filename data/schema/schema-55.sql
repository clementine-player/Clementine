/* Database::RebuildFtsTablesWithFts5() replaces every FTS3 table, which used
   Clementine's own tokenizer, with an FTS5 one using SQLite's unicode61, and
   fills it from its songs table. */

UPDATE schema_version SET version=55;
