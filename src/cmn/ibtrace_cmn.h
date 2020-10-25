#ifndef _IBTRACE_DEF_H_
#define _IBTRACE_DEF_H_

#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <strings.h>
#include <ctype.h>
#include <time.h>
#include <float.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>
#if defined(__LINUX__)
    #include <stdint.h>
    #include <inttypes.h>
    #include <unistd.h>
    #include <dlfcn.h>
    #include <sys/time.h>
    #include <sys/socket.h>
    #include <sys/ioctl.h>
    #include <netinet/in.h>
    #include <arpa/inet.h>
    #include <pthread.h>
#else
    #include <windows.h>
    #include <winsock.h>
    #include <process.h>
#endif

#define UNDEFINED_VALUE (-1)
#define ibtrace_timestamp_diff(t_val)  (ibtrace_timestamp() - (t_val))

void *sys_dlsym(const char *symname, const char *symver);
int sys_dlcheck(const char *libname);

typedef enum {
    IBTRACE_ERR_NONE = 0x0,
    IBTRACE_ERR_BAD_ARGUMENT,
    IBTRACE_ERR_NOT_EXIST,
    IBTRACE_ERR_UNKNOWN
} IBTRACE_ERROR;

typedef struct _IBTRACE_MODULE_CALL {
    int call;
    const char *name;
    const char *sign;
} IBTRACE_MODULE_CALL;

typedef struct _IBTRACE_MODULE_OBJECT {
    int id;
    const char *name;
    const char *desc;
    const IBTRACE_MODULE_CALL *tbl_call;
    IBTRACE_ERROR (*init)(struct _IBTRACE_MODULE_OBJECT *);
    IBTRACE_ERROR (*exit)(struct _IBTRACE_MODULE_OBJECT *);
    void *context;
} IBTRACE_MODULE_OBJECT;

#endif