#ifndef TYPES_H
#define TYPES_H

#define NULL 0
#define false 0
#define true 1

typedef unsigned long  u64;
typedef          long  s64;
typedef unsigned int   u32;
typedef          int   s32;
typedef unsigned short u16;
typedef          short s16;
typedef unsigned char  u8;
typedef          char  s8;
typedef unsigned char  bool;

#define u32_low(address) (u16)((address) & 0xffff)
#define u32_high(address) (u16)((address >> 16) & 0xffff)

#define u16_low(address) (u8)((address) & 0xff)
#define u16_high(address) (u8)((address >> 8) & 0xff)

#include "string.h"

#endif
