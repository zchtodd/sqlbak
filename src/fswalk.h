static int const FOLLOW_ON = 1;
static int const FOLLOW_OFF = 0;

typedef int (*walk_callback)(const char* path);

int set_follow_lnk(int follow);

int walk_dir(const char *directory, 
             walk_callback cb);
