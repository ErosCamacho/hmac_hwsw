/******************************************************************************/
/*
 *  IMSE.CNM_SPIRS-HwSec_puf4r4_2.2: trng_getdata.c for PUF4R4_2.2  32/64-bits
 *
 *  Updated on: 28/11/2022
 *      Author: rojas@imse-cnm.csic.es
 *
 *  Updated on: 07/09/2023
 *      Author: santiago @imse-cnm.csic.es
 *
 *  Function:
 *   - Collects 10^6 random bits by running 2000 tests for the PUF instantiated 
 *     in the System. This amount of bits is reached by performing 125 ROs 
 *     comparisons and making use of the PUF's TRNG functionality.
 *
 *  Outputs:
 *               Filename of the implemented Test System Bitstream 
 *               + Full Processing time
 *   DBG = 0 --> + Bin file in the "out/puf/" directory containing 10^6 random bits 
 *   DBG = 1 --> + Number of PUF/TRNG output registers containing random data  
 *               + Time to collect the random data for each test
 *   DBG = 2 --> + Content of the PUF/TRNG output registers for each test
 *   DBG = 3 --> + Time for bitstream loading (for PYNQ-Boards)
 *               + Time invested in each stage of operation of the PUF/TRNG
 *   DBG = 4 --> + Wait cycles until getting the response from the PUF/TRNG
 *
 *   Output Files:
 *   
 *          SPIRS_RoT_<puf>_ES_<conf>_22.txt
 *          SPIRS_RoT_<puf>_ES_<conf>_90.bin
*/
/******************************************************************************/

/******************************** Include Files *******************************/

#include "puf4r4.h"
#include "testsystem.h" 

/******************************** Test parameters *****************************/

int N_CMPS     = 2000;     // Number of comparisons
int PE         = 0;        // Forced to perform as Entropy Source (in Operation mode)
int SD         = 1;        // -s Same/different LUTs
int BG         = 1;        // -g Binary/Gray counters
int NR         = 0;        // -n Nearby/Remote RO pairs
int LH         = 0;        // Lower/Higher Bits
int N_tests    = 125 ;     // Number of test
int N_runs     = 1;        // Number of runs
int DBG        = 0;        // -d Debug level
int OUT_FILE   = 1;        // Generate output files
char * BOARD   = "01";     // -b Board ID

char * T_code   = "---";   // Test code
char * T_board;            // Board ID
 
/******************************************************************************/
/******************************************************************************/


int main(int argc, char** argv) {

   int Status;
   int board;
   int triesr;
   int last_reg;
   int ns = 0;		  
   
#if defined(AXI64)
   unsigned long long puf_output[MN_REGS/2];
   // Added TRNG
   unsigned long long es_output; // [64] = {0x0000000000000000}
   unsigned long long xor0=0,xor1=1;
   unsigned long long bit_3=0,bit_2=0,bit_1=0,bit_0=0;
   // End added TRNG													  					
#else
   unsigned int puf_output[MN_REGS];
	// Added TRNG
   unsigned int es_output; // [32] = {0x00000000}
   unsigned int xor0=0,xor1=1;
   unsigned int bit_3=0,bit_2=0,bit_1=0,bit_0=0;
   // End added TRNG
#endif	

   unsigned int CONFIG;
   unsigned long long toc, ts, tr, tt;
   
 #if defined(PYNQ)			
   char * bitstream_file;
   char input_file[100];
 #endif
 
   char output_file[100];
   // char crun[3];
   // char idtrail[20], odtrail[20];
   char cboard[3];
   char ctcode[6];
   char aux_file[100];            
   char bin_file[100];
   
   FILE *fptr;
   FILE *fp;
   MMIO_WINDOW puf;
  
 #if !defined(STANDALONE)

   int arg;
   for(arg=1; arg < argc; arg++) {
      if(argv[arg][0] == '-') {
         if(argv[arg][1] == 'h') {
            printf("\n Usage: trng_getdata [ -h | [-g] [-n] [-i <puf_instance>]");
			printf("\n                     [-b <board_id>] [-d <dbg_level>] ]\n\n");
            return 0;  
         } else if(argv[arg][1] == 'b') { 
            if(arg<argc-1) {
               board = atoi(argv[arg+1]);
               sprintf(cboard, "%02d", board);
               BOARD = cboard;          
            }
         } else if(argv[arg][1] == 'd') { 
            if(arg<argc-1) DBG = atoi(argv[arg+1]);
         } else if(argv[arg][1] == 'i') { 
            if(arg<argc-1) ns = atoi(argv[arg+1]);
			if(ns < 0) ns = 0;
			if(ns > sizeof(opt)/sizeof(opt[0])) {
				ns = sizeof(opt)/sizeof(opt[0]); 
			}
         }else if(argv[arg][1] == 'g') {
            BG=0;
         } else if(argv[arg][1] == 'n') {
            NR=1;
         // } else if(argv[arg][1] == 's') {
            // SD=1;
         } else {
            printf("\n unknown option: %s\n\n", argv[arg]);
            return ERROR;
         }
      }
   }
   
	int lb=4*N_CMPS*N_tests;  
	unsigned char buffer[lb];
	int bin_cnt=0;			 
	int limit=32; 	

#endif 

   sprintf(ctcode, "ES_%01d%01d", BG, NR);
   T_code = ctcode;
   
   T_board = BOARD;
   if (strcmp(T_board, "00") == 0) {
      printf("\nBoard ID: ");
      scanf("%d", &board);
      sprintf(cboard, "%02d", board);
      T_board = cboard;
   }
       
   // sprintf(idtrail, "_%d_%d_%d", Nx*Ny, Ns, Nbc);
   // sprintf(odtrail, "_%d_%d_%d", Nx*Ny, Ns, Nbc);
          
   //Length adjustment due to XOR post-process
   if ((BG != 1) || (NR != 0)) {
	  N_tests=N_tests*2;		
   }	  
	
   printf("\n\t>> PUF - ENTROPY SOURCE VALIDATION \n");
   
   printf("\n\t1) COLLECTION OF RANDOM DATA : %d bits ",lb);
   printf("\n\t   running...\n\n");
   
   ts = Wtime();
   
 #if defined(PYNQ)			
   // Load Bitstream
   strcpy(input_file, InF);
   //strcat(input_file, idtrail);
   bitstream_file = strcat(input_file, ".bit");
   if (DBG > 0) printf("\t--- Bitstream_file: %s\n", bitstream_file);
      
   PUF_loadBitstream(bitstream_file, DBG); 
   
 #endif

   CONFIG =  (PE<<6) +( SD<<5) + (BG<<4) + (NR<<3) + (LH<<2);
   
   // For PUF instance (s)
   for (int s=ns; s<ns+1; s++) {
	   
      // For each Test Run (run)  ---  (uncomment the selected option)
      
      for (int run=1; run<=N_runs; run++) {    // Generate r runs
      //for (int run=N_runs; run<=N_runs; run++) {  // Generate only run r
    
         if (OUT_FILE == 1) {      
            //sprintf(crun, "%02d", run);
            strcpy(aux_file, OutF);  
            //strcat(output_file, odtrail);
            strcat(aux_file, "_");     
            // strcat(aux_file, T_board);
            // strcat(aux_file, "_");     
            strcat(aux_file, opt[s]);
            strcat(aux_file, "_");     
            strcat(aux_file, T_code);

			//SP800-22
            strcpy(output_file, aux_file);
			strcat(output_file, "_22.txt");      
            fptr = fopen(output_file,"w");  
			 
			// SP800-90b
			bin_cnt = 0;
			strcpy(bin_file, aux_file);
			//strcat(bin_file, "_rst");
			strcat(bin_file, "_90.bin");
			fp = fopen(bin_file, "wb");
         }
         printf("\t--- Run %3d --- %s ", run, opt[s]);

 #if defined(STANDALONE)
         puf = PUF_BASEADDR[s];
 #endif
         // Create PUF interface
         PUF_createMMIOWindow(&puf, PUF_BASEADDR[s], PUF_LENGTH, DBG);  

          // Reset Challenges Mask
		 unsigned int op;
		 op = 0;
         writeMMIO(&puf, &op, CHLADDC, sizeof(op));	 

		 triesr = 0;			
         tr = Wtime();  
		
         // For each Test (t)
         for (int t=1; t<=N_tests; t++) {
           
            tt = Wtime();
				                        
            if ((DBG > 0) & (DBG < 4)) {
               printf( "\n      -- Test: %3d  ", t);
            }
            
            // Reset, configure and generate PUF responses
            last_reg = PUF_applyChallenges(&puf, CONFIG, N_CMPS, DBG);
            
			if (last_reg < ceil(N_CMPS/(Dbw/4.))) {
               triesr++;
               t = t-1;
            }
            
            //Read PUF results
            Status = PUF_readOutput(&puf, N_CMPS, puf_output, DBG);
            if (Status == ERROR) {
               printf("failed!\n");
            }
            
            if ((OUT_FILE == 1) | (DBG == 2)) {
               for (int r=0; r<ceil(N_CMPS/(Dbw/4.)); r++) {
               if (OUT_FILE == 1) {

 #if defined(STANDALONE)
                  printf(" %8.8x", puf_output[r]);    // Hex output
 #else
	    #if defined(AXI64)
				es_output = puf_output[r];
					
				// Limit to 10^6 bits
				if ( r==15 ) 
					{
						limit=64;
					} else {
						limit=64;
					}
				
				//(binary/remote)
				if ((BG == 1) && (NR == 0)) {

					
				   for (int dd=0; dd<limit; dd++) {
					fprintf(fptr, "%llu", (es_output & 0x0000000000000001));
					
					buffer[bin_cnt]=(es_output & 0x0000000000000001);
					bin_cnt++;
					
					es_output = es_output >> 1;
				  }

				//xor 
				} else {
				   for (int dd=0; dd<limit/4; dd++) {
					bit_3=(es_output & 0x0000000000000008)>>3;
					bit_2=(es_output & 0x0000000000000004)>>2;
					bit_1=(es_output & 0x0000000000000002)>>1;
					bit_0=(es_output & 0x0000000000000001);
					
					if (bit_3 == bit_1){
						fprintf(fptr,"%llu",xor0);
						
						buffer[bin_cnt]=xor0;
						bin_cnt++;
					}else {
						fprintf(fptr,"%llu",xor1);
						
						buffer[bin_cnt]=xor1;
						bin_cnt++;
					}
					
					if (bit_2 == bit_0){
						fprintf(fptr,"%llu",xor0);
						
						buffer[bin_cnt]=xor0;
						bin_cnt++;
					}else {
						fprintf(fptr,"%llu",xor1);
						
						buffer[bin_cnt]=xor1;
						bin_cnt++;
					}
					es_output = es_output >> 4;
				 }  
		     } 
				
        #elif defined(AXI32)
                  			  
			 es_output = puf_output[r];
					
				// Limit 
				if ( r==(ceil(N_CMPS/(Dbw/4.))-1)) 
					{
						limit=32;       //////////////////////////////////////////////
					} else {
						limit=32;
					} 
				
				//(binary/remote)
				//if ((BG == 1) && (NR == 0)) {
				if (1) {
					
				   for (int dd=0; dd<limit; dd++) {
					fprintf(fptr, "%d", (es_output & 0x00000001));
					
					buffer[bin_cnt]=(es_output & 0x00000001);
					bin_cnt++;
					
					es_output = es_output >> 1;
				  }

				//xor 
				} else {
				   for (int dd=0; dd<limit/4; dd++) {
					bit_3=(es_output & 0x00000008)>>3;
					bit_2=(es_output & 0x00000004)>>2;
					bit_1=(es_output & 0x00000002)>>1;
					bit_0=(es_output & 0x00000001);
					
					if (bit_3 == bit_1){
						fprintf(fptr,"%d",xor0);
						
						buffer[bin_cnt]=xor0;
						bin_cnt++;
					}else {
						fprintf(fptr,"%d",xor1);
						
						buffer[bin_cnt]=xor1;
						bin_cnt++;
					}
					
					if (bit_2 == bit_0){
						fprintf(fptr,"%d",xor0);
						
						buffer[bin_cnt]=xor0;
						bin_cnt++;
					}else {
						fprintf(fptr,"%d",xor1);
						
						buffer[bin_cnt]=xor1;
						bin_cnt++;
					}
					es_output = es_output >> 4;
				   }  
				}
				  
        #else
                  fprintf(fptr, " %8.8X", puf_output[r]);    // Hex output
        #endif

 #endif
                  }
               }
            }
            if (OUT_FILE == 1) {
 #if defined(STANDALONE)
                printf("\n");
 #else
                fprintf(fptr, "\n");
 #endif
            }
            
            if (DBG == 2) {
               printf("\n");
            }
            
            if (DBG == 1) {
               toc = Wtime() - tt;
               printf("  -- (%.2f ms)", toc/1000.);    // Test time
            }        
            
         }  //  end tests
		 
         if (DBG > 0) {
            printf("\n");
            printf("\n   -- Registers: %3d,  Tries failed: %3d,  ", last_reg, triesr);                                      
         }
         
         toc = Wtime() - tr;              // Run time
         printf("  (%.3f s)", toc/1000000.);
         if (DBG > 0) {
            printf("\n");
         }
         if (OUT_FILE == 1) {      
            fclose(fptr);
         }

 #if !defined(STANDALONE)
         // Close PUF interface
         Status = closeMMIOWindow(&puf);
         if (Status != SUCCESS) {
            printf(" closeMMIOWindow failed !!!\n\n");
            return ERROR;
        }
 
#endif

		if (OUT_FILE == 1) {			

			//SP900-90B
			size_t element_size = sizeof *buffer;
			size_t elements_to_write = sizeof buffer;

			/* Writes buffer (_including_ the NUL-terminator) to the binary file. */
			size_t elements_written = fwrite(buffer, element_size, elements_to_write, fp); 
			if (elements_written != elements_to_write)
			{
				/* This works for >=c99 only, else the z length modifier is unknown. */
				fprintf(stderr,"fwrite() failed: wrote only %zu out of %zu elements.\n",elements_written,elements_to_write);
			}

			fclose(fp);
		}
      }  //  end runs
	  
	  if (DBG > 0) {
		 toc = Wtime() - ts;
         printf("\n  --- End Test ---   (%.3f s)\n", toc/1000000.);
      }
	  	  	  
   }  // end pufs
   
   if (DBG == 1) printf("\t--- Output File : %s\n", bin_file);
   printf("\n\t--- Data collection finished!!! \n\n");

}
 
