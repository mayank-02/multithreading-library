/**
 * @file attr.c
 * @brief Attribute handling functions
 * @author Mayank Jain
 * @bug No known bugs
 */

#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <stdarg.h>
#include "mthread.h"
#include "utils.h"

/**
 * @brief Controls the attributes depending on the operation
 * @param[in] cmd Get or Set the attribute
 * @param[in,out] a Attribute object
 * @param[in] op Attribute
 * @param[in] ap Variable argument list corresponding to attribute
 * @return On success, returns 0; on error, it returns an error number
 */
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
                dst = &a->a_joinable;
            }
            else {
                src = &a->a_joinable;
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
                dst = &a->a_stacksize;
            }
            else {
                src = &a->a_stacksize;
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
                dst = &a->a_stackaddr;
            }
            else {
                src = &a->a_stackaddr;
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

/**
 * @brief Make a new thread attributes object and initialise it
 * @return On success, returns pointer to attribute object;
 * on error, it returns NULL
 */
mthread_attr_t *mthread_attr_new(void) {
    mthread_attr_t *a;
    a = (mthread_attr_t *) calloc(1, sizeof(mthread_attr_t));
    if(a == NULL) {
        return NULL;
    }

    mthread_attr_init(a);
    return a;
}

/**
 * @brief Controls the attributes depending on the operation
 * @param[in,out] a Attribute object to be initialised
 * @return On success, returns 0; on error, it returns an error number
 */
int mthread_attr_init(mthread_attr_t *a) {
    if(a == NULL)
        return EINVAL;

    util_strncpy(a->a_name, "Unknown", MTHREAD_TCB_NAMELEN);
    a->a_joinable = 1;
    a->a_stacksize = 64 * 1024;
    a->a_stackaddr = NULL;

    return 0;
}

/**
 * @brief Get particular attribute from thread attribute object
 * @param[in] a Attribute object
 * @param[in] op Attribute
 * @param[in] ... Variable argument list corresponding to attribute
 * @return On success, returns 0; on error, it returns an error number
 */
int mthread_attr_get(mthread_attr_t *a, int op, ...) {
    va_list ap;
    int ret;

    va_start(ap, op);
    ret = mthread_attr_ctrl(MTHREAD_ATTR_GET, a, op, ap);
    va_end(ap);
    return ret;
}

/**
 * @brief Set particular attribute in thread attribute object
 * @param[in,out] a Attribute object
 * @param[in] op Attribute
 * @param[in] ... Variable argument list corresponding to attribute
 * @return On success, returns 0; on error, it returns an error number
 */
int mthread_attr_set(mthread_attr_t *a, int op, ...) {
    va_list ap;
    int ret;

    va_start(ap, op);
    ret = mthread_attr_ctrl(MTHREAD_ATTR_SET, a, op, ap);
    va_end(ap);
    return ret;
}

/**
 * @brief Destroy thread attribute
 * @param[in,out] a Attribute object
 * @return On success, returns 0; on error, it returns an error number
 */
int mthread_attr_destroy(mthread_attr_t *a) {
    if(a == NULL)
        return EINVAL;
    free(a);
    return 0;
}