#ifndef DMA_H
#define DMA_H

#include <stdint.h>

#define DMA2_BASE_ADDRESS 0x40026400
#define DMA1_BASE_ADDRESS 0x40026000

typedef struct{
	uint32_t LISR;
	uint32_t HISR;
	uint32_t LIFCR;
	uint32_t HIFCR;
	uint32_t S0CR;
	uint32_t S0NDTR;
	uint32_t S0PAR;
	uint32_t S0M0AR;
	uint32_t S0M1AR;
	uint32_t S0FCR;
	uint32_t S1CR;
	uint32_t S1NDTR;
	uint32_t S1PAR;
	uint32_t S1M0AR;
	uint32_t S1M1AR;
	uint32_t S1FCR;
	uint32_t S2CR;
	uint32_t S2NDTR;
	uint32_t S2PAR;
	uint32_t S2M0AR;
	uint32_t S2M1AR;
	uint32_t S2FCR;
	uint32_t S3CR;
	uint32_t S3NDTR;
	uint32_t S3PAR;
	uint32_t S3M0AR;
	uint32_t S3M1AR;
	uint32_t S3FCR;
	uint32_t S4CR;
	uint32_t S4NDTR;
	uint32_t S4PAR;
	uint32_t S4M0AR;
	uint32_t S4M1AR;
	uint32_t S4FCR;
	uint32_t S5CR;
	uint32_t S5NDTR;
	uint32_t S5PAR;
	uint32_t S5M0AR;
	uint32_t S5M1AR;
	uint32_t S5FCR;
	uint32_t S6CR;
	uint32_t S6NDTR;
	uint32_t S6PAR;
	uint32_t S6M0AR;
	uint32_t S6M1AR;
	uint32_t S6FCR;
	uint32_t S7CR;
	uint32_t S7NDTR;
	uint32_t S7PAR;
	uint32_t S7M0AR;
	uint32_t S7M1AR;
	uint32_t S7FCR;
} DirectMemoryAccess;

#endif