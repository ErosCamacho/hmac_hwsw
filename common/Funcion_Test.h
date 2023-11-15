void reset_in(unsigned char* INPUT);
void input_nist(unsigned char* INPUT, unsigned char* OUTPUT, unsigned long long int* LEN, int sel, int b, int DBG);
void input_file(unsigned char* INPUT, unsigned long long int* LEN, unsigned char* FILE_NAME, int hex, int DBG);
void char_to_hex(unsigned char in0, unsigned char in1, unsigned char* out);
int check_hex(unsigned char in);
void show_array(unsigned char* ar, unsigned int length, unsigned int mod);