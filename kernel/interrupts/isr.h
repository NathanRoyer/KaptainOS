#ifndef ISR_H
#define ISR_H

#include "../../utils/types.h"

#define PIC_MASTER_CMD 0x0020
#define PIC_MASTER_DAT 0x0021
#define PIC_SLAVE_CMD  0x00A0
#define PIC_SLAVE_DAT  0x00A1

typedef struct {
	// tout est en u32 car on est en 32-bit pm mode, "push" pousse 32 bits à chaque fois

	// segment data
	u32 ds;
	
	// résultat de pusha
	u32 edi;
	u32 esi;
	
	u32 ebp;
	u32 esp;
	
	u32 ebx;
	u32 edx;
	u32 ecx;
	u32 eax;
	
	// infos
	u32 interrupt_id;
	u32 error_code;
	
	// automatiquement pushé par le proc
	u32 eip;
	u32 cs;
	u32 eflags;
	u32 useresp;
	u32 ss;
} registers_t;

void install_isrs();
void isr_handler(registers_t regs);
void irq_handler(registers_t regs);

typedef void (*int_handler_t)(registers_t);

#define IRQ0 32
#define IRQ1 33
#define IRQ2 34
#define IRQ3 35
#define IRQ4 36
#define IRQ5 37
#define IRQ6 38
#define IRQ7 39
#define IRQ8 40
#define IRQ9 41
#define IRQ10 42
#define IRQ11 43
#define IRQ12 44
#define IRQ13 45
#define IRQ14 46
#define IRQ15 47

extern void irq_0();
extern void irq_1();
extern void irq_2();
extern void irq_3();
extern void irq_4();
extern void irq_5();
extern void irq_6();
extern void irq_7();
extern void irq_8();
extern void irq_9();
extern void irq_10();
extern void irq_11();
extern void irq_12();
extern void irq_13();
extern void irq_14();
extern void irq_15();

extern void isr_0();
extern void isr_1();
extern void isr_2();
extern void isr_3();
extern void isr_4();
extern void isr_5();
extern void isr_6();
extern void isr_7();
extern void isr_8();
extern void isr_9();
extern void isr_10();
extern void isr_11();
extern void isr_12();
extern void isr_13();
extern void isr_14();
extern void isr_15();
extern void isr_16();
extern void isr_17();
extern void isr_18();
extern void isr_19();
extern void isr_20();
extern void isr_21();
extern void isr_22();
extern void isr_23();
extern void isr_24();
extern void isr_25();
extern void isr_26();
extern void isr_27();
extern void isr_28();
extern void isr_29();
extern void isr_30();
extern void isr_31();
extern void isr_32();
extern void isr_33();
extern void isr_34();
extern void isr_35();
extern void isr_36();
extern void isr_37();
extern void isr_38();
extern void isr_39();
extern void isr_40();
extern void isr_41();
extern void isr_42();
extern void isr_43();
extern void isr_44();
extern void isr_45();
extern void isr_46();
extern void isr_47();
extern void isr_48();
extern void isr_49();
extern void isr_50();
extern void isr_51();
extern void isr_52();
extern void isr_53();
extern void isr_54();
extern void isr_55();
extern void isr_56();
extern void isr_57();
extern void isr_58();
extern void isr_59();
extern void isr_60();
extern void isr_61();
extern void isr_62();
extern void isr_63();
extern void isr_64();
extern void isr_65();
extern void isr_66();
extern void isr_67();
extern void isr_68();
extern void isr_69();
extern void isr_70();
extern void isr_71();
extern void isr_72();
extern void isr_73();
extern void isr_74();
extern void isr_75();
extern void isr_76();
extern void isr_77();
extern void isr_78();
extern void isr_79();
extern void isr_80();
extern void isr_81();
extern void isr_82();
extern void isr_83();
extern void isr_84();
extern void isr_85();
extern void isr_86();
extern void isr_87();
extern void isr_88();
extern void isr_89();
extern void isr_90();
extern void isr_91();
extern void isr_92();
extern void isr_93();
extern void isr_94();
extern void isr_95();
extern void isr_96();
extern void isr_97();
extern void isr_98();
extern void isr_99();
extern void isr_100();
extern void isr_101();
extern void isr_102();
extern void isr_103();
extern void isr_104();
extern void isr_105();
extern void isr_106();
extern void isr_107();
extern void isr_108();
extern void isr_109();
extern void isr_110();
extern void isr_111();
extern void isr_112();
extern void isr_113();
extern void isr_114();
extern void isr_115();
extern void isr_116();
extern void isr_117();
extern void isr_118();
extern void isr_119();
extern void isr_120();
extern void isr_121();
extern void isr_122();
extern void isr_123();
extern void isr_124();
extern void isr_125();
extern void isr_126();
extern void isr_127();
extern void isr_128();
extern void isr_129();
extern void isr_130();
extern void isr_131();
extern void isr_132();
extern void isr_133();
extern void isr_134();
extern void isr_135();
extern void isr_136();
extern void isr_137();
extern void isr_138();
extern void isr_139();
extern void isr_140();
extern void isr_141();
extern void isr_142();
extern void isr_143();
extern void isr_144();
extern void isr_145();
extern void isr_146();
extern void isr_147();
extern void isr_148();
extern void isr_149();
extern void isr_150();
extern void isr_151();
extern void isr_152();
extern void isr_153();
extern void isr_154();
extern void isr_155();
extern void isr_156();
extern void isr_157();
extern void isr_158();
extern void isr_159();
extern void isr_160();
extern void isr_161();
extern void isr_162();
extern void isr_163();
extern void isr_164();
extern void isr_165();
extern void isr_166();
extern void isr_167();
extern void isr_168();
extern void isr_169();
extern void isr_170();
extern void isr_171();
extern void isr_172();
extern void isr_173();
extern void isr_174();
extern void isr_175();
extern void isr_176();
extern void isr_177();
extern void isr_178();
extern void isr_179();
extern void isr_180();
extern void isr_181();
extern void isr_182();
extern void isr_183();
extern void isr_184();
extern void isr_185();
extern void isr_186();
extern void isr_187();
extern void isr_188();
extern void isr_189();
extern void isr_190();
extern void isr_191();
extern void isr_192();
extern void isr_193();
extern void isr_194();
extern void isr_195();
extern void isr_196();
extern void isr_197();
extern void isr_198();
extern void isr_199();
extern void isr_200();
extern void isr_201();
extern void isr_202();
extern void isr_203();
extern void isr_204();
extern void isr_205();
extern void isr_206();
extern void isr_207();
extern void isr_208();
extern void isr_209();
extern void isr_210();
extern void isr_211();
extern void isr_212();
extern void isr_213();
extern void isr_214();
extern void isr_215();
extern void isr_216();
extern void isr_217();
extern void isr_218();
extern void isr_219();
extern void isr_220();
extern void isr_221();
extern void isr_222();
extern void isr_223();
extern void isr_224();
extern void isr_225();
extern void isr_226();
extern void isr_227();
extern void isr_228();
extern void isr_229();
extern void isr_230();
extern void isr_231();
extern void isr_232();
extern void isr_233();
extern void isr_234();
extern void isr_235();
extern void isr_236();
extern void isr_237();
extern void isr_238();
extern void isr_239();
extern void isr_240();
extern void isr_241();
extern void isr_242();
extern void isr_243();
extern void isr_244();
extern void isr_245();
extern void isr_246();
extern void isr_247();
extern void isr_248();
extern void isr_249();
extern void isr_250();
extern void isr_251();
extern void isr_252();
extern void isr_253();
extern void isr_254();
extern void isr_255();

#endif
