#include "ibtrace_cmn.h"
#include "ibtrace_api.h"

#include <infiniband/verbs.h>

#include "ibtrace_ibv.h"
#include "ibtrace_logger.h"

#define DEFAULT_SYMVER      "IBVERBS_1.1"

#define check_dlsym(_func) check_dlsymv(_func, DEFAULT_SYMVER)
#define check_dlsymv(_func, _ver) \
        do {                                                            \
            ibv_module_context.noble._func = sys_dlsym(#_func, _ver);   \
            if (!ibv_module_context.noble._func)                        \
                status = IBTRACE_ERR_UNSUPPORTED;                       \
        } while(0)
#define check_api(_func) \
	do {                                                                \
		ret->ops._func = ibv_module_context.mean.ibv_##_func;       \
	} while (0)

struct ibv_ctx_t {
    uintptr_t               addr;
    struct ibv_context      item;
    struct ibv_ctx_t        *next;
};

#define OP_ON_MEMBERS_LIST(OP)  \
        OP(ibv_post_send)       \
        OP(ibv_poll_cq)         \
        OP(ibv_post_recv)       \
        OP(ibv_open_device)     \
        OP(ibv_close_device)    

OP_ON_MEMBERS_LIST(DECLARE_TYPE)

struct ibv_module_api_t {
    OP_ON_MEMBERS_LIST(DECLEAR_STRUCT_MEMBER)
};

static struct module_context_t {
    struct ibv_module_api_t noble;
    struct ibv_module_api_t mean;
    struct ibv_ctx_t *ibv_ctx;
} ibv_module_context;

#define DECLARE_OPTION_STRUCT(TYPE) \
struct ibv_module_api_t ibv_##TYPE##_funcs = { \
	OP_ON_MEMBERS_LIST(PREFIX##_##TYPE) \
};

#define DECLARE_OPTION_FUNCTIONS_PROTOTYPED(TYPE) \
        struct ibv_context* TYPE ## ibv_open_device(struct ibv_device *device) \
        { \
            void* ret = NULL;                                  \
            int flip_ret = 0;                                  \
            EMPLOY_TYPE(ibv_open_device) *f;                   \
            FUNC_BODY_RESOLVE_(ibv_open_device)                \
            PRE_##TYPE(ibv_open_device)                        \
            ret = f(device);                                   \
            ibv_open_device_handler((struct ibv_context*)ret); \
            POST_RET_##TYPE(ibv_open_device)                   \
            PRETEND_USED(flip_ret);                            \
            return ret;                                        \
        }; \
        int TYPE ## ibv_close_device(struct ibv_context *context) \
        { \
            int ret;                                \
            int flip_ret = 1;                       \
            EMPLOY_TYPE(ibv_close_device) *f;       \
            ibv_close_device_handler(context);      \
            FUNC_BODY_RESOLVE_(ibv_close_device)    \
            PRE_##TYPE(ibv_close_device)            \
            ret = f(context);                       \
            POST_##TYPE(ibv_close_device)           \
            PRETEND_USED(flip_ret);                 \
            return ret;                             \
        };

#define DECLARE_OPTION_FUNCTIONS_INLINE(TYPE) \
        int TYPE ## ibv_poll_cq(struct ibv_cq *cq, int ne, struct ibv_wc *wc) \
        { FUNC_BODY_INT(TYPE, _IBV, ibv_poll_cq, poll_cq, cq->context, cq, ne, wc) }; \
        int TYPE ## ibv_post_send(struct ibv_qp *ibqp, struct ibv_send_wr *wr, struct ibv_send_wr **bad_wr) \
        { FUNC_BODY_INT(TYPE, _IBV, ibv_post_send, post_send, ibqp->context, ibqp, wr, bad_wr) }; \
        int TYPE ## ibv_post_recv(struct ibv_qp *ibqp, struct ibv_recv_wr *wr, struct ibv_recv_wr **bad_wr) \
        { FUNC_BODY_INT(TYPE, _IBV, ibv_post_recv, post_recv, ibqp->context, ibqp, wr, bad_wr) };

// module declaration place
#define DECLARE_OPTION(type) \
DECLARE_OPTION_FUNCTIONS_PROTOTYPED(type) \
DECLARE_OPTION_FUNCTIONS_INLINE(type) \
DECLARE_OPTION_STRUCT(type)

DECLARE_OPTION_FUNCTIONS_PROTOTYPED( )

#define PREFIX_TRACE(x) TRACE##x,
DECLARE_OPTION(TRACE)

static const IBTRACE_MODULE_CALL ibv_tbl_call[] =
{
    OP_ON_MEMBERS_LIST(TBL_CALL_ENTRY)
    {UNDEFINED_VALUE, NULL, NULL}
};

static inline void ibv_open_device_handler(struct ibv_context *ret)
{
	if (ret)  {
		struct ibv_ctx_t *cur_ibv_ctx = ibv_module_context.ibv_ctx;
		struct ibv_ctx_t *new_ibv_ctx = NULL;
        printf("libibtrace ibv_open_device\n");

		/* This protection is in place because this function is called
		* twice: one with the prefix and suffix of choice (e.g. profiling)
		* and once as the original function, replacing ibv_open_device() so
		* that the LD_PRELOAD replaces the original, so we can hook it.
		*/
		if (cur_ibv_ctx && ((void*)cur_ibv_ctx->addr == ret)) {
			return;
		}

		new_ibv_ctx = sys_malloc(sizeof(*cur_ibv_ctx));
		new_ibv_ctx->next = cur_ibv_ctx;
		cur_ibv_ctx = ibv_module_context.ibv_ctx = new_ibv_ctx;
		cur_ibv_ctx->addr = (uintptr_t)ret;

		/* Save original addresses of ops */
#if defined(IBV_API_EXT)
		sys_memcpy(&(cur_ibv_ctx->item), verbs_get_ctx(ret), sizeof(cur_ibv_ctx->item));
# if (IBV_API_EXT > 1)
		sys_memcpy(&(cur_ibv_ctx->item_exp), verbs_get_exp_ctx(ret), sizeof(cur_ibv_ctx->item_exp));
# endif
#else
		sys_memcpy(&(cur_ibv_ctx->item), ret, sizeof(cur_ibv_ctx->item));
#endif /* IBV_API_EXT */

		/* Replace original ops with wrappers */
		check_api(poll_cq);
		check_api(post_send);
		check_api(post_recv);

	}
}

static inline void ibv_close_device_handler(struct ibv_context *context)
{
	if (ibv_module_context.ibv_ctx)	{
		struct ibv_ctx_t *cur_ibv_ctx = ibv_module_context.ibv_ctx;
		struct ibv_ctx_t *prev_ibv_ctx = NULL;
		while (cur_ibv_ctx && (cur_ibv_ctx->addr != (uintptr_t)context)) {
			prev_ibv_ctx = cur_ibv_ctx;
			cur_ibv_ctx = cur_ibv_ctx->next;
		}

        /* This protection is in place because this function is called
         * twice: one with the prefix and suffix of choice (e.g. profiling)
         * and once as the original function, replacing ibv_close_device() so
         * that the LD_PRELOAD replaces the original, so we can hook it.
         */
		if (!cur_ibv_ctx) {
		    return;
		}

#if defined(IBV_API_EXT)
		sys_memcpy(verbs_get_ctx(context), &(cur_ibv_ctx->item), sizeof(cur_ibv_ctx->item));
# if (IBV_API_EXT > 1)
		sys_memcpy(verbs_get_exp_ctx(context), &(cur_ibv_ctx->item_exp), sizeof(cur_ibv_ctx->item_exp));
# endif
#else
		sys_memcpy(context, &(cur_ibv_ctx->item), sizeof(cur_ibv_ctx->item));
#endif /* IBV_API_EXT */
		if (prev_ibv_ctx)
			prev_ibv_ctx->next = cur_ibv_ctx->next;
		else
			ibv_module_context.ibv_ctx = cur_ibv_ctx->next;
		sys_free(cur_ibv_ctx);
	}
}

static IBTRACE_ERROR 
__ibv_init(IBTRACE_MODULE_OBJECT *mod_obj)
{
    IBTRACE_ERROR status = IBTRACE_ERR_NONE;
    struct ibv_module_api_t* src_api = NULL;

    if ((status = sys_dlcheck("libibverbs.so")) != IBTRACE_ERR_NONE)
        return status;
    
    check_dlsym(ibv_post_send);
    check_dlsym(ibv_post_recv);
    check_dlsym(ibv_poll_cq);
    check_dlsym(ibv_open_device);
    check_dlsym(ibv_close_device);

    ibv_module_context.ibv_ctx = NULL;

    // let src_api be the defined funcs
    src_api = &ibv_TRACE_funcs;

    memcpy(&ibv_module_context.mean, src_api, sizeof(*src_api));
    return status;
}

static IBTRACE_ERROR
__ibv_exit(IBTRACE_MODULE_OBJECT *mod_obj)
{
    IBTRACE_ERROR status = IBTRACE_ERR_NONE;
    struct ibv_ctx_t *cur_ibv_ctx = ibv_module_context.ibv_ctx;

    while (cur_ibv_ctx) {
        struct ibv_context *context = (struct ibv_context *)cur_ibv_ctx->addr;
        ibv_module_context.ibv_ctx = cur_ibv_ctx->next;
        sys_memcpy(context, &(cur_ibv_ctx->item), sizeof(cur_ibv_ctx->item));
        sys_free(cur_ibv_ctx);
    }
    ibv_module_context.ibv_ctx = NULL;
    return status;
}

IBTRACE_MODULE_OBJECT ibv_module = {
    IBTRACE_MODULE_IBV,
    "libibverbs",
    "user library",
    ibv_tbl_call,
    __ibv_init,
    __ibv_exit,
    (void *)&ibv_module_context
};

void 
ibtrace_post_ret(int retval, char *func_name, ...)
{
    printf("%s incoked\n", func_name);
    // apply instrumentation according to the func_name
    if (strcmp(func_name, "ibv_post_send") == 0) {
        // restore args
        va_list args;
        struct ibv_qp *qp;
        struct ibv_send_wr *wr;
        struct ibv_send_wr **bad_wr;
        va_start(args, func_name);
        qp = va_arg(args, struct ibv_qp *);
        wr = va_arg(args, struct ibv_send_wr *);
        bad_wr = va_arg(args, struct ibv_send_wr **);
        ibtrace_post_ret_ibv_post_send(retval, qp, wr, bad_wr);
    } else if (strcmp(func_name, "ibv_post_recv") == 0) {
        // restore args
        va_list args;
        struct ibv_qp *qp;
        struct ibv_recv_wr *wr;
        struct ibv_recv_wr **bad_wr;
        va_start(args, func_name);
        qp = va_arg(args, struct ibv_qp *);
        wr = va_arg(args, struct ibv_recv_wr *);
        bad_wr = va_arg(args, struct ibv_recv_wr **);
        ibtrace_post_ret_ibv_post_recv(retval, qp, wr, bad_wr);
    } else if (strcmp(func_name, "ibv_poll_cq") == 0) {
        // restore args of ibv_poll_cq
        va_list args;
        struct ibv_cq *cq;
        struct ibv_wc *wc;
        int ne;
        va_start(args, func_name);
        cq = va_arg(args, struct ibv_cq *);
        ne = va_arg(args, int);
        wc = va_arg(args, struct ibv_wc *);
        ibtrace_post_ret_ibv_poll_cq(retval, cq, ne, wc);
    }

}

void
ibtrace_post_ret_ibv_post_send(int retval, struct ibv_qp *qp, struct ibv_send_wr *wr, struct ibv_send_wr **bad_wr)
{
    // TODO: Add more data to log
    // getpid
    long pid = getpid();
    if (retval != 0) {
        printlog("pid: %ld, ibv_post_send, qp: %d, error: %d\n", 
            pid, qp->qp_num, retval);
    } else {
        struct ibv_send_wr *wrp;
        for (wrp = wr; wrp; wrp = wrp->next) {
            printlog("pid: %ld, ibv_post_send, qp: %d, wr_id: %d. num_sge: %d\n",
                pid, qp->qp_num, wr->wr_id, wr->num_sge);
        }
    }
}

void
ibtrace_post_ret_ibv_post_recv(int retval, struct ibv_qp *qp, struct ibv_recv_wr *wr, struct ibv_recv_wr **bad_wr)
{
    // TODO: Add more data to log
    // getpid
    long pid = getpid();
    if (retval != 0) {
        printlog("pid: %ld, ibv_post_recv, qp: %d, error: %d\n", 
            pid, qp->qp_num, retval);
    } else {
        struct ibv_recv_wr *wrp;
        for (wrp = wr; wrp; wrp = wrp->next) {
            printlog("pid: %ld, ibv_post_recv, qp: %d, wr_id: %d, num_sge: %d\n", 
                pid, qp->qp_num, wrp->wr_id, wrp->num_sge);
        }
    }
}

void 
ibtrace_post_ret_ibv_poll_cq(int retval, struct ibv_cq *cq, int ne, struct ibv_wc *wc) 
{
    // TODO: Add more data to log
    if (wc == NULL) {
        return;
    }
    long pid = getpid();
    if (wc->status == IBV_WC_SUCCESS) {
        printlog("pid: %ld, ibv_poll_cq, qp: %d, wr_id: %ld, src_qp: %ld, opcode: %d\n", 
            pid, wc->qp_num, wc->wr_id, wc->src_qp, wc->opcode);
    } else {
        printlog("pid: %ld, ibv_poll_cq, qp: %d, status: %d, vender_syndrom: %d\n", 
            pid, wc->qp_num, wc->status, wc->vendor_err);
    }
}