sqlbak
======

Many applications use a collection of SQLite3 databases.  Simply copying or using tar to backup a set of production databases may work -- but this approach will inevitably
fail with an application that has any serious amount of write activity.  SQLite will create a journal file whenever a transaction takes place, and if this happens to be when
that tar cron job fires off, malformed database backups become very likely.  Tar may finish copying important.db3-journal just as SQLite alters the file, making it out of sync with important.db3 in the backup.

Thankfully, SQLite provides an API for reliable backup.  This API is available via both the C API and the SQLite shell. 

This utility strives to provide the reliability of the C API, while also making the backup process as convenient as using the 'tar' approach
(minus the malformed databases).  Simply specify a directory ('.', '/home/', '~', '../', etc.) and all SQLite3 databases under that directory will be
backed up to a tar file matching the existing file structure.  

Any damaged, unreadable, or encrypted databases will be skipped with a warning.

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

Having trouble with file locking?  Try to out wait another process by increasing the busy timeout.
<pre>
sqlbak . --ms-towait=5000
backing up '/home/zach/Desktop/sqlbak/tests/locked.db3'
</pre>

Use sqlbak to check the health of all SQLite3 databases under a given directory.
<pre>
sqlbak . --integrity-check
sqlbak: file is encrypted or is not a database (/home/zach/Desktop/sqlbak/tests/malformed.db3)
/home/zach/Desktop/sqlbak/tests/locked.db3: ok
/home/zach/Desktop/sqlbak/tests/simple.db3: ok
</pre>



