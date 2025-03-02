#ifndef RCC_H
#define RCC_H
#include <stdint.h>
#define RCC_BASE_ADDRESS 0x40023800
#define RCC_AHB1_OFFSET 0x30
#define RCC_APB2_OFFSET 0x44
#define RCC_APB1_OFFSET 0x40

typedef struct {

	uint32_t CR;
	uint32_t PLLCFGR;
	uint32_t CFGR;
	uint32_t CIR;
	uint32_t AHB1RSTR;
	uint32_t AHB2RSTR;
	uint32_t AHB3RSTR;
	uint32_t none;
	uint32_t APB1RSTR;
	uint32_t APB2RSTR;
	uint32_t none1;
	uint32_t none2;
	uint32_t AHB1ENR;
	uint32_t AHB2ENR;
	uint32_t AHB3ENR;
	uint32_t none3;
	uint32_t APB1ENR;
	uint32_t APB2ENR;
	uint32_t none4;
	uint32_t none5;
	uint32_t AHB1LPENR;
	uint32_t AHB2LPENR;
	uint32_t AHB3LPENR;
	uint32_t none6;
	uint32_t APB1LPENR;
	uint32_t APB2LPENR;
	uint32_t none7;
	uint32_t none8;
	uint32_t BDCR;
	uint32_t CSR;
	uint32_t none9;
	uint32_t none10;
	uint32_t SSCGR;
	uint32_t PLLI2SCFGR;	
} ResetandClockControl;
#endif