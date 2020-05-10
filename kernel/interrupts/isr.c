/*
 * KaptainOS kernel
 * 
 * Interrupt Service Routine
 * 
 */
#include "isr.h"
#include "idt.h"
#include "../../services/drivers/teletype.h"
#include "../../services/hardware/io.h"
#include "../../utils/conv.h"

int_handler_t handlers[256];
u8 handler_present[256] = { 0 };

char * interrupt_name[] = {
	"Divide error",
	"Debug",
	"Non-maskable interrupt",
	"Breakpoint",
	"Overflow",
	"Bounds check",
	"Invalid opcode",
	"Device not available",
	"Double fault",
	"Coprocessor segment overrun",
	"Invalid TSS",
	"Segment not present",
	"Stack segment",
	"General protection",
	"Page fault",
	"Reserved",
	"Floating-point error",
	"Alignment check",
	"Machine check",
	"SIMD floating point error"
};

u8 max_interrupt_name = 19;

void register_interrupt_handler(u8 id, int_handler_t handler){
	handler_present[id] = 1;
	handlers[id] = handler;
}

#define FROM_ISR 0
#define FROM_IRQ 1

void debug_handler(registers_t regs, u8 origin){
	kprint_color("--- Unhandled Interruption ! ---\n", 0b00000100);
	kprint("Origin : ");
	if (origin == FROM_ISR) kprint("Software\n");
	else if (origin == FROM_IRQ) kprint("Hardware\n");
	
	kprint("- Interrupt details -\n");
	print_kv("ID", regs.interrupt_id);
	print_kv("Error code", regs.error_code);
	if (regs.interrupt_id <= max_interrupt_name){
		char * int_name = interrupt_name[regs.interrupt_id];
		kprint("Interrupt name : ");
		kprint(int_name);
		kprint("\n");
	} else kprint("Unknown interrupt ID\n");
	
	kprint("- Result of pusha -\n");
	print_kv("EDI", regs.edi);
	print_kv("ESI", regs.esi);
	print_kv("EBP", regs.ebp);
	print_kv("ESP", regs.esp);
	print_kv("EAX", regs.eax);
	print_kv("EBX", regs.ebx);
	print_kv("ECX", regs.ecx);
	print_kv("EDX", regs.edx);
	
	kprint("- Other infos -\n");
	print_kv("Data segment", regs.ds);
	print_kv("EIP", regs.eip);
	print_kv("CS", regs.cs);
	print_kv("EFLAGS", regs.eflags);
	print_kv("USERESP", regs.useresp);
	print_kv("SS", regs.ss);
	
	kprint_color("--- End of interruption description. ---\n", 0b00000100);
	
	//while(1);
}

void isr_handler(registers_t regs){
	kprint("\nHardware Interrupt !\n");
	if (handler_present[regs.interrupt_id]) handlers[regs.interrupt_id](regs);
	else debug_handler(regs, FROM_ISR);
}

// rappel d'isr.h :
// PIC_MASTER_CMD = 0x0020
// PIC_MASTER_DAT = 0x0021
// PIC_SLAVE_CMD  = 0x00A0
// PIC_SLAVE_DAT  = 0x00A1

void irq_handler(registers_t regs){
	if (regs.error_code >= 7) io_out_b(PIC_SLAVE_CMD, 0x20); // slave End of interrupt
	io_out_b(PIC_MASTER_CMD, 0x20); // master End of interrupt
	
	if (handler_present[regs.interrupt_id]) handlers[regs.interrupt_id](regs);
	else debug_handler(regs, FROM_IRQ);
}

void install_isrs(){
	// IRQS
	
	// Remappage du PIC
	// séquence d'initialisation générique du PIC trouvée sur wiki.osdev.org/PIC
	// = Programmable Interrupt Controller
	
	// sauvegarde de l'etat initial
	u8 a1, a2;
	a1 = io_in_b(PIC_MASTER_DAT);
	a2 = io_in_b(PIC_SLAVE_DAT);
	
	// reset
	io_out_b(PIC_MASTER_CMD, 0x11);
	io_wait();
	io_out_b(PIC_SLAVE_CMD,  0x11);
	io_wait();
	// apparemment les "vector offsets" (bornes de relocalisation ?)
	io_out_b(PIC_MASTER_DAT, 0x20);
	io_wait();
	io_out_b(PIC_SLAVE_DAT,  0x28);
	io_wait();
	// relations master/slave
	io_out_b(PIC_MASTER_DAT, 0x04);
	io_wait();
	io_out_b(PIC_SLAVE_DAT,  0x02);
	io_wait();
	// environment : 80x86
	io_out_b(PIC_MASTER_DAT, 0x01);
	io_wait();
	io_out_b(PIC_SLAVE_DAT,  0x01);
	io_wait();
	// end of transmission
	io_out_b(PIC_MASTER_DAT, 0x0);
	io_wait();
	io_out_b(PIC_SLAVE_DAT,  0x0);
	io_wait();
	// restauration
	io_out_b(PIC_MASTER_DAT, a1);
	io_wait();
	io_out_b(PIC_SLAVE_DAT, a2);
	io_wait();
	// fin d'initialisation
	
	set_idt_gate(32, (u32)irq_0);
	set_idt_gate(33, (u32)irq_1);
	set_idt_gate(34, (u32)irq_2);
	set_idt_gate(35, (u32)irq_3);
	set_idt_gate(36, (u32)irq_4);
	set_idt_gate(37, (u32)irq_5);
	set_idt_gate(38, (u32)irq_6);
	set_idt_gate(39, (u32)irq_7);
	set_idt_gate(40, (u32)irq_8);
	set_idt_gate(41, (u32)irq_9);
	set_idt_gate(42, (u32)irq_10);
	set_idt_gate(43, (u32)irq_11);
	set_idt_gate(44, (u32)irq_12);
	set_idt_gate(45, (u32)irq_13);
	set_idt_gate(46, (u32)irq_14);
	set_idt_gate(47, (u32)irq_15);
	
	
	// ISRS
	set_idt_gate(0, (u32)isr_0);
	set_idt_gate(1, (u32)isr_1);
	set_idt_gate(2, (u32)isr_2);
	set_idt_gate(3, (u32)isr_3);
	set_idt_gate(4, (u32)isr_4);
	set_idt_gate(5, (u32)isr_5);
	set_idt_gate(6, (u32)isr_6);
	set_idt_gate(7, (u32)isr_7);
	set_idt_gate(8, (u32)isr_8);
	set_idt_gate(9, (u32)isr_9);
	set_idt_gate(10, (u32)isr_10);
	set_idt_gate(11, (u32)isr_11);
	set_idt_gate(12, (u32)isr_12);
	set_idt_gate(13, (u32)isr_13);
	set_idt_gate(14, (u32)isr_14);
	set_idt_gate(15, (u32)isr_15);
	set_idt_gate(16, (u32)isr_16);
	set_idt_gate(17, (u32)isr_17);
	set_idt_gate(18, (u32)isr_18);
	set_idt_gate(19, (u32)isr_19);
	set_idt_gate(20, (u32)isr_20);
	set_idt_gate(21, (u32)isr_21);
	set_idt_gate(22, (u32)isr_22);
	set_idt_gate(23, (u32)isr_23);
	set_idt_gate(24, (u32)isr_24);
	set_idt_gate(25, (u32)isr_25);
	set_idt_gate(26, (u32)isr_26);
	set_idt_gate(27, (u32)isr_27);
	set_idt_gate(28, (u32)isr_28);
	set_idt_gate(29, (u32)isr_29);
	set_idt_gate(30, (u32)isr_30);
	set_idt_gate(31, (u32)isr_31);
	// IRQS : from 32 to 47
	set_idt_gate(48, (u32)isr_48);
	set_idt_gate(49, (u32)isr_49);
	set_idt_gate(50, (u32)isr_50);
	set_idt_gate(51, (u32)isr_51);
	set_idt_gate(52, (u32)isr_52);
	set_idt_gate(53, (u32)isr_53);
	set_idt_gate(54, (u32)isr_54);
	set_idt_gate(55, (u32)isr_55);
	set_idt_gate(56, (u32)isr_56);
	set_idt_gate(57, (u32)isr_57);
	set_idt_gate(58, (u32)isr_58);
	set_idt_gate(59, (u32)isr_59);
	set_idt_gate(60, (u32)isr_60);
	set_idt_gate(61, (u32)isr_61);
	set_idt_gate(62, (u32)isr_62);
	set_idt_gate(63, (u32)isr_63);
	set_idt_gate(64, (u32)isr_64);
	set_idt_gate(65, (u32)isr_65);
	set_idt_gate(66, (u32)isr_66);
	set_idt_gate(67, (u32)isr_67);
	set_idt_gate(68, (u32)isr_68);
	set_idt_gate(69, (u32)isr_69);
	set_idt_gate(70, (u32)isr_70);
	set_idt_gate(71, (u32)isr_71);
	set_idt_gate(72, (u32)isr_72);
	set_idt_gate(73, (u32)isr_73);
	set_idt_gate(74, (u32)isr_74);
	set_idt_gate(75, (u32)isr_75);
	set_idt_gate(76, (u32)isr_76);
	set_idt_gate(77, (u32)isr_77);
	set_idt_gate(78, (u32)isr_78);
	set_idt_gate(79, (u32)isr_79);
	set_idt_gate(80, (u32)isr_80);
	set_idt_gate(81, (u32)isr_81);
	set_idt_gate(82, (u32)isr_82);
	set_idt_gate(83, (u32)isr_83);
	set_idt_gate(84, (u32)isr_84);
	set_idt_gate(85, (u32)isr_85);
	set_idt_gate(86, (u32)isr_86);
	set_idt_gate(87, (u32)isr_87);
	set_idt_gate(88, (u32)isr_88);
	set_idt_gate(89, (u32)isr_89);
	set_idt_gate(90, (u32)isr_90);
	set_idt_gate(91, (u32)isr_91);
	set_idt_gate(92, (u32)isr_92);
	set_idt_gate(93, (u32)isr_93);
	set_idt_gate(94, (u32)isr_94);
	set_idt_gate(95, (u32)isr_95);
	set_idt_gate(96, (u32)isr_96);
	set_idt_gate(97, (u32)isr_97);
	set_idt_gate(98, (u32)isr_98);
	set_idt_gate(99, (u32)isr_99);
	set_idt_gate(100, (u32)isr_100);
	set_idt_gate(101, (u32)isr_101);
	set_idt_gate(102, (u32)isr_102);
	set_idt_gate(103, (u32)isr_103);
	set_idt_gate(104, (u32)isr_104);
	set_idt_gate(105, (u32)isr_105);
	set_idt_gate(106, (u32)isr_106);
	set_idt_gate(107, (u32)isr_107);
	set_idt_gate(108, (u32)isr_108);
	set_idt_gate(109, (u32)isr_109);
	set_idt_gate(110, (u32)isr_110);
	set_idt_gate(111, (u32)isr_111);
	set_idt_gate(112, (u32)isr_112);
	set_idt_gate(113, (u32)isr_113);
	set_idt_gate(114, (u32)isr_114);
	set_idt_gate(115, (u32)isr_115);
	set_idt_gate(116, (u32)isr_116);
	set_idt_gate(117, (u32)isr_117);
	set_idt_gate(118, (u32)isr_118);
	set_idt_gate(119, (u32)isr_119);
	set_idt_gate(120, (u32)isr_120);
	set_idt_gate(121, (u32)isr_121);
	set_idt_gate(122, (u32)isr_122);
	set_idt_gate(123, (u32)isr_123);
	set_idt_gate(124, (u32)isr_124);
	set_idt_gate(125, (u32)isr_125);
	set_idt_gate(126, (u32)isr_126);
	set_idt_gate(127, (u32)isr_127);
	set_idt_gate(128, (u32)isr_128);
	set_idt_gate(129, (u32)isr_129);
	set_idt_gate(130, (u32)isr_130);
	set_idt_gate(131, (u32)isr_131);
	set_idt_gate(132, (u32)isr_132);
	set_idt_gate(133, (u32)isr_133);
	set_idt_gate(134, (u32)isr_134);
	set_idt_gate(135, (u32)isr_135);
	set_idt_gate(136, (u32)isr_136);
	set_idt_gate(137, (u32)isr_137);
	set_idt_gate(138, (u32)isr_138);
	set_idt_gate(139, (u32)isr_139);
	set_idt_gate(140, (u32)isr_140);
	set_idt_gate(141, (u32)isr_141);
	set_idt_gate(142, (u32)isr_142);
	set_idt_gate(143, (u32)isr_143);
	set_idt_gate(144, (u32)isr_144);
	set_idt_gate(145, (u32)isr_145);
	set_idt_gate(146, (u32)isr_146);
	set_idt_gate(147, (u32)isr_147);
	set_idt_gate(148, (u32)isr_148);
	set_idt_gate(149, (u32)isr_149);
	set_idt_gate(150, (u32)isr_150);
	set_idt_gate(151, (u32)isr_151);
	set_idt_gate(152, (u32)isr_152);
	set_idt_gate(153, (u32)isr_153);
	set_idt_gate(154, (u32)isr_154);
	set_idt_gate(155, (u32)isr_155);
	set_idt_gate(156, (u32)isr_156);
	set_idt_gate(157, (u32)isr_157);
	set_idt_gate(158, (u32)isr_158);
	set_idt_gate(159, (u32)isr_159);
	set_idt_gate(160, (u32)isr_160);
	set_idt_gate(161, (u32)isr_161);
	set_idt_gate(162, (u32)isr_162);
	set_idt_gate(163, (u32)isr_163);
	set_idt_gate(164, (u32)isr_164);
	set_idt_gate(165, (u32)isr_165);
	set_idt_gate(166, (u32)isr_166);
	set_idt_gate(167, (u32)isr_167);
	set_idt_gate(168, (u32)isr_168);
	set_idt_gate(169, (u32)isr_169);
	set_idt_gate(170, (u32)isr_170);
	set_idt_gate(171, (u32)isr_171);
	set_idt_gate(172, (u32)isr_172);
	set_idt_gate(173, (u32)isr_173);
	set_idt_gate(174, (u32)isr_174);
	set_idt_gate(175, (u32)isr_175);
	set_idt_gate(176, (u32)isr_176);
	set_idt_gate(177, (u32)isr_177);
	set_idt_gate(178, (u32)isr_178);
	set_idt_gate(179, (u32)isr_179);
	set_idt_gate(180, (u32)isr_180);
	set_idt_gate(181, (u32)isr_181);
	set_idt_gate(182, (u32)isr_182);
	set_idt_gate(183, (u32)isr_183);
	set_idt_gate(184, (u32)isr_184);
	set_idt_gate(185, (u32)isr_185);
	set_idt_gate(186, (u32)isr_186);
	set_idt_gate(187, (u32)isr_187);
	set_idt_gate(188, (u32)isr_188);
	set_idt_gate(189, (u32)isr_189);
	set_idt_gate(190, (u32)isr_190);
	set_idt_gate(191, (u32)isr_191);
	set_idt_gate(192, (u32)isr_192);
	set_idt_gate(193, (u32)isr_193);
	set_idt_gate(194, (u32)isr_194);
	set_idt_gate(195, (u32)isr_195);
	set_idt_gate(196, (u32)isr_196);
	set_idt_gate(197, (u32)isr_197);
	set_idt_gate(198, (u32)isr_198);
	set_idt_gate(199, (u32)isr_199);
	set_idt_gate(200, (u32)isr_200);
	set_idt_gate(201, (u32)isr_201);
	set_idt_gate(202, (u32)isr_202);
	set_idt_gate(203, (u32)isr_203);
	set_idt_gate(204, (u32)isr_204);
	set_idt_gate(205, (u32)isr_205);
	set_idt_gate(206, (u32)isr_206);
	set_idt_gate(207, (u32)isr_207);
	set_idt_gate(208, (u32)isr_208);
	set_idt_gate(209, (u32)isr_209);
	set_idt_gate(210, (u32)isr_210);
	set_idt_gate(211, (u32)isr_211);
	set_idt_gate(212, (u32)isr_212);
	set_idt_gate(213, (u32)isr_213);
	set_idt_gate(214, (u32)isr_214);
	set_idt_gate(215, (u32)isr_215);
	set_idt_gate(216, (u32)isr_216);
	set_idt_gate(217, (u32)isr_217);
	set_idt_gate(218, (u32)isr_218);
	set_idt_gate(219, (u32)isr_219);
	set_idt_gate(220, (u32)isr_220);
	set_idt_gate(221, (u32)isr_221);
	set_idt_gate(222, (u32)isr_222);
	set_idt_gate(223, (u32)isr_223);
	set_idt_gate(224, (u32)isr_224);
	set_idt_gate(225, (u32)isr_225);
	set_idt_gate(226, (u32)isr_226);
	set_idt_gate(227, (u32)isr_227);
	set_idt_gate(228, (u32)isr_228);
	set_idt_gate(229, (u32)isr_229);
	set_idt_gate(230, (u32)isr_230);
	set_idt_gate(231, (u32)isr_231);
	set_idt_gate(232, (u32)isr_232);
	set_idt_gate(233, (u32)isr_233);
	set_idt_gate(234, (u32)isr_234);
	set_idt_gate(235, (u32)isr_235);
	set_idt_gate(236, (u32)isr_236);
	set_idt_gate(237, (u32)isr_237);
	set_idt_gate(238, (u32)isr_238);
	set_idt_gate(239, (u32)isr_239);
	set_idt_gate(240, (u32)isr_240);
	set_idt_gate(241, (u32)isr_241);
	set_idt_gate(242, (u32)isr_242);
	set_idt_gate(243, (u32)isr_243);
	set_idt_gate(244, (u32)isr_244);
	set_idt_gate(245, (u32)isr_245);
	set_idt_gate(246, (u32)isr_246);
	set_idt_gate(247, (u32)isr_247);
	set_idt_gate(248, (u32)isr_248);
	set_idt_gate(249, (u32)isr_249);
	set_idt_gate(250, (u32)isr_250);
	set_idt_gate(251, (u32)isr_251);
	set_idt_gate(252, (u32)isr_252);
	set_idt_gate(253, (u32)isr_253);
	set_idt_gate(254, (u32)isr_254);
	set_idt_gate(255, (u32)isr_255);
	
	set_idt();
}
