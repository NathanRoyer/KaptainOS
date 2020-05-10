#include "../services/drivers/teletype.h"
#include "../services/drivers/vga.h"
#include "../services/drivers/keyboard.h"
#include "../services/drivers/cpu_timer.h"
#include "snake.h"

#define SPAWN_X 10
#define SPAWN_Y 5

u32 width, height, screen_height;
char * terrain_nts;
bool program_end, restart_question;
s8 vx, vy;
u32 x, y;
u32 bck_x, bck_y, tmp_x, tmp_y;
char perso_charset[] = { 0xDF, 0xDC, 0xDB };
u32 snake_length;
u32 high_score = 0;
snake_dot_t *snake_end, *snake_head;
u32 fruit_x, fruit_y;
u8 blocks_to_spawn;
unsigned char * _cstr = "_";

u8 random(){
	u32 n = get_tick();
	u8 * number = &n;
	u8 out = *(number + (n%2));
	out |= *(number + (n%4+3));
	out ^= *(number + (n%3+4));
	return out;
}

bool snake_kb_cb(keyboard_packet_t packet){
	if (restart_question){
		_cstr[0] = packet.value;
		kprint_at(_cstr, 0, 0, 0x04);
		/**/ if (packet.value == 'y' ){ program_end = false; }
		else if (packet.value == 'n' ){ program_end = false; restart_question = false; }
	} else {
		/**/ if (packet.spe_state.arrow_down ){ vx =  0; vy =  1; }
		else if (packet.spe_state.arrow_up   ){ vx =  0; vy = -1; }
		else if (packet.spe_state.arrow_left ){ vx = -1; vy =  0; }
		else if (packet.spe_state.arrow_right){ vx =  1; vy =  0; }
		else if (packet.value == 27) program_end = true;
	}
	return true;
}

void set(u32 x, u32 y, char * charset, u8 color){
	unsigned char c = get_char_at(x, y/2+1);
	if (c != charset[y%2]) c = c == ' ' ? charset[y%2] : charset[2];
	_cstr[0] = c;
	kprint_at(_cstr, x, y/2+1, color);
}

void lengthen_snake(u32 x, u32 y){
	snake_end->next = malloc(sizeof(snake_dot_t), false);
	snake_end->next->x = x;
	snake_end->next->y = y;
	snake_end->next->next = 0;
	snake_end = snake_end->next;
	snake_length++;
}

void reset_fruit(){
	fruit_x = random()%width;
	fruit_y = random()%height;
	lengthen_snake(snake_end->x, snake_end->y);
}

void reset_variables(){
	program_end = false;
	restart_question = false;
	vx = 1;
	vy = 0;
	x = SPAWN_X;
	y = SPAWN_Y;
	fruit_x = 10;
	fruit_y = 10;
	snake_length = 1;
	blocks_to_spawn = 5;
}

void reduce_snake(){
	snake_dot_t * next, * current_dot = snake_head->next;
	while (current_dot){
		next = current_dot->next;
		free(current_dot);
		current_dot = next;
	}
	snake_head->next = 0;
	snake_end = snake_head;
}

void snake_init(){
	teletype_disable_cursor();
	reset_variables();

	width = current_vga_mode.width;
	screen_height = current_vga_mode.height - 1;
	height = screen_height * 2;
	snake_head = (snake_end = malloc(sizeof(snake_dot_t), false));
	snake_head->x = x;
	snake_head->y = y;
	
	u32 khid = register_keyboard_handler(&snake_kb_cb, false);
	while (!program_end){
		if (blocks_to_spawn){
			blocks_to_spawn--;
			lengthen_snake(SPAWN_X, SPAWN_Y);
		}
		// sleep() inutile: voir le fameux bug de SpaceInvaders
		u32 wanted = get_tick() + 50;
		
		// stop at walls :
		/*if ((x == 0 && vx < 0) || (x == width-1 && vx > 0)) vx = 0;
		if ((y == 0 && vy < 0) || (y == height-1 && vy > 0)) vy = 0;*/
		x += vx;
		y += vy;
		if (x == fruit_x && y == fruit_y) reset_fruit();
		else {
			char c = get_char_at(x, y/2+1);
			if (c == 0 || c == perso_charset[2] || (y%2 == 0 && c == perso_charset[0]) || (y%2 == 1 && c == perso_charset[1])){
				if (high_score < snake_length) high_score = snake_length;
				clear_screen();
				reduce_snake();
				kprint_at("Game Over !", width/2 - 5, screen_height/2, 0x04);
				kprint_at("Restart ? press y/n", width/2 - 9, screen_height-1, 0x04);
				restart_question = true;
				program_end = true;
				while(program_end);
				if (restart_question){
					reset_variables();
				} else break;
			}
		}
		
		clear_screen();
		set_print_color(0x70);
		if (high_score < snake_length) kprint("KaptainOS Snake | New High Score !");
		else {
			kprint("KaptainOS Snake | High Score: ");
			print_number(high_score);
		}
		kprint(" | head: ");
		print_number(snake_head->x);
		kprint(", ");
		print_number(snake_head->y);
		kprint(" | fruit: ");
		print_number(fruit_x);
		kprint(", ");
		print_number(fruit_y);
		kprint(" | Length: ");
		print_number(snake_length);
		for (u8 i = get_cursor_offset() / 2; i < current_vga_mode.width; i++) kprint(" ");
		set_print_color(WHITE_ON_BLACK);
		
		tmp_x = x;
		tmp_y = y;
		snake_dot_t * current_dot = snake_head;
		while (current_dot){
			bck_x = current_dot->x;
			bck_y = current_dot->y;
			current_dot->x = tmp_x;
			current_dot->y = tmp_y;
			set(tmp_x, tmp_y, perso_charset, WHITE_ON_BLACK);
			tmp_x = bck_x;
			tmp_y = bck_y;
			current_dot = current_dot->next;
		}
		
		set(fruit_x, fruit_y, perso_charset, 0x04);
		
		while (wanted > get_tick()); // take the loop time use into account
	}
	if (high_score < snake_length) high_score = snake_length;
	remove_keyboard_handler(khid, false);
	reduce_snake();
	free(snake_head);
	clear_screen();
	kprint("\n");
	
	return;
}
