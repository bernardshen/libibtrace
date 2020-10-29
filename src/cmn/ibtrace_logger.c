#include "ibtrace_logger.h"

int logger_init() {
    long pid;
    struct timeval time;
    char log_file_name[256];
    int res = 0;

    // get pid and timestamp
    pid = getpid();
    gettimeofday(&time, NULL);

    // genereate the name of log file
    res = sprintf(log_file_name, "log_%ld_%ld.log", pid, time.tv_sec);
    if (res < 0) {
        return ERR_FILENAME;
    }

    // open the log file
    log_file = fopen(log_file_name, "w");
    if (log_file == NULL) {
        return ERR_OPEN_FILE;
    }
    return ERR_NONE;
}

void printlog(char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    fprintf(log_file, fmt, args);
}