/** 
 * @file utils.c
 * @brief Utility functions
 * @author Mayank Jain
 * @bug No known bugs
 */

#include <sys/resource.h>
#include "utils.h"

/**
 * @brief Get the limit of extant processes
 * @return Extant process limiit
 */
size_t get_extant_process_limit(void) {
    struct rlimit limit;
    getrlimit(RLIMIT_NPROC, &limit);
    return limit.rlim_cur;
}

/**
 * @brief Copy a string of length atmost n bytes including '\0' character
 * @param[in/out] dst String to be copied into
 * @param[in] src String to be copied from
 * @param[in] dst_size Number of characters to copy
 * @return Pointer to first character of destination string
 */
char *util_strncpy(char *dst, const char *src, size_t dst_size) {
    if(dst_size == 0)
        return dst;

    char *d = dst;
    char *end = dst + dst_size - 1;

    while(d < end) {
        if((*d = *src) == '\0')
            return dst;
        d++;
        src++;
    }
    *d = '\0';

    return dst;
}