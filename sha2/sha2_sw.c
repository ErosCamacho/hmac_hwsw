#include "sha2_sw/sha2_256.h"
#include "sha2_sw/sha2_512.h"


void sha2_sw_256(unsigned char* data, size_t length, unsigned char* digest) {

    Sha256Context context;

    sha256Init(&context);
    //Digest the message
    sha256Update(&context, data, length);
    //Finalize the SHA-256 message digest
    sha256Final(&context, digest);
}

void sha2_sw_512(unsigned char* data, size_t length, unsigned char* digest) {

    Sha512Context context;

    sha512Init(&context);
    //Digest the message
    sha512Update(&context, data, length);
    //Finalize the SHA-256 message digest
    sha512Final(&context, digest);
}