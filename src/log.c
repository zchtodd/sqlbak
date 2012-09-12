#include <stdarg.h>
#include <stdio.h>

#include "log.h"

static int logging = 0;

int setlog(int log_level)
{
    logging = log_level;
    return 0;
}

int logmsg(char *format, ...)
{
    va_list args;
    va_start(args, format);

    if (logging) { vprintf(format, args); }
    va_end(args);

    return 0;
}
