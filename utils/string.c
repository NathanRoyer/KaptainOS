#include "string.h"
#include "../kernel/malloc.h"

// nts = null-terminated string

// to and from C strings :
char * str_to_nts(string str){
	char * nts = malloc(sizeof(char) * (str->length+1), false);
	for (int i = 0; i < str->length; i++){
		nts[i] = str_at(str, i);
	}
	nts[str->length] = 0;
	return nts;
}

string str_from_nts(char * nts){
	string str = malloc(sizeof(string_struct), false);
	str->first_char = 0;
	str->last_char = 0;
	str->length = 0;
	for (int i = 0; nts[i]; i++) str_push(str, nts[i]);
	return str;
}

void str_push(string str, char c){
	str_insert(str, str->length, c);
}

void str_insert(string str, u32 index, char c){
	if (str->length < index) return 0;
	string_char_t * to_be_pushed = malloc(sizeof(string_char_t), false);
	to_be_pushed->_char = c;
	
	if (index == 0){
		to_be_pushed->prev = 0;
		if (str->length == 0){
			to_be_pushed->next = 0;
			str->last_char = to_be_pushed;
		} else to_be_pushed->next = str->first_char;
		str->first_char = to_be_pushed;
		str->length++;
		return;
	}
	
	string_char_t * prev = str->first_char;
	while (--index) prev = prev->next;
	string_char_t * next = prev->next;
	
	to_be_pushed->prev = prev;
	prev->next = to_be_pushed;
	to_be_pushed->next = next;
	if (next) next->prev = to_be_pushed;
	else str->last_char = to_be_pushed;
	str->length++;
}

char str_pop(string str){
	return str_remove(str, str->length-1);
}

char str_remove(string str, u32 index){
	if (str->length <= index) return 0;
	string_char_t * to_be_popped = str->first_char;
	while (index--) to_be_popped = to_be_popped->next;
	char c = to_be_popped->_char;
	string_char_t * prev = to_be_popped->prev;
	string_char_t * next = to_be_popped->next;
	if (prev) prev->next = next;
	if (next) next->prev = prev;
	if (to_be_popped == str->first_char) str->first_char = next;
	if (to_be_popped == str->last_char) str->last_char = prev;
	if (!free(to_be_popped)) kprint("Damn, char not found !\n");
	str->length--;
	return c;
}

char str_at(string str, u32 index){
	if (str->length == 0) return 0; // undefined behaviour ?
	string_char_t * f = str->first_char;
	for (;index && f->next;index--) f = f->next;
	return f->_char;
}

void strcat(string dest, string src){
	for (int i = 0; i < src->length; i++) str_push(dest, str_at(src, i));
}

u8 strcmp(string str1, string str2){
	kprint("NOT IMPLEMENTED : strcmp in utils/string.c\n");
	return 0;
}

string substr(string str, u32 start, u32 length){
	string new_str = str_from_nts("");
	for (int i = 0; i < length && (i+start < str->length); i++) str_push(new_str, str_at(str, start + i));
	return new_str;
}

string strcpy(string str){
	string new_str = str_from_nts("");
	strcat(new_str, str);
	return new_str;
}

s32 strstr(string search, string subject){
	bool found;
	for (int i = 0; i < subject->length; i++){
		found = true;
		for (int j = 0; j < search->length; j++){
			found = str_at(search, j) ==  str_at(subject, j+i);
			if (!found) break;
		}
		if (found) return i;
	}
	return -1;
}

void strfree(string str){
	while (str->length) str_pop(str);
	free(str);
}

u32 strcount(string search, string subject){
	u32 count = 0;
	s32 index_in_tmpstr;
	string tmpstr;
	for (u32 i = 0; i < subject->length; i+= index_in_tmpstr + search->length){
		tmpstr = substr(subject, i, subject->length);
		index_in_tmpstr = strstr(search, tmpstr);
		strfree(tmpstr);
		if (index_in_tmpstr == -1) break;
		else count++;
	}
	return count;
}

bool str_equ(string str1, string str2){
	return str1->length == str2->length && strstr(str1, str2) == 0;
}

bool str_equ_nts(string str1, char * nts){
	string str2 = str_from_nts(nts);
	bool ret = str_equ(str1, str2);
	strfree(str2);
	return ret;
}













