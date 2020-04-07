#ifndef _STACK_H_
#define _STACK_H_

size_t  get_stack_size(void);

void *  allocate_stack(size_t stack_size);

int     deallocate_stack(void *base, size_t stack_size);

#endif