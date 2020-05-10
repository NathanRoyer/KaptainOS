#ifndef KEYBOARD_H
#define KEYBOARD_H

#include "../../utils/types.h"


typedef struct handler handler_t;

typedef struct {
	u8 left_shift;
	u8 left_ctrl;
	u8 left_alt;
	u8 left_gui;
	
	u8 right_shift;
	u8 right_ctrl;
	u8 right_alt;
	u8 right_gui;
	
	u8 fn;
} modifier_keys_t;

typedef struct {
	u8 insert;
	u8 page_up;
	u8 page_down;
	u8 arrow_up;
	u8 arrow_down;
	u8 arrow_left;
	u8 arrow_right;
} special_keys_t;

typedef struct keyboard_packet {
	u8 value;
	bool low_level;
	modifier_keys_t mod_state;
	special_keys_t spe_state;
} keyboard_packet_t;

typedef bool (*keyboard_handler_t)(keyboard_packet_t packet);

typedef struct handler {
	keyboard_handler_t callback;
	handler_t * next;
	u32 id;
} handler_t;

void init_keyboard();
u32 register_keyboard_handler(keyboard_handler_t kbh, bool low_level);
void remove_keyboard_handler(u32 handler_id, bool low_level);
void wait_for_keyboard_input();
char * readline(char * prefix);

#endif
