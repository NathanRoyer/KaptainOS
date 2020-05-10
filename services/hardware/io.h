#ifndef IO_H
#define IO_H

#include "../../utils/types.h"

u8 io_in_b(u16 port);
void io_out_b(u16 port, unsigned char data);
u16 io_in_w(u16 port);
void io_out_w(u16 port, u16 data);
void io_wait();

#endif
