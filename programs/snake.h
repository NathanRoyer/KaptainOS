#include "../utils/types.h"

void snake_init();

typedef struct snake_dot snake_dot_t;
typedef struct snake_dot {
	u32 x;
	u32 y;
	snake_dot_t * next;
} snake_dot_t;
