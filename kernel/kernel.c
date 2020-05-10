/*
 * KaptainOS kernel
 */

#include "../services/drivers/teletype.h"
#include "../services/drivers/cpu_timer.h"
#include "../services/drivers/vga.h"
#include "../services/drivers/keyboard.h"
#include "../services/hardware/pci.h"
#include "../programs/console.h"
#include "interrupts/isr.h"
#include "paging.h"
#include "malloc.h"
#include "../utils/string.h"

void banner(bool minimal){
	if (!minimal) kprint("\n +-------------+\n |  ");
	else kprint("Welcome to ");
	kprint_color("KaptainOS", 0b00000100);
	if (!minimal) kprint("  |\n +-------------+\n\n");
	else kprint("\n");
}


void kernel_start(){
	// keyboard
	init_keyboard();
	// CPU timer
	init_cpu_timer(50);
	// interrupts
	install_isrs();
	// PCI tree
	init_pci();
	
	// big logo
	show_boot_logo(500);
	
	// paging
	init_paging(0x3fffffff, 0xa00000); // 1GiB ram, 10MiB kernel space
	
	clear_screen();
	// big console
	// vga_text_big(); // incompatible avec certains environnements réels
	
	// small logo
	// banner(false);
	
	console();
}














