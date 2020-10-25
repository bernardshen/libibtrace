#include "ibtrace_cmn.h"
#include "ibtrace_api.h"

#include <infiniband/verbs.h>
#include <infiniband/arch.h>

#include "ibtrace_ibv.h"

struct ibv_ctx_t {
    uintptr_t               addr;
    struct ibv_context      item;
    struct ibv_ctx_t        *next;
};

#define OP_ON_MEMBERS_LIST(OP) \
        OP(ibv_post_send) \
        OP(ibv_poll_cq) \
        OP(ibv_post_recv)

static struct module_context_t {
    struct ibv_module_api_t noble;
    struct ibv_module_api_t mean;
    struct ibv_ctx_t *ibv_ctx;
} ibv_module_context;

OP_ON_MEMBERS_LIST(DECLARE_TYPE)

struct ibv_module_api_t {
    OP_ON_MEMBERS_LIST(DECLEAR_STRUCT_MEMBER)
};

#define DECLARE_OPTION_STRUCT(TYPE) \
struct ibv_module_api_t ibv_##TYPE##_funcs = { \
	OP_ON_MEMBERS_LIST(PREFIX##_##TYPE) \
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
DECLARE_OPTION_FUNCTIONS_INLINE(type) \
DECLARE_OPTION_STRUCT(type)

#define PREFIX_TRACE(x) TRACE##x,
DECLARE_OPTION(TRACE)

static const IBTRACE_MODULE_CALL ibv_tbl_call[] =
{
    OP_ON_MEMBERS_LIST(TBL_CALL_ENTRY)
    {UNDEFINED_VALUE, NULL, NULL}
};

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

    ibv_module_context.ibv_ctx = NULL;

    // let src_api be the defined funcs
    src_api = &ibv_TRACE_funcs;

    memcpy(&ibv_module_context.mean, src_api, sizeof(*src_api));
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