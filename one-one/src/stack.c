#include <unistd.h>
#include <sys/mman.h>
#include <sys/time.h>
#include <sys/resource.h>
#include "stack.h"

size_t get_stack_size(void) {
    struct rlimit limit;
    getrlimit(RLIMIT_STACK, &limit);
    return limit.rlim_cur;
}

size_t get_page_size() {
    return sysconf(_SC_PAGESIZE);
}

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

int deallocate_stack(void *base, size_t stack_size) {
    size_t page_size = get_page_size();
    return munmap(base - page_size, stack_size + page_size);
}