#ifndef __VM_SWAP_H
#define __VM_SWAP_H

#include <hash.h>
#include <list.h>
#include <bitmap.h>

void swap_init(size_t used_index, void *kaddr);
void swap_in(size_t used_index, void *kaddr);
size_t swap_out(void *kaddr);

#endif