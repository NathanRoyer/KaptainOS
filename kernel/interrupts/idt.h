#ifndef IDT_H
#define IDT_H

#include "../../utils/types.h"

#define KERNEL_CODE_SEGMENT 0x08
#define IDT_ENTRIES 256

typedef struct {
	u16 handler_low_address;
	u16 segment_selector;
	u8 unused; // toujours 0
	u8 flags;
	/*
	 * bits :
	 *   7     - est présent
	 *   6 & 5 - privilège = valeur de 0 à 3, correspondant plus ou moins aux rings
	 *   4     - segment destinataires, 0 pour les interruptions
	 *   3 à 0 - type d'interrupt gate : 16 ou 32 bit, piège ou pas, 80386 ou 80286
	 */
	u16 handler_high_address;
} __attribute__((packed)) idt_gate_t;


// liste de idt_gate :
typedef struct {
	u16 limit;
	u32 base;
} __attribute__((packed)) idt_register_t;


idt_gate_t idt[IDT_ENTRIES];
idt_register_t idt_reg;

void set_idt_gate(int id, u32 handler);
void set_idt();

#endif
