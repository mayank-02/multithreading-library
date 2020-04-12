/** 
 * @file utils.c
 * @brief Utility functions
 * @author Mayank Jain
 * @bug No known bugs
 */

#include "utils.h"

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