#include "math.h"
#include "../services/drivers/teletype.h"

char hexadecimal[] = "0123456789ABCDEF";

int strlen(char * nts){
	int i = 0;
	for (; nts[i]; i++);
	return i;
}

int parse_int_in_char(char c, int * error_out, int base){
	int error_b10 = c < '0' || c > '9', error_b16 = 1, error_b16_shift = 1;
	if (base == 16){
		error_b16 = (c < 'a' || c > 'f');
		error_b16_shift = (c < 'A' || c > 'F');
	}
	int error = error_b10 && error_b16 && error_b16_shift;
	if (error){
		if (error_out != 0) *error_out = error;
		return 0;
	}
	if (!error_b10) return c - '0';
	else if (!error_b16) return 10 + c - 'a';
	else /*if (!error_b16_shift)*/ return 10 + c - 'A';
}

unsigned int parse_int(char * nts, int * error_out){
	int error = 0;
	unsigned int result = 0;
	int len = strlen(nts) - 1;
	int base = 10;
	int b_limit = 0;
	if (nts[0] == '0' && nts[1] == 'x'){ base = 16; b_limit = 2; }
	for (int i = 0; nts[i + b_limit]; i++){
		result += parse_int_in_char(nts[len-i], &error, base) * pow(base, i);
		if (error) break;
	}
	if (error && error_out != 0) *error_out = error;
	return result;
}

int int_to_string(unsigned int number, unsigned int base, char * result){
	int log = 0;
	unsigned int copy = number;
	while (copy){
		log++;
		copy /= base;
	}
	if (log == 0){
		result[0] = '0';
		result[1] = 0;
		return 1;
	}
	result[log] = 0;
	while (number){
		result[--log] = hexadecimal[number%base];
		number /= base;
	}
	return 1;
}
