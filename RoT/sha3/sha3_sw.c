#include "sha3.h"

void sha3_sw_256(const void* in, size_t inlen, unsigned char* out)
{
    sha3_ctx_t sha3;

    sha3_init(&sha3, 32);
    sha3_update(&sha3, in, inlen);
    sha3_final(out, &sha3);
}

void sha3_sw_512(const void* in, size_t inlen, unsigned char* out)
{
    sha3_ctx_t sha3;

    sha3_init(&sha3, 64);
    sha3_update(&sha3, in, inlen);
    sha3_final(out, &sha3);
}