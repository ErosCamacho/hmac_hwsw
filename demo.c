#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include "pynq_api.h"
#include "common/Funciones_HW.h"
#include "sha2/sha2_hw.h"
#include "sha2/sha2_sw.h"
#include "sha3/sha3_hw.h"
#include "sha3/sha3_sw.h"
#include "config.h"
#include "HMAC.h"
#include "puf/puf.h"

int main(int argc, char** argv) {
	
	unsigned char* key;
	unsigned char* msg;
	unsigned char* hmac_out;
	unsigned char* out_puf;

	unsigned long long tic = 0, toc;
	unsigned long long time_hw = 0, time_sw = 0;

	load_bitstream(0);

	hmac_out	= malloc(32 * sizeof(char));
	out_puf		= malloc(1024 * sizeof(char));
	key			= malloc(1024 * sizeof(char));
	msg			= malloc(1024 * sizeof(char));

	int k_bits	= 128;
	int sel_puf	= 0;
	int mode_puf = 1;
	int mode = 1;

	for (int arg = 1; arg < argc; arg++) {
		if (argv[arg][0] == '-') {
			if (argv[arg][1] == 'h') {
				printf("\n Usage: HMAC [ [-h] | [-s] [-m] [-k] [-p/t] [-n]]\n");
				printf("\n  -h:		Show the help.");
				printf("\n  -s:		1. SHA-256  --- 2. SHA-512 --- 3. SHA3-256 --- 4. SHA3-512 ");
				printf("\n  -m:		Input message (HEX format).");
				printf("\n  -k:		Input key (HEX format).");
				printf("\n  -p/t:	Generate a key using PUF(p) / TRNG(t) response");
				printf("\n  -n:		Number of bits of the generated key (max: 512)");
				printf("\n \n");
				return 1;
			}
			else if ((argv[arg][1] == 's')) {
				mode = atoi(argv[arg + 1]);
			}
			else if (argv[arg][1] == 'm') {
				if (arg < argc - 1) memcpy(msg, argv[arg + 1], 128 * sizeof(char));
			}
			else if (argv[arg][1] == 'k') {
				if (arg < argc - 1) memcpy(key, argv[arg + 1], 128 * sizeof(char));
			}
			else if (argv[arg][1] == 'p') {
				sel_puf = 1; mode_puf = 1;
			}
			else if (argv[arg][1] == 't') {
				sel_puf = 1; mode_puf = 0;
			}
			else if (argv[arg][1] == 'n') {
				k_bits = atoi(argv[arg + 1]);
			}
			else {
				printf("\n unknown option: %s\n\n", argv[arg]);
				return 0;
			}
		}
	}

	PYNQ_MMIO_WINDOW ms2xl;
	if (mode == 1) PYNQ_createMMIOWindow(&ms2xl, MS2XL_BASEADDR_SHA2_256, MS2XL_LENGTH);
	if (mode == 2) PYNQ_createMMIOWindow(&ms2xl, MS2XL_BASEADDR_SHA2_512, MS2XL_LENGTH);
	if (mode == 3) PYNQ_createMMIOWindow(&ms2xl, MS2XL_BASEADDR_SHA3_256, MS2XL_LENGTH);
	if (mode == 4) PYNQ_createMMIOWindow(&ms2xl, MS2XL_BASEADDR_SHA3_512, MS2XL_LENGTH);

	if (mode == 1) printf("\n --- HMAC (SHA-256) --- \n");
	if (mode == 2) printf("\n --- HMAC (SHA-512) --- \n");
	if (mode == 3) printf("\n --- HMAC (SHA3-256) --- \n");
	if (mode == 4) printf("\n --- HMAC (SHA3-512) --- \n");
	
	if (sel_puf) {
		puf(k_bits, mode_puf, out_puf);

		printf("\n");
		for (int i = 0; i < (k_bits / 8); i++) sprintf(key, "%s%d", key, out_puf[i]);

		printf("\n key (l=%d): \n", k_bits);
		show_array(out_puf, (k_bits / 8), 32);

	}

	time_hw = PYNQ_Wtime();
	hmac_hw(ms2xl, key, msg, hmac_out, mode, 0);
	time_hw = PYNQ_Wtime() - time_hw; 
	
	printf("\n hmac_hw: \n");
	if (mode == 1) show_array(hmac_out, (256 / 8), 32);
	if (mode == 2) show_array(hmac_out, (512 / 8), 32);
	if (mode == 3) show_array(hmac_out, (256 / 8), 32);
	if (mode == 4) show_array(hmac_out, (512 / 8), 32);

	time_sw = PYNQ_Wtime();
	hmac_sw(key, msg, hmac_out, mode, 0);
	time_sw = PYNQ_Wtime() - time_sw;

	printf("\n hmac_sw: \n");
	if (mode == 1) show_array(hmac_out, (256 / 8), 32);
	if (mode == 2) show_array(hmac_out, (512 / 8), 32);
	if (mode == 3) show_array(hmac_out, (256 / 8), 32);
	if (mode == 4) show_array(hmac_out, (512 / 8), 32);

	printf("\n HW Time: \t (%6llu us.)", time_hw);
	printf("\n SW Time: \t (%6llu us.)", time_sw);
	printf("\n    Acc.: \t %.2f ", ((double)time_sw / (double)time_hw));

	printf("\n\n");


	
	
}