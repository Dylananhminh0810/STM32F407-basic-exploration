#ifndef SPI_H
#define SPI_H
#include <stdint.h>
#define SPI1_ 0x40013000
//The SPI reg map can be found in the manual on page 925
typedef struct {
	uint32_t SPI_CR1;
	uint32_t SPI_CR2;
	uint32_t SPI_SR;
	uint32_t SPI_DR;
	uint32_t SPI_CRCPR;
	uint32_t SPI_RXCRCR;
	uint32_t SPI_TXCRCR;
	uint32_t SPI_I2SCFGR;
	uint32_t SPI_I2SPR;
}SerialPeripheralInterface;

#endif