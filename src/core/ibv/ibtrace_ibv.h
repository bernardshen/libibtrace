#ifndef _IBTRACE_IBV_H_
#define _IBTRACE_IBV_H_

#ifndef offsetof
#define offsetof(TYPE, MEMBER) ((uintptr_t) &((TYPE *)0)->MEMBER)
#endif

#define FUNC_BODY_RESOLVE_GET_CTX(context) ({                           \
    struct ibv_ctx_t *cur_ibv_ctx = ibv_module_context.ibv_ctx;         \
        while ((cur_ibv_ctx->addr != (uintptr_t)context) &&             \
            cur_ibv_ctx->next)                                          \
            cur_ibv_ctx = cur_ibv_ctx->next;                            \
    cur_ibv_ctx;                                                        \
    })

#define FUNC_BODY_RESOLVE_(func_name, ...)                              \
    f = ibv_module_context.noble.func_name;

#define INTERNAL_CHECK()
#define PRETEND_USED(var) do { (void)(var); } while (0)
#define EMPLOY_TYPE(func_name) __type_of_##func_name

// TODO: modify these two macros to support for trace
#define PRE_TRACE(func_name) ((void)0);
#define POST_TRACE(func_name) ((void)0);
#define POST_RET_TRACE(func_name) ((void)0);
#define POST_RET_TRACE_N(ret, func_name, ...) \
        ibtrace_post_ret(ret, #func_name, __VA_ARGS__);

#define PRE_(func_name) f = ibv_module_context.mean.func_name;
#define POST_(func_name)
#define POST_RET_(func_name)

#define FUNC_BODY_RESOLVE_IBV(func_name, ex_name, ctx)                  \
        struct ibv_ctx_t *cur_ibv_ctx = FUNC_BODY_RESOLVE_GET_CTX(ctx); \
        f = cur_ibv_ctx->item.ops.ex_name;

// TODO: modify the this macro to trace
#define FUNC_BODY_INT(type, ctx_type, func_name, ex_name, ctx, ...) \
        int ret;                                                    \
        int flip_ret = 1;                                           \
        EMPLOY_TYPE(func_name) *f;                                  \
        FUNC_BODY_RESOLVE##ctx_type(func_name, ex_name, ctx)        \
        PRE_##type(func_name)                                       \
        INTERNAL_CHECK();                                           \
        ret = f(__VA_ARGS__);                                       \
        POST_RET_##type##_N(ret, func_name, __VA_ARGS__)            \
        PRETEND_USED(flip_ret);                                     \
        return ret;

#define FUNC_BODY_PTR(type, ctx_type, func_name, ex_name, ctx, ...)     \
    void* ret;                                                          \
    int flip_ret = 0;                                                   \
    EMPLOY_TYPE(func_name) *f;                                          \
    FUNC_BODY_RESOLVE##ctx_type(func_name, ex_name, ctx)                \
    PRE_##type(func_name)                                               \
    INTERNAL_CHECK();                                                   \
    ret = f(__VA_ARGS__);                                               \
    POST_RET_##type(func_name)                                          \
    PRETEND_USED(flip_ret);                                             \
    return ret;

#define FUNC_BODY_VOID(type, ctx_type, func_name, ex_name, ctx, ...)    \
    int flip_ret = 0;                                                   \
    EMPLOY_TYPE(func_name) *f;                                          \
    FUNC_BODY_RESOLVE##ctx_type(func_name, ex_name, ctx)                \
    PRE_##type(func_name)                                               \
    INTERNAL_CHECK();                                                   \
    f(__VA_ARGS__);                                                     \
    POST_##type(func_name)                                              \
    PRETEND_USED(flip_ret);

#define DECLARE_TYPE(func_name) \
        typedef typeof(func_name) EMPLOY_TYPE(func_name);
#define DECLEAR_STRUCT_MEMBER(func_name) \
        EMPLOY_TYPE(func_name) * func_name;
#define TBL_CALL_NUMBER(func_name) \
        offsetof(struct ibv_module_api_t, func_name) / sizeof(void*)
#define TBL_CALL_ENTRY(func_name) \
        { TBL_CALL_NUMBER(func_name), #func_name, NULL},

static inline void ibv_open_device_handler(struct ibv_context *ret);
static inline void ibv_close_device_handler(struct ibv_context *context);

void ibtrace_post_ret(int retval, char *func_name, ...);

void ibtrace_post_ret_ibv_poll_cq(int retval, struct ibv_cq *cq, int num_entries, struct ibv_wc *wc);
void ibtrace_post_ret_ibv_post_send(int retval, struct ibv_qp *qp, struct ibv_send_wr *wr, struct ibv_send_wr **bad_wr);
void ibtrace_post_ret_ibv_post_recv(int retval, struct ibv_qp *qp, struct ibv_recv_wr *wr, struct ibv_recv_wr **bad_wr);

#endif

