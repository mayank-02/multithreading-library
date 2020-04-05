#include <sys/resource.h>
#include "utils.h"

size_t get_extant_process_limit(void) {
    struct rlimit limit;
    getrlimit(RLIMIT_NPROC, &limit);
    return limit.rlim_cur;
}

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