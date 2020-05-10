#include "mset.h"

void mcopy(u8 * source, u8 * dest, u32 count){
	for (u32 i = 0; i < count; i++) *(dest + i) = *(source + i);
}

void mset(u8 * dest, u8 value, u32 count){
	for (;count; count--) *(dest++) = value;
}
