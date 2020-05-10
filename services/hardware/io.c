/*
 * KaptainOS kernel
 * Hardware inputs and outputs
 */
#include "io.h"

// BYTE

u8 io_in_b(u16 port){
	u8 result;
	__asm__("inb %%dx, %%al" : "=a" (result) : "d" (port));
	return result;
}

void io_out_b(u16 port, u8 data){
	__asm__("outb %%al, %%dx" : : "a" (data), "d" (port));
}

// WORD

u16 io_in_w(u16 port){
	u16 result;
	__asm__("inw %%dx, %%ax" : "=a" (result) : "d" (port));
	return result;
}

void io_out_w(u16 port, u16 data){
	__asm__("outw %%ax, %%dx" : : "a" (data), "d" (port));
}

// LONG

u32 io_in_l(u16 port){
	u32 result;
	__asm__("inl %%dx, %%ax" : "=a" (result) : "d" (port));
	return result;
}

void io_out_l(u16 port, u32 data){
	__asm__("outl %%ax, %%dx" : : "a" (data), "d" (port));
}

void io_wait(){
	__asm__ ("out %%ax, %%dx" : : "a"(0), "d"(0x80)); // checkpoint addr
}
