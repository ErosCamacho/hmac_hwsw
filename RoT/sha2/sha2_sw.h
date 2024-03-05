#include "params.h"
#include "sha2_sw/sha2_256.h"
#include "sha2_sw/sha2_512.h"

void sha2_sw_256(unsigned char* data, size_t length, unsigned char* digest);
void sha2_sw_512(unsigned char* data, size_t length, unsigned char* digest);