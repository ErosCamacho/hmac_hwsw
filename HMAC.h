#include "pynq_api.h"

void hmac_hw(PYNQ_MMIO_WINDOW ms2xl, unsigned char* key, unsigned char* msg, unsigned char* hmac_final, int mode, int verb);
void hmac_sw(unsigned char* key, unsigned char* msg, unsigned char* hmac_final, int mode, int verb);