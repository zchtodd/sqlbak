sqlbak
======

Backup directories of SQLite3 databases.  Uses the SQLite3 backup C API to ensure that backups can be done reliably without any process downtime.  Should a database file be corrupted or locked by a rogue process, sqlbak will provide as much information as possible. 

Backups are automatically added to a tar file.  By default, this tar file is placed in the working directory of sqlbak, but this location can be specified using the --tarname option.

To back up all SQLite3 databases under your home directory:
<pre>
sqlbak ~
</pre>

Should sqlbak encounter errors during the backup process, they will be printed to stderr.  For example:

<pre>
sqlbak: not a database (/home/zach/Desktop/sqlbak/tests/malformed.db3)
sqlbak: cannot obtain lock (lock held by pid 5044) (/home/zach/Desktop/sqlbak/tests/locked.db3)
backing up '/home/zach/Desktop/sqlbak/tests/simple.db3'
</pre>
