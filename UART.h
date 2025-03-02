#ifndef UART_H
#define UART_H

#include <stdint.h>

#define UART_4 0x40004C00
typedef struct{
	uint32_t SR;
	uint32_t DR;
	uint32_t BRR;
	uint32_t CR1;
	uint32_t CR2;
	uint32_t CR3;
	uint32_t GTPR;
} Uart;


#endif 