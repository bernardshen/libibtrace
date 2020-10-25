#include "ibtrace_cmn.h"

FILE *ibprof_dump_file;

static const char *_libname = NULL;
static int _libname_counter = 0;

void 
*sys_dlsym(const char *symname, const char *symver)
{
	void *symaddr = NULL;
        const char *err = NULL;
#if defined(__LINUX__)
	void *libc_handle = RTLD_NEXT;
again:
	dlerror(); /* Clear any existing error */
	if (NULL == symver)
	symaddr = dlsym(libc_handle, symname);
	else
	symaddr = dlvsym(libc_handle, symname, symver);

	err = dlerror();
	if (!symaddr || err) {
		/* This code is a workaround to RTLD_NEXT unexpected behaivour */
		if (libc_handle == RTLD_NEXT) {
#ifndef __COVERITY__
			/* If the same library is loaded again with dlopen(), the same library
			 * handle is returned
			 */
			dlerror();    /* Clear any existing error */
			libc_handle = dlopen(_libname, RTLD_LAZY);
			err = dlerror();
			if (libc_handle && !err) {
				if (_libname_counter > 0) {
					dlclose(libc_handle);
				}
				_libname_counter++;
				goto again;
			}
#endif /* __COVERITY__ */
		}
		IBPROF_TRACE("Can't resolve %s: %s\n", symname, err);
	}
#else
#endif
	return symaddr;
}

int
sys_dlcheck(const char *libname) 
{
    int ret = IBTRACE_ERR_NONE;
    void *libc_handle = NULL;
    const char *err = NULL;
#if defined(__LINUX__)
    dlerror();
    libc_handle = dlopen(libname, RTLD_LAZY);

    err = dlerror();
    if (!libc_handle || err) {
        IBTRACE_WARN("Can't find %s: %s\n", libname, err);
        ret = IBTRACE_ERR_NOT_EXIST;
    }
    if (libc_handle) {
        _libname = libname;
        _libname_counter = 0;
        dlclose(libc_handle);
    }
#else
#endif
    return ret;
}