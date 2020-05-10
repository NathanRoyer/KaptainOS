#include "../services/drivers/teletype.h"
#include "malloc.h"
#include "paging.h"

// Time to deliver the cookies
/*
 * Memory Allocation
 * +--------------+--------+---------------------+--------+
 * | Heap Block   - Footer | Heap Block          - Footer |
 * +--------------+--------+---------------------+--------+
 */

#define DEBUG if (debug_var)
#define FLAG(flagname) (flags & flagname)
#define ALIGN 1

#define BLOCK_START(block) ((u32) block - block->size)

u32 non_paged_heap_btm = 0x100000;
u32 non_paged_heap_top = 0x100000;

u32 malloc_non_paged(u32 size, u32 flags){
	if (FLAG(ALIGN) && (non_paged_heap_top & 0x00000fff)){ // fixed bug in James Molloy Heap code
		non_paged_heap_top &= 0xfffff000;
		non_paged_heap_top += 0x1000;
	}
	u32 tmp = non_paged_heap_top;
	non_paged_heap_top += size;
	return tmp;
}

heap_block_footer * paged_heap_fbf; // first block footer
heap_block_footer * paged_heap_lbf; // last block footer
void * paged_heap_start;
u32 allocated_memory_amount = 0;

bool paging_enabled = false;
u32 HEADER_SZ;
bool debug_var = false;

void init_paged_malloc(){
	paging_enabled = true;
	HEADER_SZ = sizeof(heap_block_footer);
	if (non_paged_heap_top & 0x00000fff) paged_heap_start = (non_paged_heap_top & 0xfffff000) + 0x1000;
	else paged_heap_start = non_paged_heap_top;
	paged_heap_fbf = (heap_block_footer *) (((u32)paged_heap_start) + 0x1000 - HEADER_SZ);
	paged_heap_fbf->size = 0x1000 - HEADER_SZ;
	paged_heap_fbf->next_footer = 0;
	paged_heap_fbf->prev_footer = 0;
	paged_heap_fbf->available = true;
	paged_heap_lbf = paged_heap_fbf;
}

heap_block_footer * split(heap_block_footer * upper_block, u32 new_upper_block_start){
	u32 _start = BLOCK_START(upper_block); // block start backup
	u32 new_upper_block_size = ((u32)upper_block) - new_upper_block_start;
	
	upper_block->size = new_upper_block_size;
	
	heap_block_footer * lower_block = new_upper_block_start - HEADER_SZ;
	lower_block->size = ((u32)lower_block) - _start;
	
	lower_block->next_footer = upper_block;
	lower_block->prev_footer = upper_block->prev_footer;
	
	upper_block->prev_footer = lower_block;
	if (upper_block == paged_heap_fbf) paged_heap_fbf = lower_block;
	else lower_block->prev_footer->next_footer = lower_block;
	
	return lower_block;
}

heap_block_footer * lookup_in_blocks(u32 size, bool align){
	u32 closest_aligned;
	bool req;
	heap_block_footer * current_block = paged_heap_fbf;
	while (0 != (u32)current_block){
		if (current_block->available){
			if (align){
				if (BLOCK_START(current_block) & 0x00000fff) closest_aligned = (BLOCK_START(current_block) & 0xfffff000) + 0x1000;
				else closest_aligned = BLOCK_START(current_block);
				
				req = (u32)current_block - closest_aligned >= size;
			} else req = current_block->size >= size;
			if (req) return current_block;
		}
		current_block = current_block->next_footer;
	}
	return (heap_block_footer *) -1;
}

u32 malloc(u32 size, u32 flags){
	if (!paging_enabled) return malloc_non_paged(size, flags);
	DEBUG print_kv_hex("malloc, req. size", size);
	heap_block_footer * hb;
	heap_block_footer * tmp;
	while (-1 == ( hb = lookup_in_blocks(size, FLAG(ALIGN)) ) ){ // Nothing available
		// new page allocation
		u32 new_page_addr = (u32) paged_heap_lbf + HEADER_SZ;
		page_t * new_page = get_page(new_page_addr, true, current_page_directory);
		alloc_frame(new_page, false, false, -1);
		DEBUG kprint("New page\n");
		
		// last block expansion
		if (paged_heap_lbf->available){
			DEBUG kprint("Expanding last block with new page\n");
			tmp = paged_heap_lbf;
			paged_heap_lbf = (heap_block_footer *) ((u32) paged_heap_lbf + 0x1000);
			*paged_heap_lbf = *tmp;
			paged_heap_lbf->size += 0x1000;
			paged_heap_lbf->prev_footer->next_footer = paged_heap_lbf;
		} else {
			DEBUG kprint("New block will start with new page\n");
			paged_heap_lbf->next_footer = (u32)paged_heap_lbf + 0x1000;
			paged_heap_lbf->next_footer->prev_footer = paged_heap_lbf;
			paged_heap_lbf = paged_heap_lbf->next_footer;
			paged_heap_lbf->size = 0x1000 - HEADER_SZ;
			paged_heap_lbf->next_footer = 0;
			paged_heap_lbf->available = true;
		}
	}
	DEBUG kprint("Valid hb found:\n");
	DEBUG print_kv_hex("Addr ", hb);
	DEBUG print_kv_hex("Start ", BLOCK_START(hb));
	DEBUG print_kv_hex("Size  ", hb->size);
	
	// is there space to save under hb ?
	if (FLAG(ALIGN) && (BLOCK_START(hb) & 0x00000fff)){
		u32 closest_aligned = (BLOCK_START(hb) & 0xfffff000) + 0x1000;
		if (closest_aligned - BLOCK_START(hb) > HEADER_SZ){ // space before new block will not be wasted
			DEBUG kprint("Filling space b4 aligned block\n");
			split(hb, closest_aligned);
		}
	}
	
	// is there space to save above hb ?
	u32 overage = hb->size - size;
	if (overage > HEADER_SZ){
		DEBUG kprint("Filling space above block\n");
		hb = split(hb, BLOCK_START(hb) + size + HEADER_SZ);
	}
	
	u32 start = BLOCK_START(hb);
	mset(start, 0, size);
	hb->available = false;
	allocated_memory_amount += hb->size;
	return start;
}

u32 malloc_debug(u32 size, u32 flags){
	debug_var = true;
	u32 ret = malloc(size, flags);
	debug_var = false;
	return ret;
}

void merge(heap_block_footer * upper, heap_block_footer * lower){
	upper->size += lower->size + HEADER_SZ;
	if (lower == paged_heap_fbf) paged_heap_fbf = upper;
	upper->prev_footer = lower->prev_footer;
	upper->prev_footer->next_footer = upper;
}

bool free(void * address){
	if (!paging_enabled) return;
	heap_block_footer * current_block = paged_heap_fbf;
	heap_block_footer * prev, * next;
	while (0 != (u32)current_block){
		if (BLOCK_START(current_block) > address) return false; // address not found
		if ((u32)current_block > address){ // we'll return in this code block so the first heap block above the address must be the right one
			allocated_memory_amount -= current_block->size;
			current_block->available = true;
			current_block->size = ((u32)current_block) - BLOCK_START(current_block);
			DEBUG print_kv_hex("freeing", address);
			prev = current_block->prev_footer;
			next = current_block->next_footer;
			if ((u32)prev && prev->available) merge(current_block, prev);
			if ((u32)next && next->available) merge(next, current_block);
			return true;
		}
		current_block = current_block->next_footer;
	}
	return false;
}

void show_current_heap(){
	if (!paging_enabled) return;
	heap_block_footer * current_block = paged_heap_fbf;
	kprint("[ Start / Size / Availability / Footer size ]\n");
	while (0 != (u32)current_block){
		kprint("[ ");
		print_hex(BLOCK_START(current_block));
		kprint(" / ");
		print_hex(current_block->size);
		kprint(current_block->available ? " / Free / ":" / Taken / ");
		print_hex(HEADER_SZ);
		kprint(" ]\n");
		current_block = current_block->next_footer;
	}
	return;
}

void malloc_test(){
	char * t0 = malloc(sizeof(char) * 10, false);
	mset(t0, '0', 8); t0[8] = '\n';
	kprint(t0);
	
	char * t1 = malloc(sizeof(char) * 10, false);
	mset(t1, '1', 8); t1[8] = '\n';
	kprint(t1);
	
	free(t0);
	
	u32 a = malloc(5, false);
	print_kv_hex("m3", a);
	
	char * t2 = malloc(sizeof(char) * 10, false);
	mset(t2, '2', 8); t2[8] = '\n';
	kprint(t2);
	
	free(a);
	
	char * t3 = malloc(sizeof(char) * 10, false);
	mset(t3, '3', 8); t3[8] = '\n';
	kprint(t3);
	
	free(t3);
	
	show_current_heap();
}

















