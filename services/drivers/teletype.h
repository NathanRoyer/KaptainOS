#define WHITE_ON_BLACK 0x0f
#define RED_ON_WHITE 0xf4

void teletype_disable_cursor();
int get_cursor_offset();
void kprint_at(char *message, int col, int row, char attr);
int print_char(char c, int col, int row, char attr);
unsigned char get_char_at(int col, int row);
void set_print_color(char attr);
void kprint(char * message);
void kprint_color(char * message, char attr);
void print_number(unsigned int n);
void print_hex(unsigned int n);
void print_bin(unsigned int n);
void print_kv(char * key, unsigned int n);
void print_kv_hex(char * key, unsigned int n);
void print_kv_bin(char * key, unsigned int n);
void move_cursor(unsigned char relative, int horizontal, int vertical);
void clear_screen();
