#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <getopt.h>
#include <libgen.h>
#include <sys/time.h>
#include <time.h>

#include "log.h"
#include "fswalk.h"
#include "backup.h"

const char *version = "1.0.0";

static struct option long_opts[] = {
    { "tarname", required_argument, 0, 't' },
    { "dry-run", no_argument, 0, 'd' },
    { "integrity-check", no_argument, 0, 'i' },
    { "follow-links", no_argument, 0, 'f' },
    { "ms-towait", required_argument, 0, 'm' },
    { "verbose", no_argument, 0, 'b' },
    { "version", no_argument, 0, 'v' },
    { NULL, 0, 0, 0 }
};

static void usage()
{
    fprintf(stderr, "usage: sqlbak directory [--tarname=/path/to/tar]\n" 
                    "                        [--ms-towait=ms]\n" 
                    "                        [--dry-run]\n" 
                    "                        [--integrity-check]\n"
                    "                        [--follow-links]\n" 
                    "                        [--verbose]\n"
                    "                        [--version]\n\n");

    exit(EXIT_FAILURE);
}

static char* get_tarname(const char *path)
{
    struct timeval tv;
    char *timestr = calloc(FILENAME_MAX, sizeof(char));
    char *pathstr = calloc(FILENAME_MAX, sizeof(char));
    char *tarpstr = calloc(FILENAME_MAX, sizeof(char));
    char *basestr = NULL;

    strncpy(pathstr, path, FILENAME_MAX);
    basestr = basename(pathstr);

    gettimeofday(&tv, NULL);
    strftime(timestr, 128, "%Y_%m_%d_%H_%M_%S", localtime(&tv.tv_sec));
    snprintf(tarpstr, 128, "%s_%s.tar.gz", basestr, timestr);

    free(timestr);
    free(pathstr);

    return tarpstr;
}

int main(int argc, char *argv[])
{
    char *path = NULL;
    char *tarname = NULL;
    char *tmpname = NULL;
    int timeout = 0;
    int opt = 0;

    walk_callback cb = NULL;

    if (argc == 1) { usage(); }
    path = argv[1];

    while ((opt = getopt_long_only(argc, argv, "", 
                                   long_opts, NULL)) != -1) {
        switch (opt) {
            case 't':
                tarname = strdup(optarg);
                break;

            case 'd':
                cb = backup_dry_run;
                break;

            case 'i':
                cb = backup_integrity_check;
                break;

            case 'f':
                set_follow_lnk(FOLLOW_ON);
                break;

            case 'm':
                timeout = strtoul(optarg, NULL, 10); 
                set_lock_timeout(timeout);
                break;

            case 'b':
                setlog(LOG_ON);
                break;

            case 'v':
                printf("sqlbak v%s\n", version);
                exit(EXIT_SUCCESS);
                break;

            default:
                usage();
        }
    }

    path = realpath(path, NULL);
    if (!path) {
        perror(argv[0]);
        return 1;
    }

    if (!cb) { cb = backup; }
    if (!tarname) { tarname = get_tarname(path); }
    tmpname = tempnam(NULL, "sqbak");

    set_tar_path(tarname);
    set_tmp_path(tmpname);

    walk_dir(path, cb);

    logmsg("creating temp file '%s'\n", tmpname);
    logmsg("creating tar file '%s'\n", tarname);

    free(path);
    free(tarname);
    free(tmpname);

    return 0;
}
