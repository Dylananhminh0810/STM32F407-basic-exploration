#define DAC 0x40007400
#include <stdint.h>
typedef struct{
	uint32_t DAC_CR;
	uint32_t DAC_SWTRIGR;
	uint32_t DAC_DHR12R1;
	uint32_t DAC_DHR12L1;
	uint32_t DAC_DHR12R2;
	uint32_t DAC_DHR12L2;
	uint32_t DAC_DHR8R2;
	uint32_t DAC_DHR12RD;
	uint32_t DAC_DHR8RD;
	uint32_t DAC_DOR1;
	uint32_t DAC_DOR2;
	uint32_t DAC_SR;
} DACStruct;
