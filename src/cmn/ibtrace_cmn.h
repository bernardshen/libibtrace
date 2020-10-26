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

#define INLINE __inline

/**
 * @name ANSI C Library
 * @brief Standart ANSI C library functions
 */
/** @{*/
#define sys_memcpy      memcpy
#define sys_memset      memset
#define sys_strlen      strlen
#define sys_strcpy      strcpy
#define sys_strncpy     strncpy
#define sys_strcmp      strcmp
#define sys_strcasecmp  strcasecmp
#define sys_strstr      strstr
#define sys_strchr      strchr
#define sys_sprintf     sprintf
#define sys_vsnprintf   vsnprintf
#define sys_strtol      strtol

#define IBTRACE_TRACE(fmt, ...) ((void)0)
#define IBTRACE_WARN(fmt, ...)  ((void)0)

#define UNDEFINED_VALUE (-1)
#define ibtrace_timestamp_diff(t_val)  (ibtrace_timestamp() - (t_val))

void *sys_dlsym(const char *symname, const char *symver);
int sys_dlcheck(const char *libname);

typedef enum {
    IBTRACE_ERR_NONE = 0x0,
    IBTRACE_ERR_BAD_ARGUMENT,
    IBTRACE_ERR_NOT_EXIST,
    IBTRACE_ERR_UNSUPPORTED,
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

void sys_free(void *mem);

/**
 * sys_time
 *
 * @brief
 *    Gives the number of seconds and microseconds since the  Epoch.
 *
 * @param[in]    tv             Argument is a struct timeval.
 *
 * @retval ( 0) - on success
 * @retval (-1) - on failure
 ***************************************************************************/
static int INLINE sys_time(struct timeval *tv)
{
	int status = 0;

#if defined(__LINUX__)
	status = gettimeofday(tv, NULL);
#else
	clock_t t = clock();
	tv->tv_sec = t / CLOCKS_PER_SEC;
	tv->tv_usec = t % CLOCKS_PER_SEC;
#endif

	return status;
}

#endif