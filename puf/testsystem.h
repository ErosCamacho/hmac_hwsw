/*****************************************************************************/
/*
 *  IMSE.CNM_SPIRS-HwSec_puf4r4_2.2: testsystem.h  
 *
 *  Created on: 07/09/2023
 *      Author: santiago@imse-cnm.csic.es
 */
/*****************************************************************************/

#ifndef TESTSYSTEM_H_INCLUDED
#define TESTSYSTEM_H_INCLUDED

/****************************** Global variables *****************************/

 #if defined(PYNQZ2)
  
	char * opt[] = {"00", "01"};
							   
	int PUF_BASEADDR[] = {0x43C50000, 0x43C60000};

  #if defined(PYNQ)				   			   
	char * InF    = "bit/SPIRS_RoT";
	char * OutF   = "out/puf/SPIRS_RoT";
  #else
	char * InF    = "SPIRS_RoT";
	char * OutF   = "SPIRS_RoT";
  #endif 
  
 #elif defined(G2RISCV)

	char * opt[] = {"00", "01"};
                   
	int PUF_BASEADDR[] = {0x60090000, 0x600A0000};
                    
	char * InF  = "bit/SPIRS_RoT";
	char * OutF = "out/puf/SPIRS_RoT";

 #endif

#endif // TESTSYSTEM_H_INCLUDED
