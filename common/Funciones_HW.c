#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "pynq_api.h"

void load_bitstream(int DBG) {

	unsigned long long tic = 0, toc;
	int Status;
	char bitstream_file[80];

	/*
	if(version == 32)	sprintf(bitstream_file, "%s", "BD_wrapper_32.bit");
	else				sprintf(bitstream_file, "%s", "BD_wrapper_64.bit");
	*/

	sprintf(bitstream_file, "%s", "RoT.bit");

	FILE* file;

	if (!(file = fopen(bitstream_file, "r"))) {
		printf("\n   Bitstream not available. Bye ...\n\n");
	}
	fclose(file);

	tic = PYNQ_Wtime();
		Status = PYNQ_loadBitstream(bitstream_file);
		if (Status != PYNQ_SUCCESS) {
			printf(" Failed Loading !!!\n");
		}
	toc = PYNQ_Wtime() - tic;
	if(DBG == 1) printf("\n Load Bitsream (HW) \t \t ... \t (%6llu us.) \n ", toc);
}

