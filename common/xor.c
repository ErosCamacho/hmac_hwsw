

#include "xor.h"

void xor(unsigned char* out, unsigned char* in1, unsigned char* in2, int size)
{
    for (int i = 0; i < size; i++) {
        out[i] = in1[i] ^ in2[i];
    }
}