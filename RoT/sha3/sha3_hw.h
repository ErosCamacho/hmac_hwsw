#include <pynq_api.h>
#include "params.h"

/************************ MS2XL Constant Definitions **********************/

#define MS2XL_LENGTH   0x40

#define RESET					1
#define LOAD_PADDING			2
#define START_PADDING			4
#define LOAD					8
#define START					16
#define	READ					32
#define	START_PADDING_DOUBLE	64

#define DATA_IN  0x0		/**< data_in */
#define ADDRESS  0x8		/**< address */
#define CONTROL  0x10		/**< control */
#define DATA_OUT 0x18		/**< data_out */
#define END_OP   0x20		/**< end_op */

/************************ MS2XL Function Definitions **********************/

void sha3_ms2xl_init(PYNQ_MMIO_WINDOW ms2xl);
void sha3_ms2xl(unsigned long long int* a, unsigned long long int* b, PYNQ_MMIO_WINDOW ms2xl, unsigned long long int pos_pad, int pad, int DBG);
void sha3_ms2xl_256(unsigned long long int* a, unsigned long long int* b, PYNQ_MMIO_WINDOW ms2xl, unsigned long long int pos_pad, int pad, int DBG);
void sha3_ms2xl_512(unsigned long long int* a, unsigned long long int* b, PYNQ_MMIO_WINDOW ms2xl, unsigned long long int pos_pad, int pad, int DBG);
void sha3_hw(unsigned char* in, unsigned char* out, unsigned long long int length, PYNQ_MMIO_WINDOW ms2xl, int DBG);
void sha3_hw_256(unsigned char* in, unsigned char* out, unsigned long long int length, PYNQ_MMIO_WINDOW ms2xl, int DBG);
void sha3_hw_512(unsigned char* in, unsigned char* out, unsigned long long int length, PYNQ_MMIO_WINDOW ms2xl, int DBG);
