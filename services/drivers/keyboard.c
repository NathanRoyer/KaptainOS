#include "keyboard.h"
#include "../hardware/io.h"
#include "../../kernel/interrupts/isr.h"
#include "../../kernel/malloc.h"
#include "teletype.h"
#include "../../utils/string.h"
#include "vga.h"

// https://www.win.tue.nl/~aeb/linux/kbd/scancodes-1.html

#define DEBUG_KB_CODES false

bool process_handlers(handler_t * current_handler, u8 param, bool low_level);

handler_t * llh_first;
handler_t * hlh_first;

#define KB_DAT_PORT 0x60

#define MOD(SC, ATTR) if (scancode == SC){ mod_state.ATTR = keydown; return true; }
#define SPE(SC, ATTR) if (scancode == SC){ spe_state.ATTR = keydown; return true; }

modifier_keys_t mod_state = { 0 };
special_keys_t spe_state = { 0 };

u8 sc_to_ascii[] = {
	0, 27, '&', '~', '"', '\'', '(', '-', '`', '_', '^', '@', ')', '=', 8, 9,
	'a', 'z', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '^', '$', 10, 0,
	'q', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', 'm', '%', '\\', 0, '*',
	'w', 'x', 'c', 'v', 'b', 'n', ',', ';', ':', '!', 0, '*', 0, ' ',
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	'7', '8', '9', '-', '4', '5', '6', '+', '1', '2', '3', '0', 127
};

u8 sc_to_ascii_shift[] = {
	0, 27, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '+', 8, 9, // 16 caractères dans cette ligne
	'A', 'Z', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '[', ']', 10, 0, // 14
	'Q', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', 'M', '<', '\\', 0, '>', // 14
	'W', 'X', 'C', 'V', 'B', 'N', '?', '.', '/', '|', 0, '*', 0, ' ', // 14
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // 13
	'7', '8', '9', '-', '4', '5', '6', '+', '1', '2', '3', '0', 127 // 13
};

bool shift_lock = false;
bool num_lock = false;

u8 max_sc = 83;

bool last_was_224 = false;
bool last_was_240 = false;

bool process_modifier_key(u8 scancode, bool keydown){
	if (last_was_224){
		/**/ MOD(91, left_gui)
		else MOD(92, right_gui)
		else MOD(56, right_alt)
		else MOD(29, right_ctrl)
	} else if (last_was_240){
		// rien pour l'instant
	} else {
		/**/ MOD(29, left_ctrl)
		else MOD(56, left_alt)
		else MOD(42, left_shift)
		else MOD(54, right_shift)
	}
	
	return false;
}

bool process_special_key(u8 scancode, bool keydown){
	if (last_was_224){
		/**/ SPE(82, insert)
		else SPE(73, page_up)
		else SPE(81, page_down)
		else SPE(72, arrow_up)
		else SPE(75, arrow_left)
		else SPE(80, arrow_down)
		else SPE(77, arrow_right)
	} else if (last_was_240){
		// rien pour l'instant
	} else {
		/**/ if (scancode == 58 && !keydown) shift_lock = !shift_lock;
		else if (scancode == 58 && !keydown) num_lock = !num_lock;
	}
	
	return false;
}

bool allow_low_level = true;
bool allow_high_level = true;

static void keyboard_callback(registers_t regs){
	u8 scancode = io_in_b(KB_DAT_PORT);
	if (DEBUG_KB_CODES){
		print_number(scancode);
		kprint("\n");
		return;
	}
	
	if (!process_handlers(llh_first, scancode, true)) return;
	if (!allow_high_level) return;
	if (scancode == 0xe0){ last_was_224 = true; return; }
	else if (scancode == 0xf0){ last_was_240 = true; return; }
	else for (int i=1;i--;){
	// astuce pour utiliser break ; while aurait nécéssité plus de lignes.
		// le 8e bit de l'octet indique si la touche pressée ou relevée.
		bool keydown = scancode < 128;
		if (!keydown) scancode -= 128;
		
		u8 ascii;
		if ( !process_modifier_key(scancode, keydown) &&
		     !process_special_key(scancode, keydown) ){
			u8 * ascii_map;
			if ((mod_state.right_shift + mod_state.left_shift + shift_lock)%2)
				ascii_map = sc_to_ascii_shift;
			else ascii_map = sc_to_ascii;
			if (!keydown || scancode > max_sc || ascii_map[scancode] == 0) break;
			ascii = ascii_map[scancode];
		} else if (!keydown) break;
		else ascii = 0;
		process_handlers(hlh_first, ascii, false);
	}
	last_was_224 = false;
	last_was_240 = false;
}

bool low_level_switch(keyboard_packet_t p){ return allow_low_level; }
bool high_level_switch(keyboard_packet_t p){ return allow_high_level; }

u32 next_handler_id_ll = 0;
u32 next_handler_id_hl = 0;

u32 register_keyboard_handler(keyboard_handler_t kbh, bool low_level){
	handler_t * current_handler = low_level ? llh_first : hlh_first;
	u32 * next_handler_id = low_level ? &next_handler_id_ll : &next_handler_id_hl;
	while (current_handler->next) current_handler = current_handler->next;
	current_handler->next = malloc(sizeof(handler_t), false);
	current_handler->next->callback = kbh;
	current_handler->next->next = 0;
	current_handler->next->id = (*next_handler_id)++;
	// print_kv_hex("REG", current_handler->next);
	return current_handler->next->id;
}

void remove_keyboard_handler(u32 handler_id, bool low_level){
	handler_t * previous_handler = low_level ? llh_first : hlh_first;
	
	while (previous_handler->next && previous_handler->next->id != handler_id)
		previous_handler = previous_handler->next;
	if (!previous_handler->next) return; // je n'ai pas réussi à éviter la répétition
	
	handler_t * copy = previous_handler->next->next; // peut valoir 0
	free(previous_handler->next); // remove the handler from heap
	previous_handler->next = copy; // join !
}

bool process_handlers(handler_t * current_handler, u8 param, bool low_level){
	keyboard_packet_t packet;
	packet.mod_state = mod_state;
	packet.spe_state = spe_state;
	packet.low_level = low_level;
	packet.value = param;
	while (current_handler){
		// print_kv_hex(" ; handler", current_handler);
		if (current_handler->callback(packet)) current_handler = current_handler->next;
		else return false;
	}
	return true;
}

void init_keyboard(){
	llh_first = malloc(sizeof(handler_t), false);
	llh_first->callback = &low_level_switch;
	llh_first->next = 0;
	llh_first->id = next_handler_id_ll++;
	
	hlh_first = malloc(sizeof(handler_t), false);
	hlh_first->callback = &high_level_switch;
	hlh_first->next = 0;
	hlh_first->id = next_handler_id_hl++; // oublie d'un 'h' ici, découvert le 20/05
	
	register_interrupt_handler(IRQ1, keyboard_callback);
}

string readline_content;
bool readline_done;
u32 readline_cursor_position;
char * c, * readline_prefix;

bool readline_kbh(keyboard_packet_t packet){
	u8 ascii = packet.value;
	if (ascii == 0){ // special key
		int cursor_mod = 0;
		if (packet.spe_state.arrow_left && readline_cursor_position > 0) cursor_mod = -1;
		else if (packet.spe_state.arrow_right && readline_cursor_position < readline_content->length) cursor_mod = 1;
		move_cursor(true, cursor_mod, 0);
		readline_cursor_position += cursor_mod;
		return true;
	}
	if (ascii == 8){
		if (readline_cursor_position) str_remove(readline_content, --readline_cursor_position);
	} else if (ascii == '\n') return (readline_done = true);
	else str_insert(readline_content, readline_cursor_position++, ascii);
	c = str_to_nts(readline_content);
	kprint("\r");
	for (u32 i = 1; i < current_vga_mode.width; i++) kprint(" ");
	kprint("\r");
	kprint(readline_prefix);
	kprint(c);
	free(c);
	return true;
}

char * readline(char * prefix){
	readline_prefix = prefix;
	readline_content = str_from_nts("");
	readline_cursor_position = 0;
	readline_done = false;
	kprint(readline_prefix);
	u32 khid = register_keyboard_handler(&readline_kbh, false);
	while (!readline_done);
	kprint("\n");
	remove_keyboard_handler(khid, false);
	c = str_to_nts(readline_content);
	strfree(readline_content);
	return c;
}



