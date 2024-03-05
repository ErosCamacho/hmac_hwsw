/******************************************************************************/
/*
 *  IMSE.CNM_SPIRS-HwSec_puf4r4_2.2: puf_HDinter.c for PUF4R4_2.2  32/64-bits
 *
 *  Created on: 07/09/2023
 *      Author: santiago@imse-cnm.csic.es
 *
 *  Function:
 *   - Evaluates, for each of the PUFs instantiated in a Test System, the 
 *     Hamming Distance with respect to the other PUFs (HDinter) for successive 
 *     runs. To do this, first an enrollment process is performed for each PUF 
 *     to obtain the Reference Output and the RO Selection Mask that indicates  
 *     the discarded comparisons.
 *      
 *  Outputs:
 *   - DBG = 0 --> HDinter for each pair of PUFs and each run after applying the
 *                 selection mask 
 *                 + HDinter without removing the worst comparisons & Differences
 *                   between both cases (-a option)
 *                 + Average values for each reference PUF (if N_pufs > 2 or
 *                   N_runs > 1)
 *   - DBG = 1 --> + HDintra for enrollment stage of reference PUF 
 *                 + Processing times
 *   - DBG = 2 --> + Bitstream file of the Test System
 *                 + Time for bitstream loading (for PYNQ-Boards)
 *   Control:
 *     -i --> Wait for a key to be pressed between PUF and PUF (Interactive mode)
 */
 /******************************************************************************/

/******************************** Include Files *******************************/

#include "puf4r4.h"
#include "testsystem.h"

/******************************** Test parameters *****************************/

int N_CMPS     = 1024;     // -c Number of comparisons
int E_CMPS     = 102;      // -e Number of comparisons eliminated at enrollment
int SD         = 1;        // -s Same/different LUTs
int BG         = 1;        // -g Binary/Gray counters
int NR         = 0;        // -n Nearby/Remote RO pairs
int LH         = 0;        // -l Lower/Higher Bits
int N_tests    = 100;      // -t Number of test
int E_tests    = 100;      // -m Number of test for enrollment
int N_runs     = 1;        // -r Number of runs
int DBG        = 0;        // -d Debug level
int ARWCM      = 0;        // -a Add results without Challenges Mask
int INTER      = 0;        // -i Interactive mode (PUF to PUF)
char * BOARD   = "01";     // -b Board ID

int PE         = 1;        // PUF/Entropy Source
int CHD        = 1;        // Compute Hamming Distance
int R50        = 1;        // Round HDinter values to 50

char * T_code   = "---";   // Test code
char * T_board;            // Board ID

/******************************************************************************/
/******************************************************************************/


int main(int argc, char** argv) {

   MMIO_WINDOW puf;
  
#if defined(AXI64)  
   long long puf_selr[(int)ceil((MN_CMPS/64.))];
#else   
   int puf_selr[(int)ceil((MN_CMPS/32.))];
#endif

   int board;
   int N_pufs;
   int puf_ref[MN_CMPS];
   int puf_eref[MN_CMPS];
   int puf_selm[MN_CMPS];
   int op;
    
   unsigned int CONFIG;
   unsigned long long toc, tr, ts;
   
   HammDist HDinter1;
   HammDist HDinter2;
   HammDist HDref;
   
   double HDimean1, HDimean2;
   double HDimax1, HDimax2;
   double HDimin1, HDimin2;
      
 #if defined(PYNQ)
   char * bitstream_file;
   char input_file[100];
 #endif

   // char idtrail[20];
   char cboard[3];
   char ctcode[4];
   
 #if !defined(STANDALONE)

   int arg;
   for(arg=1; arg < argc; arg++) {
      if(argv[arg][0] == '-') {
         if(argv[arg][1] == 'h') {
            printf("\n Usage: puf_HDinter [ -h | [-g] [-n] [-l] [-a] [-i] [-b <board_id>] [-d <dbg_level>] [-c <n_cmpc>]");
            printf("\n                    [-e <n_cmps>] [-m <n_tests>] [-t <n_tests>] [-r <n_runs>] ]\n\n");
            return SUCCESS;   
         } else if(argv[arg][1] == 'b') { 
            if(arg<argc-1) {
               board = atoi(argv[arg+1]);
               sprintf(cboard, "%02d", board);
               BOARD = cboard;               
            }
         } else if(argv[arg][1] == 'd') { 
            if(arg<argc-1) DBG = atoi(argv[arg+1]);
         } else if(argv[arg][1] == 'c') {
            if(arg<argc-1) N_CMPS = atoi(argv[arg+1]);
            if(N_CMPS>MN_CMPS) N_CMPS=MN_CMPS;
            if(N_CMPS<8) N_CMPS=8;
         } else if(argv[arg][1] == 't') {
            if(arg<argc-1) N_tests = atoi(argv[arg+1]);
            if(N_tests<2) N_tests=2;
         } else if(argv[arg][1] == 'm') {
            if(arg<argc-1) E_tests = atoi(argv[arg+1]);
            if(E_tests>MN_TESTS) E_tests=MN_TESTS;
            if(E_tests<10) E_tests=10;
         } else if(argv[arg][1] == 'r') {
            if(arg<argc-1) N_runs = atoi(argv[arg+1]);
            if(N_runs<1) N_runs=1;
         } else if(argv[arg][1] == 'e') {
            if(arg<argc-1) E_CMPS = atoi(argv[arg+1]);
         } else if(argv[arg][1] == 'a') {
            ARWCM=1;
         } else if(argv[arg][1] == 'i') {
            INTER=1;
         } else if(argv[arg][1] == 'g') {
            BG=0;
         } else if(argv[arg][1] == 'n') {
            NR=1;
         } else if(argv[arg][1] == 'l') {
            LH=1;
         // } else if(argv[arg][1] == 's') {
            // SD=1;
         } else {
            printf("\n unknown option: %s\n\n", argv[arg]);
            return ERROR;
         }
      }   
   } 
   
 #endif

   N_pufs = sizeof(opt)/sizeof(opt[0]);  
   
   if (PE == 0) {
      T_code = "ES";
   } else {
      sprintf(ctcode, "%01d%01d%01d", BG, NR, LH);
      T_code = ctcode;
   }
   
   T_board = BOARD;
   if (strcmp(T_board, "00") == 0) {
      printf("\nBoard ID: ");
      scanf("%d", &board);
      sprintf(cboard, "%02d", board);
      T_board = cboard;
   }
       
   //sprintf(idtrail, "_%d_%d_%d", Nx*Ny, Ns, Nbc);
       
   printf("\n  --- Start Test ---> T_board %s, T_code %s, N_runs %d, E_tests %d, N_tests %d, N_cmps %d, E_cmps %d\n",
          T_board, T_code, N_runs, E_tests, N_tests, N_CMPS, E_CMPS);
     
 #if defined(PYNQ)
   strcpy(input_file, InF);
   // strcat(input_file, idtrail);
   bitstream_file = strcat(input_file, ".bit");
   if (DBG > 1) {
      printf("\n   --- Bitstream_file: %s\n", bitstream_file);
   }
   if (DBG < 2) {
      printf("\n");
   }

   // Load Bitstream
   PUF_loadBitstream(bitstream_file, DBG+1); 
   if (DBG > 1) {
      printf("\n");
   }
 #endif
    
   CONFIG =  (PE<<6) +( SD<<5) + (BG<<4) + (NR<<3) + (LH<<2);
   
   // For each PUF instance (sr)
   for (int sr=0; sr<N_pufs; sr++) {
	   
 #if defined(STANDALONE)
      puf = PUF_BASEADDR[sr];
 #endif
      // Create PUF interface
      PUF_createMMIOWindow(&puf, PUF_BASEADDR[sr], PUF_LENGTH, 0); 
	  
      // Reset Challenges Mask
	  op = 0;
      writeMMIO(&puf, &op, CHLADDC, sizeof(op));	 

      for (int c=0; c<ceil(MN_CMPS/8.)*8; c++) {
     	  puf_selm[c] = 0;
      }
      for (int r=0; r<ceil(MN_CMPS/Dbw); r++) {
      //for (int r=0; r<ceil(MN_CMPS/32.); r++) {     /////////////////////////////
     	  puf_selr[r] = 0;
      }

	  ts = Wtime();
	  
      // Get PUF reference output for N_CMPS
	  HDref = PUF_enrollment(&puf, 1, CHD, CONFIG, E_tests, N_CMPS, E_CMPS,
	                         puf_ref, puf_selm);
 
      if (DBG > 0) { 										  
	     printf("     HDintra = %6.3f [%6.3f-%6.3f]", HDref.mean, HDref.min, HDref.max);
		 toc = Wtime() - ts;
 	     if (ARWCM == 1) {       
            printf("                                      --       ");
         }			
         printf(" (%.3f s)", toc/1000000.);
      }
	  
	  // Get PUF reference output for N_CMPS-E_CMPS
      int cr = 0;
      for (int c=0; c<N_CMPS; c++) {
     	 if (puf_selm[c] == 0) {
		 puf_eref[cr] = puf_ref[c];
		 cr++;
         }
	  }

      chl_btor( puf_selm, N_CMPS, puf_selr);
 
	  HDimean1 = 0;   HDimean2 = 0;
	  HDimax1  = 0;   HDimax2  = 0;
 	  HDimin1  = 100; HDimin2  = 100;
	  
      for (int s=0; s<N_pufs; s++) {
		  
         if (s != sr) {
			 
            printf("\n    ");
			
 #if defined(STANDALONE)
            puf = PUF_BASEADDR[s];
 #endif
            // Create PUF interface
            PUF_createMMIOWindow(&puf, PUF_BASEADDR[s], PUF_LENGTH, 0); 
			
     	    // For each run
            for (int r=1; r<=N_runs; r++) {
      		  
       		   tr = Wtime();

               // Enable/Disable Challenges Mask
               if (E_CMPS == 0) {
      	          op = 0;
      	       } else {
 #if defined(AXI64)
                  PUF_writeChallegesMask(&puf, puf_selr, ceil(MN_CMPS/64.), 0); 
 #else
                  PUF_writeChallegesMask(&puf, puf_selr, ceil(MN_CMPS/32.), 0); 
 #endif
                  op = RCHL;
      	       }
               writeMMIO(&puf, &op, CHLADDC, sizeof(op));	 
	 		 			                	  
      	       HDinter1 = PUF_HD(&puf, 1, CONFIG, puf_eref, puf_selm, N_tests, N_CMPS, E_CMPS);
			   
			   if (R50 == 1) {
				  if (HDinter1.mean > 50) HDinter1.mean = 100-HDinter1.mean;
				  if (HDinter1.min > 50)  HDinter1.min  = 100-HDinter1.min;
			      if (HDinter1.max > 50)  HDinter1.max  = 100-HDinter1.max;
			   }
     	       printf(" HDinter = %6.3f [%6.3f-%6.3f]", HDinter1.mean, HDinter1.min, HDinter1.max);
     			
       		   HDimean1 = HDimean1 + HDinter1.mean;
      		   if (HDinter1.max > HDimax1) HDimax1 = HDinter1.max;
      		   if (HDinter1.min < HDimin1) HDimin1 = HDinter1.min;
      		 
	           if (ARWCM == 1) {
                  printf(" -->");
		          op = 0;
                  writeMMIO(&puf, &op, CHLADDC, sizeof(op));
      	          HDinter2 = PUF_HD(&puf, 0, CONFIG, puf_ref, puf_selm, N_tests, N_CMPS, 0);
      			
			      if (R50 == 1) {
			         if (HDinter2.mean > 50) HDinter2.mean = 100-HDinter2.mean;
			         if (HDinter2.min > 50)  HDinter2.min  = 100-HDinter2.min;
			         if (HDinter2.max > 50)  HDinter2.max  = 100-HDinter2.max;
				  }
				  
    	          printf(" HDinter = %6.3f [%6.3f-%6.3f]", HDinter2.mean, HDinter2.min, HDinter2.max);
				  
      		      HDimean2 = HDimean2 + HDinter2.mean;
      		      if (HDinter2.max > HDimax2) HDimax2 = HDinter2.max;
      		      if (HDinter2.min < HDimin2) HDimin2 = HDinter2.min;
      			
                  printf(" --  %5.2f", HDinter1.mean-HDinter2.mean);
				  
 #if !defined(STANDALONE)
                  // Close PUF interface
                  int Status;
                  Status = closeMMIOWindow(&puf);
                  if (Status != SUCCESS) {
				     printf(" closeMMIOWindow failed !!!\n\n");
                     return ERROR;
                  } 
#endif

               }  // end If ARWCM	

               if (DBG > 0) { 
                  toc = Wtime() - tr;
                  printf(" (%.3f s)", toc/1000000.);
               }
			   
               if ((N_runs > 1) & (r < N_runs)) {	
                  printf("\n              run %3d ---", r+1);
               }
			         		 
            }	 // end runs
		  		
		 }  // end if
		
	  }  //  end pufs
	  
  	  if ((N_pufs > 2) | (N_runs > 1)) {
         printf("\n       ---  Average   --- HDinter = %6.3f [%6.3f-%6.3f]", HDimean1/((N_pufs-1)*N_runs), HDimin1, HDimax1);
         if (ARWCM == 1) {
            printf(" --> HDinter = %6.3f [%6.3f-%6.3f] --  %5.2f", HDimean2/((N_pufs-1)*N_runs), HDimin2, HDimax2, 
                   (HDimean1-HDimean2)/((N_pufs-1)*N_runs) );
         }
      }
      if (DBG > 0) {
         toc = Wtime() - ts;
         printf(" (%.3f s)", toc/1000000.);
      }
 
      if ((INTER == 1) & (sr < N_pufs-1)) {
         printf("\n\n   Press any key to continue");
         getchar();
      }

      printf("\n\n");	
	  
   }  // end puf ref
     
   printf("  --- End Test   \n\n");	
}
  
