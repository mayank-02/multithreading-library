#ifndef _UTILS_H_
#define _UTILS_H_

#include <sys/types.h>

size_t get_extant_process_limit(void);

char *util_strncpy(char *dst, const char *src, size_t dst_size);

#endif