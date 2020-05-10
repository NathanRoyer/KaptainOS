#ifndef PAGING_H
#define PAGING_H

#include "../utils/types.h"
#include "../utils/mset.h"
#include "malloc.h"
#include "interrupts/isr.h"

/*
 * PAGE FRAME < PAGE TABLE < PAGE DIRECTORY
 *    4096         1024           1024
 *    bytes         PF             PT
 * 
 * 1024*1024*4096 = 4GiB
 *
 * Most useful explanations : jamesmolloy.co.uk/tutorial_html/6.-Paging.html
 */

typedef struct {
	u32 present  :  1;
	u32 rw       :  1;
	u32 usermode :  1;
	u32 rsvd1    :  2;
	u32 accessed :  1;
	u32 written  :  1;
	u32 rsvd2    :  2;
	u32 custom1  :  1;
	u32 custom2  :  1;
	u32 custom3  :  1;
	u32 frmaddr  : 20;
} page_t;

typedef struct {
	page_t pages[1024];
} page_table_t;

typedef struct {
	page_table_t * tables[1024];
	u32 tables_and_flags[1024];
	u32 meta_phy_addr;
} page_directory_t;

extern page_directory_t * current_page_directory;

void init_paging();
void update_page_directory(page_directory_t * new_pdir);
void ext_id_map(u32 start_address, u32 end_address);
void * ext_getphyaddr(void * address);

#endif
