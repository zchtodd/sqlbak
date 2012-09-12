sqlbak
======

Backup directories of SQLite3 databases.  Uses the SQLite3 backup C API to ensure that backups can be done reliably without any process downtime.  Should a database file be corrupted or locked by a rogue process, sqlbak will provide as much information as possible. 