#ifndef EXTI_H
#define EXTI_H
#include <stdint.h>
#define EXTI_ADDRESS 0x40013C00
typedef struct
{
	uint32_t IMR;
	uint32_t EMR;
	uint32_t RTSR;
	uint32_t FTSR;
	uint32_t SWIER;
	uint32_t PR;
}Exti;


#endif