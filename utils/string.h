#ifndef STRING_H
#define STRING_H

#include "types.h"

typedef struct string_char string_char_t;
typedef struct nts_list_member nts_list_member_t;

typedef struct string_char {
	u8 _char;
	string_char_t * prev;
	string_char_t * next;
} string_char_t;

typedef struct {
	string_char_t * first_char;
	string_char_t * last_char;
	u32 length;
} string_struct;

typedef string_struct * string;

char * str_to_nts(string str);
string str_from_nts(char* nts);
void   str_push(string str, char c);
void   str_insert(string str, u32 index, char c);
char   str_pop(string str);
char   str_remove(string str, u32 index);
char   str_at(string str, u32 index);
void   strcat(string dest, string src);
u8     strcmp(string str1, string str2);
string substr(string str, u32 start, u32 length);
string strcpy(string str);
s32    strstr(string search, string subject);
void   strfree(string str);

#endif
