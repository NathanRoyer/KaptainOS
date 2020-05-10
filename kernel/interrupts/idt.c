/*
 * KaptainOS kernel
 * 
 * Interrupt Descriptor Table
 * 
 */

#include "idt.h"
#include "../../utils/mset.h"

void set_idt_gate(int id, u32 handler){
	idt[id].handler_low_address = u32_low(handler);
	idt[id].segment_selector = KERNEL_CODE_SEGMENT;
	idt[id].unused = 0;
	idt[id].flags = 0b10001110;
	/*			1      présent
				00     DPL 0, privilège maximum
				0      segment destinataire = 0
				1110   80386 32-bit Interrupt gate */
	idt[id].handler_high_address = u32_high(handler);
}

void set_idt(){
	idt_reg.base = (u32) &idt;
	idt_reg.limit = IDT_ENTRIES * sizeof(idt_gate_t) - 1;
	__asm__ __volatile__ ("lidtl (%0)" : : "r" (&idt_reg));
	__asm__ __volatile__ ("sti");
}
