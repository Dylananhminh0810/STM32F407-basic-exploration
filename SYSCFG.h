#ifndef SYSCFG_H
#define SYSCFG_H

#include <stdint.h>
#define SYSCFG_ADDRESS 0x40013800

typedef struct
{ 
	uint32_t MEMRMP;
	uint32_t PMC;
	uint32_t EXTICR1;
	uint32_t EXTICR2;
	uint32_t EXTICR3;
	uint32_t EXTICR4;
	uint32_t CMPCR;
}Syscfg;


#endif