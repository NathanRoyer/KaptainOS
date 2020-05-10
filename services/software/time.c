#include "time.h"
#include "../drivers/cpu_timer.h"

void sleep(u32 ms){
	u32 wanted = get_tick() + ms;
	while (wanted > get_tick());
}
