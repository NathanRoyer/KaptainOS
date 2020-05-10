#ifndef VGA_H
#define VGA_H

#define	VGA_VIDEO_MEMORY		0xA0000
#define	VGA_AC_INDEX			0x3C0
#define	VGA_AC_WRITE			0x3C0
#define	VGA_AC_READ				0x3C1
#define	VGA_MISC_WRITE			0x3C2
#define	VGA_SEQ_INDEX			0x3C4
#define	VGA_SEQ_DATA			0x3C5
#define	VGA_DAC_READ_INDEX		0x3C7
#define	VGA_DAC_WRITE_INDEX		0x3C8
#define	VGA_DAC_DATA			0x3C9
#define	VGA_MISC_READ			0x3CC
#define	VGA_GC_INDEX 			0x3CE
#define	VGA_GC_DATA 			0x3CF
#define	VGA_CRTC_INDEX			0x3D4
#define	VGA_CRTC_DATA			0x3D5
#define	VGA_INSTAT_READ			0x3DA

#define	VGA_NUM_SEQ_REGS		5
#define	VGA_NUM_CRTC_REGS		25
#define	VGA_NUM_GC_REGS			9
#define	VGA_NUM_AC_REGS			21
#define	VGA_NUM_REGS			(1 + VGA_NUM_SEQ_REGS + VGA_NUM_CRTC_REGS + VGA_NUM_GC_REGS + VGA_NUM_AC_REGS)

#include "../../utils/types.h"

typedef struct vga_mode {
	u8 misc;
	u8 sequencer[VGA_NUM_SEQ_REGS];
	u8 crtc[VGA_NUM_CRTC_REGS];
	u8 graphics_controller[VGA_NUM_GC_REGS];
	u8 attr_controller[VGA_NUM_AC_REGS];
	bool text;
	u8 width, height;
	u8 depth;
} vga_mode_t;

extern vga_mode_t current_vga_mode;

void vga_test();

#endif
