int set_tmp_path(const char* path);
int set_tar_path(const char* path);
int set_lock_timeout(int timeout);

int get_blocking_pid(const char* path);

int backup_dry_run(const char* path);
int backup_integrity_check(const char* path);
int backup(const char* path);
