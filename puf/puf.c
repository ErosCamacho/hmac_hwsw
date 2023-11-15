#include "puf4r4.h"
#include <pynq_api.h>

void puf(int K_bits, int PE, unsigned char* out) {

	MMIO_WINDOW puf;

	int N_CMPS = 4096;      //    Number of comparisons
	int E_CMPS = 0;       //    Number of discarded comparisons
	//int PE = 1;        //    PUF/Entropy Source (in IP Operation mode)
	int SD = 1;        // -s Same/different LUTs
	int BG = 1;        // -g Binary/Gray counters
	int NR = 0;        // -n Nearby/Remote RO pairs
	int LH = 0;        // -l Lower/Higher Bits
	//int K_bits = 1024;      // -k Number of bits of KEY
	int RC = 5;        // -p ECC Repetition code
	int E_tests = 10;      // -m Number of test for enrollment

	int puf_ref[MN_CMPS];
	int puf_selm[MN_CMPS];
	int puf_eref[MN_CMPS];

	//int* puf_ref;
	//int* puf_selm;
	//int* puf_eref;

	//puf_ref		= malloc(MN_CMPS * sizeof(int));
	//puf_selm	= malloc(MN_CMPS * sizeof(int));
	//puf_eref	= malloc(MN_CMPS * sizeof(int));

	int N_regs;
	int N_bits;

	unsigned int CONFIG;
	int  puf_outc[MN_CMPS];
	char puf_outb[MN_BITS];
	char puf_outecc[MN_BITS];

	int op;
	unsigned int puf_output[MN_REGS];
	int puf_selr[(int)ceil((MN_CMPS / 32.))];

	// Calculate No. of challenges that can be discarded (E_CMPS)

	N_bits = ceil(RC * K_bits / 4.) * 4;         // Number of bits 
	if (N_bits / 4 > N_CMPS) {
		printf("\n  %d comparisons are not enough to obfuscate a %d-bit key with a repetition code of length %d.  Bye ...\n\n", N_CMPS, K_bits, RC);
	}
	else {
		E_CMPS = N_CMPS - N_bits / 4;           // Number of discarded comparisons
	}

	N_regs = ceil((N_CMPS - E_CMPS) / 8.);

	CONFIG = (PE << 6) + (SD << 5) + (BG << 4) + (NR << 3) + (LH << 2);


	// Create PUF interface
	PUF_createMMIOWindow(&puf, 0x43C40000, PUF_LENGTH, 0);

	// Reset Challenges Mask
	op = 0;
	writeMMIO(&puf, &op, CHLADDC, sizeof(op));
	for (int c = 0; c < ceil(N_CMPS / (Dbw / 4.)) * 16; c++) {
		puf_selm[c] = 0;
		puf_ref[c] = 0;
	}
	for (int r = 0; r < ceil(MN_CMPS / 32.); r++) {
		puf_selr[r] = 0;
	}

	// Get PUF reference output for N_CMPS-E_CMPS
	int cr = 0;
	for (int c = 0; c < N_CMPS; c++) {
		if (puf_selm[c] == 0) {
			puf_eref[cr] = puf_ref[c];
			cr++;
		}
	}

	chl_btor(puf_selm, N_CMPS, puf_selr);

	// Perform the enrollment process
	PUF_enrollment(&puf, 0, 0, CONFIG, E_tests, N_CMPS, E_CMPS, puf_ref, puf_selm);

	// Write Challenges Selection Mask
	PUF_writeChallegesMask(&puf, puf_selr, ceil(MN_CMPS / 32.), 0);
	op = RCHL;
	writeMMIO(&puf, &op, CHLADDC, sizeof(op));

	// Generate PUF response
	PUF_applyChallenges(&puf, CONFIG, N_CMPS - E_CMPS, 0);

	//Read PUF results
	PUF_readOutput(&puf, N_CMPS - E_CMPS, puf_output, 0);

	// Get PUF comparisons      
	puf_rtoc(puf_output, N_regs, puf_outc);

	// Get PUF bits      
	puf_ctob(puf_outc, N_CMPS - E_CMPS, puf_outb);

	// ECC
	puf_ecc(puf_outb, N_bits, RC, puf_outecc);

	for (int i = 0; i < (K_bits / 8); i++)

	out[i] =	puf_outecc[8 * i + 0] << 7 |
				puf_outecc[8 * i + 1] << 6 |
				puf_outecc[8 * i + 2] << 5 |
				puf_outecc[8 * i + 3] << 4 |
				puf_outecc[8 * i + 4] << 3 |
				puf_outecc[8 * i + 5] << 2 |
				puf_outecc[8 * i + 6] << 1 |
				puf_outecc[8 * i + 7];



}