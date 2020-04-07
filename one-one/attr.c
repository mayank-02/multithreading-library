#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <stdarg.h>
#include "mthread.h"
#include "utils.h"

static int mthread_attr_ctrl(int cmd, mthread_attr_t *a, int op, va_list ap) {
    if(a == NULL)
        return EINVAL;
    switch(op) {
        case MTHREAD_ATTR_NAME: {
            /* name */
            if(cmd == MTHREAD_ATTR_SET) {
                char *src, *dst;
                src = va_arg(ap, char *);
                dst = a->a_name;
                util_strncpy(dst, src, MTHREAD_TCB_NAMELEN);
            }
            else {
                char *src, **dst;
                src = a->a_name;
                dst = va_arg(ap, char **);
                *dst = src;
            }
            break;
        }
        case MTHREAD_ATTR_JOINABLE: {
            /* detachment type */
            int val, *src, *dst;
            if(cmd == MTHREAD_ATTR_SET) {
                src = &val;
                val = va_arg(ap, int);
                dst = &a->a_detach_state;
            }
            else {
                src = &a->a_detach_state;
                dst = va_arg(ap, int *);
            }
            *dst = *src;
            break;
        }
        case MTHREAD_ATTR_STACK_SIZE: {
            /* stack size */
            size_t val, *src, *dst;
            if(cmd == MTHREAD_ATTR_SET) {
                src = &val;
                val = va_arg(ap, size_t);
                dst = &a->a_stack_size;
            }
            else {
                src = &a->a_stack_size;
                dst = va_arg(ap, size_t *);
            }
            *dst = *src;
            break;
        }
        case MTHREAD_ATTR_STACK_ADDR: {
            /* stack address */
            void *val, **src, **dst;
            if(cmd == MTHREAD_ATTR_SET) {
                src = &val;
                val = va_arg(ap, void *);
                dst = &a->a_stack_base;
            }
            else {
                src = &a->a_stack_base;
                dst = va_arg(ap, void **);
            }
            *dst = *src;
            break;
        }
        default:
            return EINVAL;
    }
    return 0;
}

mthread_attr_t *mthread_attr_new(void) {
    mthread_attr_t *a;
    a = (mthread_attr_t *) calloc(1, sizeof(mthread_attr_t));
    if(a == NULL) {
        /* return ENOMEM; */
        return NULL;
    }

    mthread_attr_init(a);
    return a;
}

int mthread_attr_init(mthread_attr_t *a) {
    if(a == NULL)
        return EINVAL;

    util_strncpy(a->a_name, "Unknown", MTHREAD_TCB_NAMELEN);
    a->a_detach_state = JOINABLE;
    a->a_stack_size = 8196 * 1024;
    a->a_stack_base = NULL;

    return 0;
}

int mthread_attr_get(mthread_attr_t *a, int op, ...) {
    va_list ap;
    int ret;

    va_start(ap, op);
    ret = mthread_attr_ctrl(MTHREAD_ATTR_GET, a, op, ap);
    va_end(ap);
    return ret;
}

int mthread_attr_set(mthread_attr_t *a, int op, ...) {
    va_list ap;
    int ret;

    va_start(ap, op);
    ret = mthread_attr_ctrl(MTHREAD_ATTR_SET, a, op, ap);
    va_end(ap);
    return ret;
}

int mthread_attr_destroy(mthread_attr_t *a) {
    if(a == NULL)
        return EINVAL;
    free(a);
    return 0;
}