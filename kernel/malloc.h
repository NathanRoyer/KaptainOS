#ifndef MALLOC_H
#define MALLOC_H

#include "../utils/types.h"

#define MALLOC_ALIGN 1

typedef struct heap_block_footer heap_block_footer;

typedef struct heap_block_footer {
	u32 size;
	heap_block_footer * next_footer;
	heap_block_footer * prev_footer;
	bool available;
} heap_block_footer;

u32 malloc(u32 size, u32 flags);
u32 malloc_debug(u32 size, u32 flags);
void init_paged_malloc();

extern u32 non_paged_heap_btm;
extern u32 non_paged_heap_top;
extern u32 allocated_memory_amount;

#endif
