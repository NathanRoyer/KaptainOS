#include "console.h"
#include "../utils/string.h"
#include "../utils/conv.h"
#include "../kernel/malloc.h"
#include "../services/drivers/vga.h"
#include "../services/drivers/keyboard.h"
#include "../services/hardware/pci.h"
#include "../services/drivers/ahci.h"
#include "snake.h"

string str_space;
string NULLSTR;
s8 kb_hook[] = { -1, -1 };

bool kbdbg_cb(keyboard_packet_t packet){
	print_kv(packet.low_level ? "LL" : "HL", packet.value);
	return true;
}

#define PRGM_EQU(name) if (str_equ_nts(program, name))

string argument(u32 arg_num, string arguments, bool * error){
	u32 count = 0, search_begin = 0;
	s32 first_space;
	string tmpstr;
	while (1){
		tmpstr = substr(arguments, search_begin, -1);
		first_space = strstr(str_space, tmpstr);
		strfree(tmpstr);
		if (count++ == arg_num) break;
		if (first_space == -1){
			if (error) *error = true;
			return strcpy(NULLSTR);
		}
		search_begin += first_space + 1;
	}
	return substr(arguments, search_begin, first_space);
}

void prgm_lookup(string program, string arguments){
	PRGM_EQU("help"){
		kprint("Available commands:         Description:\n\n");
		kprint("   help                     print this message;\n");
		kprint("   echo [text]              print text in the console;\n");
		kprint("   math add [num1] [num2]   print num1 + num2;\n");
		kprint("        sub [num1] [num2]   print num1 - num2;\n");
		kprint("        mul [num1] [num2]   print num1 * num2;\n");
		kprint("        div [num1] [num2]   print num1 / num2;\n");
		kprint("   io out [port] [value]    write value to io port;\n");
		kprint("      in  [port]            read value from io port;\n");
		kprint("   heap                     print heap blocks;\n");
		kprint("   clear                    empty the screen;\n");
		kprint("   snake                    play a snake game;\n");
		kprint("   debug_chars              print all characters from to 256;\n");
		kprint("   debug_colors             test all 256 text-mode colors;\n");
		kprint("   debug_keyboard           test the keyboard low and high level functions;\n");
		kprint("   debug_vga                test the vga driver [/svc/drivers/vga.c];\n");
		kprint("   debug_ahci               test the ahci driver;\n");
	} else PRGM_EQU("echo"){
		char * nts = str_to_nts(arguments);
		kprint(nts);
		kprint("\n");
		free(nts);
	} else PRGM_EQU("math"){
		bool error1 = false;
		bool error2 = false;
		bool error3 = false;
		bool unknown_op = false;
		string arg1 = argument(0, arguments, &error1);
		string arg2 = argument(1, arguments, &error2);
		string arg3 = argument(2, arguments, &error3);
		
		char * nts_arg2_n = str_to_nts(arg2);
		u32 arg2_n = parse_int(nts_arg2_n, &error2);
		free(nts_arg2_n);
		char * nts_arg3_n = str_to_nts(arg3);
		u32 arg3_n = parse_int(nts_arg3_n, &error3);
		free(nts_arg3_n);
		u32 result = 0;
		     if (str_equ_nts(arg1, "add")) result = arg2_n + arg3_n;
		else if (str_equ_nts(arg1, "sub")) result = arg2_n - arg3_n;
		else if (str_equ_nts(arg1, "mul")) result = arg2_n * arg3_n;
		else if (str_equ_nts(arg1, "div")) result = arg2_n / arg3_n;
		else unknown_op = true;
		strfree(arg1);
		strfree(arg2);
		strfree(arg3);
		if (unknown_op) return kprint("Unknown math operation !\n");
		if (error1 || error2 || error3) return kprint("Invalid arguments !\n");
		print_number(result);
		kprint("\n");
	} else PRGM_EQU("io"){
		bool error1 = false;
		bool error2 = false;
		bool error3 = false;
		bool unknown_op = false;
		u32 result = 0;
		bool should_print_result = false;
		string arg1 = argument(0, arguments, &error1);
		string arg2 = argument(1, arguments, &error2);
		
		char * nts_arg2_n = str_to_nts(arg2);
		u32 arg2_n = parse_int(nts_arg2_n, &error2);
		free(nts_arg2_n);
		
		if (str_equ_nts(arg1, "out")){
			string arg3 = argument(2, arguments, &error3);
		
			char * nts_arg3_n = str_to_nts(arg3);
			u32 arg3_n = parse_int(nts_arg3_n, &error3);
			free(nts_arg3_n);
			
			strfree(arg3);
			
			io_out_b(arg2_n, arg3_n);
		} else if (str_equ_nts(arg1, "in")){
			result = io_in_b(arg2_n);
			should_print_result = true;
		} else unknown_op = true;
		strfree(arg1);
		strfree(arg2);
		if (unknown_op) return kprint("Unknown math operation !\n");
		if (error1 || error2 || error3) return kprint("Invalid arguments !\n");
		if (should_print_result){
			print_number(result);
			kprint("\n");
		}
	} else PRGM_EQU("debug_chars"){
		char * l = "_ ";
		char * hex = "0123456789ABCDEF";
		kprint("     ");
		for (u32 i = 0; i < 16; i++){
			l[0] = hex[i];
			kprint(l);
		}
		for (s8 i = 0; i < 16; i++){
			kprint("\n ");
			l[0] = hex[i];
			kprint(l);
			kprint(": ");
			for (s8 j = 0; j < 16; j++){
				l[0] = i + j*16;
				kprint(l);
			}
		}
		kprint("\n");
	} else PRGM_EQU("debug_colors"){
		char * l = "_ ";
		char * hex = "0123456789ABCDEF";
		kprint("     ");
		for (u32 i = 0; i < 16; i++){
			l[0] = hex[i];
			kprint(l);
		}
		for (s8 i = 0; i < 16; i++){
			kprint("\n ");
			l[0] = hex[i];
			kprint(l);
			kprint(": ");
			for (s8 j = 0; j < 16; j++) kprint_color("X ", i + j*16);
		}
		kprint("\n");
	} else PRGM_EQU("debug_vga"){
		vga_test();
	} else PRGM_EQU("debug_ahci"){
		ahci_test();
	} else PRGM_EQU("pci_tree"){
		pci_info_t * pci_tree = pci_get_tree();
		pci_info_t * tmp_pci;
		while (pci_tree){
			for (tmp_pci = pci_tree; tmp_pci->parent; tmp_pci = tmp_pci->parent) kprint("  ");
			kprint("Vendor: ");
			print_hex(pci_tree->vendor_id);
			kprint("; Device: ");
			print_hex(pci_tree->device_id);
			kprint("; Class: ");
			print_hex(pci_tree->base_class);
			kprint(", ");
			print_hex(pci_tree->sub_class);
			kprint(", ");
			print_hex(pci_tree->interface);
			if (pci_tree->parent){
				u32 parent = pci_tree->parent->vendor_id << 16;
				parent |= pci_tree->parent->device_id;
				kprint("; (");
				print_hex(parent);
				kprint(")");
			}
			kprint(";\n");
			pci_tree = pci_tree->next;
		}
	} else PRGM_EQU("debug_keyboard"){
		if (kb_hook[0] == -1){
			kb_hook[0] = register_keyboard_handler(&kbdbg_cb, true);
			kb_hook[1] = register_keyboard_handler(&kbdbg_cb, false);
		} else {
			remove_keyboard_handler(kb_hook[0], true);
			remove_keyboard_handler(kb_hook[1], false);
		}
	} else PRGM_EQU("heap"){
		show_current_heap();
	} else PRGM_EQU("snake"){
		snake_init();
	} else PRGM_EQU("clear"){
		clear_screen();
	} else {
		kprint("Unknown command \"");
		char * nts_prgm = str_to_nts(program);
		kprint(nts_prgm);
		kprint("\"\n");
		free(nts_prgm);
	}
}

void console(){
	string cmd;
	str_space = str_from_nts(" ");
	NULLSTR = str_from_nts("");
	string program;
	string arguments;
	u32 heap_taken = allocated_memory_amount;
	while (true){
		if (heap_taken != allocated_memory_amount){
			kprint_color("Heap leak during last command !\n", 0b00000100);
			print_kv("before", heap_taken);
			print_kv("after", allocated_memory_amount);
		}
		heap_taken = allocated_memory_amount;
		char * c = readline("> ");
		cmd = str_from_nts(c);
		
		while (str_at(cmd, 0) == ' '){
			string tmp = substr(cmd, 1, -1);
			strfree(cmd);
			cmd = tmp;
		}
		
		u32 args_start = strstr(str_space, cmd);
		program = substr(cmd, 0, args_start);
		if (args_start != -1) arguments = substr(cmd, args_start+1, -1);
		else arguments = str_from_nts("");
		
		/*kprint("program: ");
		kprint(str_to_nts(program));
		kprint("\nargs: ");
		kprint(str_to_nts(arguments));
		kprint("\n");*/
		
		prgm_lookup(program, arguments);
		
		strfree(program);
		strfree(arguments);
		strfree(cmd);
		free(c);
	}
	strfree(str_space);
	strfree(NULLSTR);
}



