#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <sqlite3.h>
#include <libtar.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>

#include "fswalk.h"
#include "log.h"

#include "backup.h"

static const char* tmp_path = NULL;
static const char* tar_path = NULL;
static int lock_timeout = 2000;

int set_tmp_path(const char* path)
{
    tmp_path = path;
    return 0;
}

int set_tar_path(const char* path)
{
    tar_path = path;
    return 0;
}

int set_lock_timeout(int timeout)
{
    lock_timeout = timeout;
    return 0;
}

int get_blocking_pid(const char* path)
{
    struct flock fl;
    int fd = 0;

    fd = open(path, O_RDWR);

    fl.l_type = F_WRLCK;
    fl.l_start = 0;
    fl.l_whence = SEEK_SET;
    fl.l_len = 0;

    fcntl(fd, F_GETLK, &fl);

    return fl.l_pid;
}

int backup_dry_run(const char* path)
{
    printf("backing up '%s' (dry run)\n", path);
    return 0;
}

int backup_integrity_check(const char* path)
{
    sqlite3 *src_db = NULL;
    sqlite3_stmt *stmt = NULL;
    const unsigned char* integrity = NULL;
    int rc = 0;

    rc = sqlite3_open(path, &src_db);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "sqlbak: %s (%s)\n", sqlite3_errmsg(src_db), path);
        return -1;
    }

    rc = sqlite3_prepare_v2(src_db, "PRAGMA integrity_check", -1, &stmt, NULL);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "sqlbak: %s (%s)\n", sqlite3_errmsg(src_db), path);
        return -1;
    }

    sqlite3_step(stmt);
    integrity = sqlite3_column_text(stmt, 0);

    printf("%s: %s\n", path, integrity);

    sqlite3_finalize(stmt);
    sqlite3_close(src_db);
    return 0;
}

int backup(const char* path)
{
    TAR *t = NULL;
    sqlite3 *tmp_db = NULL;
    sqlite3 *src_db = NULL;
    sqlite3_backup *bak = NULL;
    int rc = 0;

    unlink(tmp_path);

    rc = sqlite3_open(tmp_path, &tmp_db);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "sqlbak: %s (%s)\n", sqlite3_errmsg(tmp_db), tmp_path);
        return -1;
    }

    rc = sqlite3_open(path, &src_db);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "sqlbak: %s (%s)\n", sqlite3_errmsg(src_db), path);
        return -1;
    }

    sqlite3_busy_timeout(src_db, lock_timeout);

    bak = sqlite3_backup_init(tmp_db, "main", src_db, "main");
    if (!bak) {
        fprintf(stderr, "sqlbak: %s (%s)\n", sqlite3_errmsg(tmp_db), tmp_path);
        return -1;
    }

    rc = sqlite3_backup_step(bak, -1);
    if (rc != SQLITE_DONE) {
        switch (rc) {
            case SQLITE_NOTADB:
                fprintf(stderr, "sqlbak: %s (%s)\n", "not a database", path);
                break;
            
            case SQLITE_BUSY:
                fprintf(stderr, "sqlbak: %s (lock held by pid %d) (%s)\n", 
                                "cannot obtain lock", get_blocking_pid(path), path);
                break;
        }
        return -1;
    }

    rc = sqlite3_backup_finish(bak);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "sqlbak: %s (%s)\n", sqlite3_errmsg(tmp_db), tmp_path);
        return -1;
    }

    sqlite3_close(tmp_db);
    sqlite3_close(src_db);

    printf("backing up '%s'\n", path);

    tar_open(&t, (char*)tar_path, NULL, O_WRONLY | O_CREAT | O_APPEND, 0644, 0);
    tar_append_file(t, (char*)tmp_path, (char*)path);
    tar_close(t);

    unlink(tmp_path);
 
    return 0;
}
