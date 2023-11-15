#include <pynq_api.h>
#include "params.h"

/************************ MS2XL Constant Definitions **********************/

#define MS2XL_LENGTH   0x40

#define RESET					1
#define LOAD					2
#define START					4
#define LOAD_PADDING			8

#define DATA_IN  0x0		/**< data_in */
#define ADDRESS  0x8		/**< address */
#define CONTROL  0x10		/**< control */
#define DATA_OUT 0x18		/**< data_out */
#define END_OP   0x20		/**< end_op */

/************************ MS2XL Function Definitions **********************/

void sha2_ms2xl_init(PYNQ_MMIO_WINDOW ms2xl, unsigned long long int length, int DBG);
void sha2_ms2xl_init_sha2_256(PYNQ_MMIO_WINDOW ms2xl, unsigned long long int length, int DBG);
void sha2_ms2xl_init_sha2_512(PYNQ_MMIO_WINDOW ms2xl, unsigned long long int length, int DBG);
void sha2_ms2xl(unsigned long long int* a, unsigned long long int* b, unsigned long long int length, PYNQ_MMIO_WINDOW ms2xl, int last_hb, int DBG);
void sha2_ms2xl_256(unsigned long long int* a, unsigned long long int* b, unsigned long long int length, PYNQ_MMIO_WINDOW ms2xl, int last_hb, int DBG);
void sha2_ms2xl_512(unsigned long long int* a, unsigned long long int* b, unsigned long long int length, PYNQ_MMIO_WINDOW ms2xl, int last_hb, int DBG);
void sha2_hw(unsigned char* in, unsigned char* out, unsigned long long int length, PYNQ_MMIO_WINDOW ms2xl, int DBG);
void sha2_hw_256(unsigned char* in, unsigned char* out, unsigned long long int length, PYNQ_MMIO_WINDOW ms2xl, int DBG);
void sha2_hw_512(unsigned char* in, unsigned char* out, unsigned long long int length, PYNQ_MMIO_WINDOW ms2xl, int DBG);

