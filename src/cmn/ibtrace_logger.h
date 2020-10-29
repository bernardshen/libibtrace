#ifndef _IBTRACE_LOGGER_H_
#define _IBTRACE_LOGGER_H_

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdarg.h>
#include <sys/time.h>

FILE *log_file;
enum Logger_Error {
    ERR_NONE,
    ERR_FILENAME,
    ERR_OPEN_FILE
};

int logger_init();
void printlog(char *fmt, ...);


#endif