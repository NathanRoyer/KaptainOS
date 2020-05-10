/*
 * KaptainOS kernel
 */

#include "../hardware/io.h"
#include "teletype.h"
#include "../../utils/mset.h"
#include "../../kernel/malloc.h"
#include "vga.h"

#define VIDEO_ADDRESS 0xb8000

#define REG_SCREEN_CTRL 0x3d4
#define REG_SCREEN_DATA 0x3d5

char PRINT_COLOR = WHITE_ON_BLACK;

int get_cursor_offset(){
	io_out_b(REG_SCREEN_CTRL, 14);
	int offset = io_in_b(REG_SCREEN_DATA) << 8;
	io_out_b(REG_SCREEN_CTRL, 15);
	offset += io_in_b(REG_SCREEN_DATA);
	return offset * 2;
}

int set_cursor_offset(int offset){
	offset /= 2;
	io_out_b(REG_SCREEN_CTRL, 14);
	io_out_b(REG_SCREEN_DATA, (unsigned char)(offset >> 8));
	io_out_b(REG_SCREEN_CTRL, 15);
	io_out_b(REG_SCREEN_DATA, (unsigned char)(offset & 0xff));
}

void teletype_disable_cursor(){
	io_out_b(REG_SCREEN_CTRL, 0x0A);
	io_out_b(REG_SCREEN_DATA, 0x20);
}

int get_offset(int col, int row){ return 2 * (row * current_vga_mode.width + col); }
int get_offset_row(int offset){ return offset / (2 * current_vga_mode.width); }
int get_offset_col(int offset){ return (offset - (get_offset_row(offset)*2*current_vga_mode.width))/2; }

int print_char(char c, int col, int row, char attr){
	unsigned char *vidmem = (unsigned char *) VIDEO_ADDRESS;
	if (!attr) attr = WHITE_ON_BLACK;
	if (col >= current_vga_mode.width || row >= current_vga_mode.height){
		clear_screen();
		return print_char(c, 0, 0, attr);
	}
	int offset;
	if (col >=0 && row >= 0) offset = get_offset(col, row);
	else offset = get_cursor_offset();

	if (c == '\n'){
		row = get_offset_row(offset);
		offset = get_offset(0, row+1);
	} else if (c == '\r'){
		row = get_offset_row(offset);
		offset = get_offset(0, row);
	} else if (c == 8){ // backspace
		row = get_offset_row(offset);
		col = get_offset_col(offset);
		if (col == 0) return offset;
		offset -= 2;
		vidmem[offset] = ' ';
		vidmem[offset+1] = attr;
	} else {
		vidmem[offset] = c;
		vidmem[offset+1] = attr;
		offset += 2;
	}
	set_cursor_offset(offset);
	return offset;
}

void move_cursor(unsigned char relative, int horizontal, int vertical){
	if (relative){
		int offset = get_cursor_offset();
		set_cursor_offset(get_offset(get_offset_col(offset)+horizontal, get_offset_row(offset)+vertical));
	} else set_cursor_offset(get_offset(horizontal, vertical));
}

void kprint_at(char * message, int col, int row, char attr){
	int offset;
	if (col >= 0 && row >= 0) offset = get_offset(col, row);
	else {
		offset = get_cursor_offset();
		row = get_offset_row(offset);
		col= get_offset_col(offset);
	}

	for (int i = 0; message[i]; i++){
		offset = print_char(message[i], col, row, attr);
		row = get_offset_row(offset);
		col = get_offset_col(offset);
	}
}

unsigned char get_char_at(int col, int row){
	if (col >= current_vga_mode.width || row >= current_vga_mode.height || col < 0 || row < 0) return 0;
	unsigned char *vidmem = (unsigned char *) VIDEO_ADDRESS;
	int offset = get_offset(col, row);
	if (vidmem[offset]) return vidmem[offset];
	else return ' ';
}

void set_print_color(char attr){
	PRINT_COLOR = attr;
}

void kprint(char * message){
	kprint_at(message, -1, -1, PRINT_COLOR);
}

void kprint_color(char * message, char attr){
	kprint_at(message, -1, -1, attr);
}

void print_number_base(u32 base, unsigned int n){
	if (base == 16) kprint("0x");
	else if (base == 2) kprint("0b");
	char char_[33] = { 0 };
	int_to_string(n, base, char_);
	kprint(char_);
}

void print_number(unsigned int n){
	print_number_base(10, n);
}

void print_hex(unsigned int n){
	print_number_base(16, n);
}

void print_bin(unsigned int n){
	print_number_base(2, n);
}

void print_kv_base(u32 base, char * key, u32 value){
	kprint(key);
	kprint(": ");
	print_number_base(base, value);
	kprint("\n");
}

void print_kv(char * key, u32 value){
	print_kv_base(10, key, value);
}

void print_kv_hex(char * key, u32 value){
	print_kv_base(16, key, value);
}

void print_kv_bin(char * key, u32 value){
	print_kv_base(2, key, value);
}

void clear_screen(){
	unsigned char *vidmem = (unsigned char *) VIDEO_ADDRESS;
	mset(VIDEO_ADDRESS, 0, 2 * current_vga_mode.height * current_vga_mode.width);
	set_cursor_offset(0);
}
