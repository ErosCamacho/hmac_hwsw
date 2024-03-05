#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include "pynq_api.h"
#include "RoT/common/Funcion_Test.h"
#include "RoT/common/xor.h"
#include "RoT/sha2/sha2_hw.h"
#include "RoT/sha2/sha2_sw.h"
#include "RoT/sha3/sha3_hw.h"
#include "RoT/sha3/sha3_sw.h"
#include "RoT/config.h"


void hmac_hw(PYNQ_MMIO_WINDOW ms2xl, unsigned char* key, unsigned char* msg, unsigned char* hmac_final, int mode, int sel_puf, int verb) {

	int DBG = 0;

	int sha_256		= 0;
	int sha_512		= 0;
	int sha3_256	= 0;
	int sha3_512	= 0;

	if (mode == 1) sha_256	= 1;
	if (mode == 2) sha_512	= 1;
	if (mode == 3) sha3_256 = 1;
	if (mode == 4) sha3_512 = 1;

	int size_block = 0;
	if (mode == 1) size_block = SIZE_BLOCK_SHA2_256;
	if (mode == 2) size_block = SIZE_BLOCK_SHA2_512;
	if (mode == 3) size_block = SIZE_BLOCK_SHA3_256;
	if (mode == 4) size_block = SIZE_BLOCK_SHA3_512;

	int size_out = 0;
	if (mode == 1) size_out = SIZE_SHA2_256 / 8;
	if (mode == 2) size_out = SIZE_SHA2_512 / 8;
	if (mode == 3) size_out = SIZE_SHA3_256 / 8;
	if (mode == 4) size_out = SIZE_SHA3_512 / 8;

	int size_sha = 0;
	if (mode == 1) size_sha = SIZE_SHA2_256;
	if (mode == 2) size_sha = SIZE_SHA2_512;
	if (mode == 3) size_sha = SIZE_SHA3_256;
	if (mode == 4) size_sha = SIZE_SHA3_512;

	unsigned char* opad;
	unsigned char* ipad;
	opad = malloc((size_block / 8) * sizeof(char));
	ipad = malloc((size_block / 8) * sizeof(char));
	memset(opad, OPAD, size_block / 8);
	memset(ipad, IPAD, size_block / 8);


	unsigned char *key_hex;
	unsigned char *msg_hex;
	unsigned char *key_prime;
	unsigned char *key_opad;
	unsigned char *key_ipad;
	unsigned char out_hw[64];
	unsigned char *concat1;
	unsigned char *concat2;
	unsigned char key_ipad_msg_hash[64];

	key_hex		= malloc(SIZE_INPUT * sizeof(char));
	msg_hex		= malloc(SIZE_INPUT * sizeof(char));
	key_opad	= malloc((size_block / 8) * sizeof(char));
	key_ipad	= malloc((size_block / 8) * sizeof(char));
	key_prime	= malloc((size_block / 8) * sizeof(char));
	concat1		= malloc(SIZE_INPUT * sizeof(char));
	concat2		= malloc(SIZE_INPUT * sizeof(char));

	memset(key_hex, 0, SIZE_INPUT);
	memset(msg_hex, 0, SIZE_INPUT);

	/* Length */
	unsigned long long length_key;
	unsigned long long length_msg;
	unsigned long long length_key_bit;
	unsigned long long length_msg_bit;

	if (sel_puf) length_key = (unsigned long long)strlen(key);
	else length_key = (unsigned long long)strlen(key) / 2;

	length_msg = (unsigned long long)strlen(msg) / 2;
	length_key_bit = 8 * length_key;
	length_msg_bit = 8 * length_msg;

	/* Key & Data Hex */
	int ind1, ind2;
	char character;
	if (sel_puf) {
		memcpy(key_hex, key, sizeof(unsigned char) * length_key);
	}
	else {
		for (int i = 0; i < length_key; i++) {
			ind1 = 2 * i;
			ind2 = ind1 + 1;
			char_to_hex(key[ind1], key[ind2], &character);
			key_hex[i] = character;
		}
	}


	for (int i = 0; i < length_msg; i++) {
		ind1 = 2 * i;
		ind2 = ind1 + 1;
		char_to_hex(msg[ind1], msg[ind2], &character);
		msg_hex[i] = character;
	}

	/* HMAC KEY */
	if (length_key_bit > size_block) {
		if (mode == 1) sha2_hw_256(key_hex, out_hw, length_key_bit, ms2xl, DBG);
		if (mode == 2) sha2_hw_512(key_hex, out_hw, length_key_bit, ms2xl, DBG);
		if (mode == 3) sha3_hw_256(key_hex, out_hw, length_key_bit, ms2xl, DBG);
		if (mode == 4) sha3_hw_512(key_hex, out_hw, length_key_bit, ms2xl, DBG);
		memcpy(key_prime, out_hw, size_out);
	}
	else {
		memcpy(key_prime, key_hex, size_block / 8);
	}

	xor(key_ipad, key_prime, ipad, size_block / 8);
	xor(key_opad, key_prime, opad, size_block / 8);

	/* Verbose 1 */
	if (verb == 3) { printf("\n \t length_key: \t %lld", length_key); }
	if (verb == 3) { printf("\n \t IPAD: \t");	for (int i = 0; i < (size_block / 8); i++)	printf("%02x", ipad[i]); }
	if (verb == 3) { printf("\n \t OPAD: \t");	for (int i = 0; i < (size_block / 8); i++)	printf("%02x", opad[i]); }
	if (verb == 2) { printf("\n \t KEY_HEX: \t"); for (int i = 0; i < length_key; i++)		printf("%02x", key_hex[i]); }
	if (verb == 2) { printf("\n \t MSG_HEX: \t"); for (int i = 0; i < length_msg; i++)		printf("%02x", msg_hex[i]); }
	if (verb == 2) { printf("\n \t KEY_PRIME: \t"); for (int i = 0; i < length_key; i++)		printf("%02x", key_prime[i]); }
	if (verb == 1) { printf("\n \t KEY_IPAD: \t"); for (int i = 0; i < (size_block / 8); i++) printf("%02x", key_ipad[i]); }
	if (verb == 1) { printf("\n \t KEY_OPAD: \t"); for (int i = 0; i < (size_block / 8); i++) printf("%02x", key_opad[i]); }


	/* Concatenation 1 */
	int i = 0;
	int j = 0;
	for (i = 0; i < (size_block / 8); i++) {
		concat1[j] = key_ipad[i];
		j++;
	}
	for (i = 0; i < length_msg; i++) {
		concat1[j] = msg_hex[i];
		j++;
	}
	/* HMAC 1 */
	if (mode == 1) sha2_hw_256(concat1, out_hw, (size_block + length_msg_bit), ms2xl, DBG);
	if (mode == 2) sha2_hw_512(concat1, out_hw, (size_block + length_msg_bit), ms2xl, DBG);
	if (mode == 3) sha3_hw_256(concat1, out_hw, (size_block + length_msg_bit), ms2xl, DBG);
	if (mode == 4) sha3_hw_512(concat1, out_hw, (size_block + length_msg_bit), ms2xl, DBG);
	memcpy(key_ipad_msg_hash, out_hw, size_out);

	/* Concatenation 2 */
	j = 0;
	for (i = 0; i < (size_block / 8); i++) {
		concat2[j] = key_opad[i];
		j++;
	}
	for (i = 0; i < size_out; i++) {
		concat2[j] = key_ipad_msg_hash[i];
		j++;
	}

	/* HMAC 2 */
	if (mode == 1) sha2_hw_256(concat2, out_hw, (size_block + size_sha), ms2xl, DBG);
	if (mode == 2) sha2_hw_512(concat2, out_hw, (size_block + size_sha), ms2xl, DBG);
	if (mode == 3) sha3_hw_256(concat2, out_hw, (size_block + size_sha), ms2xl, DBG);
	if (mode == 4) sha3_hw_512(concat2, out_hw, (size_block + size_sha), ms2xl, DBG);
	memcpy(hmac_final, out_hw, size_out);

	/* Verbose 1 */
	if (verb == 2) { printf("\n \t CONCAT1: \t");		for (int i = 0; i < ((size_block / 8) + length_msg); i++)	printf("%02x", concat1[i]); }
	if (verb == 1) { printf("\n \t KEY_IPAD_MSG: \t");	for (int i = 0; i < size_out; i++)							printf("%02x", key_ipad_msg_hash[i]); }
	if (verb == 2) { printf("\n \t CONCAT2: \t");		for (int i = 0; i < ((size_block / 8) + size_out); i++)		printf("%02x", concat2[i]); }
	if (verb == 4) { printf("\n \t HMAC: \t");			for (int i = 0; i < size_out; i++)							printf("%02x", hmac_final[i]); }
	if (verb == 4) { printf("\n \n"); }

}


void hmac_sw(unsigned char* key, unsigned char* msg, unsigned char* hmac_final, int mode, int sel_puf, int verb) {

	int DBG = 0;

	int sha_256 = 0;
	int sha_512 = 0;
	int sha3_256 = 0;
	int sha3_512 = 0;

	if (mode == 1) sha_256 = 1;
	if (mode == 2) sha_512 = 1;
	if (mode == 3) sha3_256 = 1;
	if (mode == 4) sha3_512 = 1;

	int size_block = 0;
	if (mode == 1) size_block = SIZE_BLOCK_SHA2_256;
	if (mode == 2) size_block = SIZE_BLOCK_SHA2_512;
	if (mode == 3) size_block = SIZE_BLOCK_SHA3_256;
	if (mode == 4) size_block = SIZE_BLOCK_SHA3_512;

	int size_out = 0;
	if (mode == 1) size_out = SIZE_SHA2_256 / 8;
	if (mode == 2) size_out = SIZE_SHA2_512 / 8;
	if (mode == 3) size_out = SIZE_SHA3_256 / 8;
	if (mode == 4) size_out = SIZE_SHA3_512 / 8;

	int size_sha = 0;
	if (mode == 1) size_sha = SIZE_SHA2_256;
	if (mode == 2) size_sha = SIZE_SHA2_512;
	if (mode == 3) size_sha = SIZE_SHA3_256;
	if (mode == 4) size_sha = SIZE_SHA3_512;

	unsigned char* opad;
	unsigned char* ipad;
	opad = malloc((size_block / 8) * sizeof(char));
	ipad = malloc((size_block / 8) * sizeof(char));
	memset(opad, OPAD, size_block / 8);
	memset(ipad, IPAD, size_block / 8);


	unsigned char* key_hex;
	unsigned char* msg_hex;
	unsigned char* key_prime;
	unsigned char* key_opad;
	unsigned char* key_ipad;
	unsigned char out_sw[64];
	unsigned char* concat1;
	unsigned char* concat2;
	unsigned char key_ipad_msg_hash[64];

	key_hex = malloc(SIZE_INPUT * sizeof(char));
	msg_hex = malloc(SIZE_INPUT * sizeof(char));
	key_opad = malloc((size_block / 8) * sizeof(char));
	key_ipad = malloc((size_block / 8) * sizeof(char));
	key_prime = malloc((size_block / 8) * sizeof(char));
	concat1 = malloc(SIZE_INPUT * sizeof(char));
	concat2 = malloc(SIZE_INPUT * sizeof(char));

	memset(key_hex, 0, SIZE_INPUT);
	memset(msg_hex, 0, SIZE_INPUT);
	memset(concat1, 0, SIZE_INPUT);

	/* Length */
	unsigned long long length_key;
	unsigned long long length_msg;
	unsigned long long length_key_bit;
	unsigned long long length_msg_bit;

	if (sel_puf) length_key = (unsigned long long)strlen(key);
	else length_key = (unsigned long long)strlen(key) / 2;

	length_msg = (unsigned long long)strlen(msg) / 2;
	length_key_bit = 8 * length_key;
	length_msg_bit = 8 * length_msg;

	/* Key & Data Hex */
	int ind1, ind2;
	char character;
	if (sel_puf) {
		memcpy(key_hex, key, sizeof(unsigned char) * length_key);
	}
	else {
		for (int i = 0; i < length_key; i++) {
			ind1 = 2 * i;
			ind2 = ind1 + 1;
			char_to_hex(key[ind1], key[ind2], &character);
			key_hex[i] = character;
		}
	}

	for (int i = 0; i < length_msg; i++) {
		ind1 = 2 * i;
		ind2 = ind1 + 1;
		char_to_hex(msg[ind1], msg[ind2], &character);
		msg_hex[i] = character;
	}

	/* HMAC KEY */
	if (length_key_bit > size_block) {
		if (mode == 1) sha2_sw_256(key_hex, length_key_bit, out_sw);
		if (mode == 2) sha2_sw_512(key_hex, length_key_bit, out_sw);
		if (mode == 3) sha3_sw_256(key_hex, length_key_bit, out_sw);
		if (mode == 4) sha3_sw_512(key_hex, length_key_bit, out_sw);
		memcpy(key_prime, out_sw, size_out);
	}
	else {
		memcpy(key_prime, key_hex, size_block / 8);
	}

	xor (key_ipad, key_prime, ipad, size_block / 8);
	xor (key_opad, key_prime, opad, size_block / 8);

	/* Verbose 1 */
	if (verb == 3) { printf("\n \t length_key: \t %lld", length_key); }
	if (verb == 3) { printf("\n \t IPAD: \t");	for (int i = 0; i < (size_block / 8); i++)	printf("%02x", ipad[i]); }
	if (verb == 3) { printf("\n \t OPAD: \t");	for (int i = 0; i < (size_block / 8); i++)	printf("%02x", opad[i]); }
	if (verb == 2) { printf("\n \t KEY_HEX: \t"); for (int i = 0; i < length_key; i++)		printf("%02x", key_hex[i]); }
	if (verb == 2) { printf("\n \t MSG_HEX: \t"); for (int i = 0; i < length_msg; i++)		printf("%02x", msg_hex[i]); }
	if (verb == 2) { printf("\n \t KEY_PRIME: \t"); for (int i = 0; i < length_key; i++)		printf("%02x", key_prime[i]); }
	if (verb == 1) { printf("\n \t KEY_IPAD: \t"); for (int i = 0; i < (size_block / 8); i++) printf("%02x", key_ipad[i]); }
	if (verb == 1) { printf("\n \t KEY_OPAD: \t"); for (int i = 0; i < (size_block / 8); i++) printf("%02x", key_opad[i]); }

	/* Concatenation 1 */
	int i = 0;
	int j = 0;
	for (i = 0; i < (size_block / 8); i++) {
		concat1[j] = key_ipad[i];
		j++;
	}
	for (i = 0; i < length_msg; i++) {
		concat1[j] = msg_hex[i];
		j++;
	}

	/* HMAC 1 */
	if (mode == 1) sha2_sw_256(concat1, ((size_block / 8) + length_msg), out_sw);
	if (mode == 2) sha2_sw_512(concat1, ((size_block / 8) + length_msg), out_sw);
	if (mode == 3) sha3_sw_256(concat1, ((size_block / 8) + length_msg), out_sw);
	if (mode == 4) sha3_sw_512(concat1, ((size_block / 8) + length_msg), out_sw);
	memcpy(key_ipad_msg_hash, out_sw, size_out);


	/* Concatenation 2 */
	j = 0;
	for (i = 0; i < (size_block / 8); i++) {
		concat2[j] = key_opad[i];
		j++;
	}
	for (i = 0; i < size_out; i++) {
		concat2[j] = key_ipad_msg_hash[i];
		j++;
	}

	/* HMAC 2 */
	if (mode == 1) sha2_sw_256(concat2, ((size_block / 8) + size_out), out_sw);
	if (mode == 2) sha2_sw_512(concat2, ((size_block / 8) + size_out), out_sw);
	if (mode == 3) sha3_sw_256(concat2, ((size_block / 8) + size_out), out_sw);
	if (mode == 4) sha3_sw_512(concat2, ((size_block / 8) + size_out), out_sw);
	memcpy(hmac_final, out_sw, size_out);

	/* Verbose 1 */
	if (verb == 2) { printf("\n \t CONCAT1: \t");		for (int i = 0; i < ((size_block / 8) + length_msg); i++)	printf("%02x", concat1[i]); }
	if (verb == 1) { printf("\n \t KEY_IPAD_MSG: \t");	for (int i = 0; i < size_out; i++)							printf("%02x", key_ipad_msg_hash[i]); }
	if (verb == 2) { printf("\n \t CONCAT2: \t");		for (int i = 0; i < ((size_block / 8) + size_out); i++)		printf("%02x", concat2[i]); }
	if (verb == 4) { printf("\n \t HMAC: \t");			for (int i = 0; i < size_out; i++)							printf("%02x", hmac_final[i]); }
	if (verb == 4) { printf("\n \n"); }

}
