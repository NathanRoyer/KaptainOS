#include "cpu_timer.h"
#include "teletype.h"
#include "../hardware/io.h"
#include "../../utils/types.h"
#include "../../kernel/interrupts/isr.h"

#define PIT_INTERNAL_FREQ 1193180
#define PIT_CMD_PORT 0x43
#define PIT_DAT_PORT 0x40

u32 tick = 0;
u32 current_freq = 1;

static void timer_callback(registers_t regs){
	tick++;
}

u32 get_tick(){
	return tick*1000 / current_freq; // return tick in miliseconds
}

void init_cpu_timer(u32 freq){
	register_interrupt_handler(IRQ0, timer_callback);
	
	if (freq < 19){ // PIT_INTERNAL_FREQ / 18 ne tient pas dans 16 bits
		kprint("PIT FREQUENCY TOO LOW !\n");
		while (1);
	}
	
	// set the Programmable Interval Timer frequency
	u16 divisor = PIT_INTERNAL_FREQ / freq;
	u8 low = u16_low(divisor);
	u8 high = u16_high(divisor);
	io_out_b(PIT_CMD_PORT, 0x36); // set_freq command
	io_wait();
	io_out_b(PIT_DAT_PORT, low);
	io_wait();
	io_out_b(PIT_DAT_PORT, high);
	io_wait();
	current_freq = freq;
}
