#include "ibtrace_api.h"
#include "ibtrace_cmn.h"


// Use to load modules when the lib is loaded
extern IBTRACE_MODULE_OBJECT ibv_module;

double 
ibtrace_timestamp(void)
{
    #if defined(CONF_TIMESTAMP) && (CONF_TIMESTAMP == 1)
        return ((double) sys_rdtsc() / __get_cpu_clocks_per_sec());
    #else
        struct timeval tv;

        sys_time(&tv);

        return ((tv.tv_sec) + (tv.tv_usec) * 1.0e-6);
    #endif
}

void
ibtrace_dump(void)
{
    // TODO: complete this
    printf("libibtrace exits\n");
}

void
ibtrace_update(int module, int call, double tm)
{
    // TODO: complete this
    printf("ibtrace_update\n");
}

FILE *ibtrace_dump_file;


void __attribute__((constructor))
__ibtrace_init(void)
{
    // IBTRACE_ERROR status = IBTRACE_ERR_NONE;

    // ibtrace_dump_file = (FILE*)stderr;
    
    // TODO: initialize ibv_module
    printf("Loading libibtrace\n");
    printf("Initializing ibv_module\n");
    ibv_module.init(&ibv_module);

    // TODO: intialize other part to trace
}

void __attribute__((destructor))
__ibtrace_exit(void)
{
    ibtrace_dump();
}