#include <stdlib.h>
#include <stdio.h>
#include <fnmatch.h>
#include <dirent.h>
#include <sys/stat.h>
#include <string.h>
#include <errno.h>

#include "fswalk.h"
#include "log.h"

static const char* format = "SQLite format 3";
static int format_len = 15;
static int follow_lnk = 0;

int set_follow_lnk(int follow) 
{
    follow_lnk = follow;
    return 0;
}

int walk_dir(const char *directory, walk_callback cb)
{
    FILE *fp = NULL;
    char header[15] = { 0 };

    DIR *dir = NULL;
    struct dirent *fileinfo = NULL; 
    struct stat sb;

    char **dirnames = NULL;
    int ndirs = 0;
    int i = 0;

    const char *d_name = NULL;
    char *fpath = calloc(FILENAME_MAX, sizeof(char));

    dir = opendir(directory);
    logmsg("entering directory %s\n", directory);

    if (!dir) {
        fprintf(stderr, "sqlbak: %s (%s)\n", 
                        strerror(errno), directory);
        free(fpath);
        return 1;
    }
 
    while ( (fileinfo = readdir(dir)) ) {
        d_name = fileinfo->d_name;

        if (strcmp(d_name, ".") == 0) { continue; }
        if (strcmp(d_name, "..") == 0) { continue; }

        snprintf(fpath, FILENAME_MAX, "%s/%s", directory, d_name);

        if (lstat(fpath, &sb) == -1) {
            fprintf(stderr, "sqlbak: %s (%s)\n", 
                            strerror(errno), fpath);
            free(fpath);
            return 1;
        }

        if (((sb.st_mode & S_IFMT) == S_IFLNK && follow_lnk) ||
            ((sb.st_mode & S_IFMT) == S_IFREG)) {

            fp = fopen(fpath, "rb");
            if (!fp) {
                fprintf(stderr, "sqlbak: %s (%s)\n", 
                                strerror(errno), fpath);
                free(fpath);
                return 1;
            }

            fread(header, 1, format_len, fp);
            fclose(fp);

            if (strncmp(header, format, format_len) == 0) {
                cb(fpath);
            }
        }

        else if ((sb.st_mode & S_IFMT) == S_IFDIR) {
            ndirs++;

            dirnames = realloc(dirnames, ndirs * sizeof(char*));
            dirnames[ndirs-1] = strdup(fpath);
        } 
    } 

    closedir(dir);
    logmsg("leaving directory %s\n", directory);

    for (i = 0; i < ndirs; i++) {
        walk_dir(dirnames[i], cb);
        free(dirnames[i]);
    }
   
    free(dirnames); 
    free(fpath); 

    return 0;
}
