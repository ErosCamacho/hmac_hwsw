/******************************************************************************/
/*
 *  IMSE.CNM_SPIRS-HwSec_puf4r4_2.2: trng_validation.c for PUF4R4_2.2  32/64-bits
 *
 *  Created on: 10/02/2023
 *      Author: rojas@imse-cnm.csic.es
 *
 *  Updated on: 07/09/2023
 *      Author: santiago @imse-cnm.csic.es
 *
 *  Function:
 *  - Detects whether the entropy source is presenting a failure by evaluating the
 *    2 health tests (Repetition Count and Adaptative Propotion) presented in the 
 *    NIST recommendation 800-90b. The parameters required to evaluate these tests 
 *    were calculated during the process of characterizing the PUF as an entropy source. 
 *
 *  Outputs:
 *  - Health check tests parameters 
 *	- Health check tests results
*/
/******************************************************************************/
 
/******************************** Include Files *******************************/

#include "puf4r4.h"

#include <stdio.h>
#include <string.h>
#include <math.h>

int main(int argc, char* argv[]) 
{
    char  X       = 0;
	char  C_rc    = 0;
	char  A       = 0;
	
	int   B       = 0;
    int   Bmax    = 0;
    int   C_ap    = 612;
    int   N_CMPS  = 2000;
    int   N_tests = 125;
    int   DBG     = 0;    // -d Debug level
    int   W       = 1024;
    int   NR      = 0;    // -n Nearby/Remote RO pairs
    int   BG      = 1;    // -g Binary/Gray counters
	int   HCS     = 0;    // Health Check Success
	
	float alpha   = 0.00000502515530065;
    float H       = 0.834914;

    int  ns        = 0;
    char inst[]    = "00";
    char ctcode[]  = "_ES_00";
    char OutF[]    = "out/puf/SPIRS_RoT_";
    char odtrail[] = "_90.bin";
    char binfile[256]; 

    FILE* bin_file;

    for (int arg = 1; arg < argc; arg++) {
        if (argv[arg][0] == '-') {
            if (argv[arg][1] == 'h') {
                printf("\n Usage: trng_validation [ -h | [-g] [-n] [-i <puf_instance>] ");
				printf("\n                        [-f <input_file>] [-d <dbg_level>]\n\n");
				return SUCCESS;               
            } else if (argv[arg][1] == 'f') {
                if (arg < argc - 1) strcpy(binfile, argv[arg + 1]);
            } else if (argv[arg][1] == 's') {
                if (arg < argc - 1) H = atof(argv[arg + 1]);
            } else if (argv[arg][1] == 'x') {
                if (arg < argc - 1) C_ap = atoi(argv[arg + 1]);
            } else if (argv[arg][1] == 'a') {
                if (arg < argc - 1) alpha = atof(argv[arg + 1]);
            } else if (argv[arg][1] == 'd') {
                if (arg < argc - 1) DBG = atoi(argv[arg + 1]);
            } else if (argv[arg][1] == 'i') {
                if (arg < argc - 1) ns = atoi(argv[arg + 1]);
                if (ns < 0) ns = 0;
                if (ns > 9) {
                    ns = 9;
                }
            } else if (argv[arg][1] == 'g') {
                BG = 0;
            } else if (argv[arg][1] == 'n') {
                NR = 1;
            } else {
                printf("\n unknown option: %s\n\n", argv[arg]);
                return ERROR;
            }
        }
    }

    sprintf(inst, "%02d", ns);
    sprintf(ctcode, "_ES_%01d%01d", BG, NR);
    strcpy(binfile, OutF);
    strcat(binfile, inst);
    strcat(binfile, ctcode);
    strcat(binfile, odtrail);

    int lb = 1000000;
    unsigned char buffer[lb]; //

    printf("\n\t2) HEALTH CHECK TESTS \n");

    if (DBG > 0) {
        printf("\n\t-- PARAMETERS -- \n\n");
        printf("\t 1)Bin file name : %s \n", binfile);
        printf("\t 2)No. Tests     : %d \n", N_tests);
        printf("\t 3)No. Cmps      : %d \n", N_CMPS);
        printf("\t 4)No. bits      : %d \n", lb);
        printf("\t 5)H_estimated   : %f \n", H);
        printf("\t 6)Alpha         : %.17f \n", alpha);
        printf("\t 7)AP_cutoff     : %d \n\n", C_ap);
    }

    // Reading *.bin file
    if ((bin_file = fopen(binfile, "rb"))) {
        fread(buffer, sizeof(buffer), 1, bin_file);
        fclose(bin_file);
    } else {
        printf("Bin file doesn't exist. Bye ...\n\n");
        return ERROR;
    }

    // HEALTH CHECKS
    //---------------------------------------------------------------------------------------------------
    // REPETITION COUNT Test
    /*
    * Parameters:
    *
    *   H     : estimated entropy
    *   alpha : false-positive probability
    *   C_rc  : cutoff value
    *
    *   H     = H_original
    *   alpha = calulated in restart tests ;  (recommended in SP800-90b - Section 4.3.3)
    *   C_rc  = 1+ [-log2(alpha) / H]  = 1 + [20/H]
    */

    if (DBG > 0) printf("\t-- TESTS -- \n");
    printf("\n");

    C_rc = 1 + ((-log(alpha) / H) / log(2));
    Bmax = 0;

    printf("\t > Repetition Count Test:  ");
    for (int i = 0; i < lb - C_rc; i++) {
        A = buffer[i];
        B = 1;
    next:
        X = buffer[i + B];
        if (X == A) {
            B++;
            if (B > C_rc) {
                printf("   FAILURE \n");
				if (DBG > 1) printf("\t\t -- Cutoff value (%d) reached at position %d \n", B, i);
                goto HC_2;
            }
            goto next;
        }
        if (Bmax < B) {
            Bmax = B;
        }
        i = i + B - 1;
    }

    if (DBG > 1) printf("\n\t\t > Max.rep/Cutoff.value=%d/%d \n\t\t > Result:", Bmax, C_rc);
	printf("   SUCCESS\n");
	HCS++;

    // ADAPTATIVE PROPORTION Test
    /*
    * Parameters:
    *
    *   C_ap    : f(H)
    *   W       : window length
    *   H       : estimated entropy
    *   alpha   : false-positive probability
    *
    *   C_ap  = 1+CRITBINOM(W, power(2,( −H)),1−alpha) ; (Excel formula quoted in SP800-90b - Section 4.4.2)
    *   W     = 1024;  (recommended in SP800-90b - Section 4.4.2)
    *   alpha = 2^(-20) ;  (recommended in SP800-90b - Section 4.3.3)
    */

    Bmax = 0;
	HC_2:
    printf("\t > Adaptive Proportion Test: ");
    for (int i = 0; i < lb - W; i++) {
        A = buffer[i];
        B = 1;
        for (int j = 1; j < W - 1; j++) {
            if (A == buffer[i + j]) B++;
            if (B >= C_ap) {
                printf(" FAILURE \n");
				if (DBG > 1) printf(" \t\t -- Cutoff value (%d) reached at position %d \n", B, i);
                goto step_out;
            }
        }
        if (Bmax < B) Bmax = B;
    }

	if (DBG > 1) printf("\n\t\t > Max.prop/Cutoff.value=%d/%d \n\t\t > Result:", Bmax, C_ap); 	
	printf(" SUCCESS \n");
	HCS++;
	step_out: 
	if (HCS == 2) {
		printf("\n      >> Entropy Source : VALIDATED \n\n");
	}else{
		printf("\n      >> Entropy Source : INVALID !!! \n\n");
	}

    return SUCCESS;
}
