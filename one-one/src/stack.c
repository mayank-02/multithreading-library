/**
 * @file stack.c
 * @brief Thread stack allocation and deallocation functions
 * @author Mayank Jain
 * @bug No known bugs
 */

#include <unistd.h>
#include <sys/mman.h>
#include <sys/time.h>
#include <sys/resource.h>
#include "stack.h"

/**
 * @brief Get the stack size of a thread
 * @return Size in bytes
 */
size_t get_stack_size(void) {
    struct rlimit limit;
    getrlimit(RLIMIT_STACK, &limit);
    return limit.rlim_cur;
}

/**
 * @brief Get the page size of a thread
 * @return Size in bytes
 */
size_t get_page_size(void) {
    return sysconf(_SC_PAGESIZE);
}

/**
 * @brief Allocate a stack
 * @param[in] stack_size Size of stack to mmap
 * @return Pointer to the base of the stack
 */
void * allocate_stack(size_t stack_size) {
    size_t page_size = get_page_size();

    void *base = mmap(NULL,
                      stack_size + page_size,
                      PROT_READ | PROT_WRITE,
                      MAP_PRIVATE | MAP_ANONYMOUS | MAP_STACK,
                      -1,
                      0);
    if(base == MAP_FAILED)
        return NULL;

    if(mprotect(base, page_size, PROT_NONE) == -1) {
        munmap(base, stack_size + page_size);
        return NULL;
    }

    return base + page_size;
}

/**
 * @brief Deallocate a stack
 * @param[in] base Base of the stack
 * @param[in] stack_size Size of stack to mmap
 * @return On success, returns 0; On error, returns -1
 */
int deallocate_stack(void *base, size_t stack_size) {
    size_t page_size = get_page_size();
    return munmap(base - page_size, stack_size + page_size);
}