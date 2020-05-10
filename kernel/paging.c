#include "paging.h"
#include "../services/drivers/teletype.h"

// TODO: alloc_frame solution

int_handler_t page_fault(registers_t regs);

// Bitmap :

u32 * frames;
u32 frames_count;

#define bit_index(a) (a/(8*4))
#define bit_offset(a) (a%(8*4))

void set_frame(u32 frame_index){
	u32 index = bit_index(frame_index);
	u32 offset = bit_offset(frame_index);
	frames[index] |= (0x1 << offset);
}

void clear_frame(u32 frame_index){
	u32 index = bit_index(frame_index);
	u32 offset = bit_offset(frame_index);
	frames[index] &= ~(0x1 << offset);
}

u32 test_frame(u32 frame_index){
	u32 index = bit_index(frame_index);
	u32 offset = bit_offset(frame_index);
	return (frames[index] & (0x1 << offset));
}

s32 find_free_frame(){
	u32 max_bit_index = bit_index(frames_count);
	for (u32 i = 0; i < max_bit_index; i++){
		if (frames[i] != 0xFFFFFFFF){
			for (u32 j = 0; j < 32; j++){
				if ( !(frames[i] & (0x1 << j)) ) return i*32+j;
			}
		}
	}
	return -1;
}

/*
 * alloc_frame(page, is_kernel, is_writeable)
 *
 * assign a page to the first free frame
 *
 */
void alloc_frame(page_t * page, bool is_kernel, bool is_writeable, s32 index){
	if (page->frmaddr != 0) return;
	else {
		if (index == -1) index = find_free_frame();
		if (index == -1) __asm__ __volatile__ ("int $14"); // no more free page ! => Kernel Panic
		if (test_frame(index)){
			kprint("ERROR: alloc_frame called on an already taken page !");
			// while (1);
		}
		set_frame(index);
		page->present  = true;
		page->rw       = is_writeable ? true  : false;
		page->usermode = is_kernel    ? false :  true;
		page->frmaddr  = index;
	}
}

// Function to deallocate a frame.
void free_frame(page_t * page){
	u32 frame;
	if (!(frame = page->frmaddr)) return; // frame is not allocated
	else {
		clear_frame(frame / 0x1000);
		page->frmaddr = 0;
	}
}

// Paging :

page_directory_t * current_page_directory;

/*
 * get_page(virtual_addr, pdir) :
 * 
 * returns the page corresponding to a virtual address
 *
 */
page_t * get_page(u32 virtual_addr, page_directory_t * pdir){
	virtual_addr >>= 12;
	
	u32 table_index = virtual_addr >> 10; // seconds 10 bits = offset dans la table
	if (pdir->tables[table_index]) return &pdir->tables[table_index]->pages[virtual_addr % 1024];
	else return NULL;
}

u32 v2phy(u32 addr, page_directory_t * pdir){
	return get_page(addr, pdir)->frmaddr * 0x1000 + (addr & 0xfff);
}

bool id_map(u32 start_addr, u32 end_addr, page_directory_t * pdir){
	u32 random_test_sauce = 0x9bd;
	while (start_addr < end_addr){
		page_t * p = get_page(start_addr, pdir);
		alloc_frame(p, false, false, start_addr >> 12);
		u32 test_address = start_addr + random_test_sauce;
		if (v2phy(test_address, pdir) != test_address){
			kprint("Id. mapping error: address mismatch\n");
			print_kv_hex("Expected phy addr", test_address);
			print_kv_hex("V2PHY result", v2phy(test_address, pdir));
			return false;
		}
		start_addr += 0x1000;
	}
	return true;
}

void init_paging(u32 ram_size, u32 kernel_space_limit){
	frames_count = ram_size / 0x1000; // = 4096
	u32 frame_count_bytes = bit_index(frames_count); // = 128
	frames = (u32*) malloc(frame_count_bytes, false); // bitmap allocation
	mset(frames, 0, frame_count_bytes);
	
	page_directory_t * kernel_dir = malloc(sizeof(page_directory_t), true);
	mset((u32)kernel_dir, 0, sizeof(page_directory_t));
	
	// allocating tables fully and early so the heap will fully work
	for (u32 table_index = 0; table_index < 1024; table_index++){
		kernel_dir->tables[table_index] = (page_table_t *) malloc(sizeof(page_table_t), true);
		mset(kernel_dir->tables[table_index], 0, 0x1000);
		kernel_dir->tables_and_flags[table_index] = (u32)kernel_dir->tables[table_index] | 7; // = present | RW | usermode (bits 1, 2, 3 : 4+2+1 = 7)
	}
	
	if (id_map(0, kernel_space_limit, kernel_dir)) kprint("Id. mapping: No mismatch\n");
	else while(1);
	
	print_kv("Kernel space goes from 0 to", kernel_space_limit);
	print_kv("Directory address", kernel_dir);
	
	register_interrupt_handler(14, page_fault);
	update_page_directory(kernel_dir);
	
	init_paged_malloc();
}

void paging_switch(bool enable){
	u32 cr0;
	__asm__ __volatile__ ("mov %%cr0, %0" : "=r"(cr0));
	if (enable) cr0 |= 0x80000000;
	else cr0 &= 0x7fffffff;
	__asm__ __volatile__ ("mov %0, %%cr0" : : "r"(cr0));
}

void update_page_directory(page_directory_t * new_pdir){
	current_page_directory = new_pdir;
	__asm__ __volatile__ ("mov %0, %%cr3" : : "r"(new_pdir->tables_and_flags));
	paging_switch(true);
}

int_handler_t page_fault(registers_t regs){
	kprint_color("Page fault ! Hanging.", 0b00000100);
	while (true);
}

void ext_id_map(u32 start_address, u32 end_address){
	if (id_map(start_address, end_address, current_page_directory))
		update_page_directory(current_page_directory);
	else kprint("id_map ERROR !\n");
}

void * ext_getphyaddr(void * address){
	return v2phy((u32)address, current_page_directory);
}







