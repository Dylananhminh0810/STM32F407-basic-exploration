#include <stdint.h>
#include <stdbool.h>
#include "Register.h"
#include "RCC.h"
#include "GPIO.h"
#include "NVIC.h"
#include "EXTI.h"
#include "SYSCFG.h"
#include "Timer.h"
#include "DMA.h"
#include "UART.h"
#include "AXDL345.h"
#include "SPI.h"


static void initGPIOforSPI(void){
//Open data sheet
// 3 pins will be used
	GPIOGeneralRegister* GPIOA;
	GPIOA = (GPIOGeneralRegister*)GPIOA_BASE_ADDRESS;
	REG32(RCC_BASE_ADDRESS + RCC_AHB1_OFFSET) |= BIT_0;
	GPIOA->MODER |= (BIT_9 | BIT_11 | BIT_15);
	GPIOA->OTYPER &= ~((unsigned int)(BIT_4 | BIT_5 | BIT_7));
	GPIOA->OSPEEDR |= ((BIT_8 | BIT_9) | (BIT_10 | BIT_11) | (BIT_14 | BIT_15));
	GPIOA->PUPDR |= (BIT_8 | BIT_10 | BIT_15);
	GPIOA->AFRL  |= ((BIT_16 | BIT_18) | (BIT_20 | BIT_22) | (BIT_28 | BIT_30));
}
//not use the SDO - PA6
void initSPIRegister3(void){
//we first need to wake up the device and set it up in measure mode
	SerialPeripheralInterface* SPI;
	SPI = (SerialPeripheralInterface*)SPI1_;
	REG32(RCC_BASE_ADDRESS + RCC_APB2_OFFSET) |= BIT_12;
	SPI->SPI_CR1 |= (BIT_3 | BIT_4);
	SPI->SPI_CR1 |= (BIT_1 | BIT_0);
	SPI->SPI_CR1 |= BIT_2;
	SPI->SPI_CR2 |= BIT_2;
	SPI->SPI_CR1 |= BIT_6;
	
	SPI->SPI_DR = 0x2D;
	while (!(SPI->SPI_SR & BIT_1)){} // the buffer is not empty then wait
		
	SPI->SPI_DR = 0x08;
	while (!(SPI->SPI_SR & BIT_1)){} // the buffer is not empty then wait
		
	while (SPI->SPI_SR & BIT_7){} // check until the busy bit is set = 0, to make sure no SPI communication is running
	SPI->SPI_CR1 &= ~((unsigned int)BIT_6);
	
	//we will also have a delay between configuring the two regs, and for that we will use a loop
	static volatile int i = 0;
	while (i<10000) {i++;}
		
	//on page 8 of the sensor manual, setting data format reg we choose 3 wire mode
	//set BIT6 of DATA_FORMAT reg to 1
	SPI->SPI_CR1 |= BIT_6;
	SPI->SPI_DR = (0x31);
	while (!(SPI->SPI_SR & BIT_1)) {}
	SPI->SPI_DR = 0x40;
	while (!(SPI->SPI_SR & BIT_1)) {}
	
	while (SPI->SPI_SR & BIT_7){} // check until the busy bit is set = 0, to make sure no SPI communication is running
	SPI->SPI_CR1 &= ~((unsigned int)BIT_6);
	
	//after configure for the sensor we also need to configure for the STM32
	//More information can be found in the reference mmanual starting from page 884
		
	SPI->SPI_CR1 |= BIT_15; // BIDI MODE bit
}

static void SPI_Transmit3( uint8_t* data, int size){
	SerialPeripheralInterface* SPI;
	SPI = (SerialPeripheralInterface*) SPI1_;
	
	SPI->SPI_CR1 |= BIT_14; // bit allow to use as transmitter
	SPI->SPI_CR1 |= BIT_6;
	
	int k = 0;
	while (k < size){
		while (!(SPI->SPI_SR & BIT_1)) {}
		SPI->SPI_DR = data[k];
		k++;
	}
	while (!(SPI->SPI_SR & BIT_1)) {}
	while (SPI->SPI_SR & BIT_7){} 
	// clear the receiver reg to clear the overrun flag
	uint32_t temp = SPI->SPI_DR;
	//This function sends certain amount of data from an array to the sensor
}
static void SPI_Receive3(uint8_t* data, int size){
	SerialPeripheralInterface* SPI;
	SPI = (SerialPeripheralInterface*) SPI1_;
	//First we need to wait the previous communication to end
	while (SPI->SPI_SR & BIT_7){}
	SPI->SPI_CR1 &= ~((unsigned int)BIT_14);
	
	while (size){
		while (!(SPI->SPI_SR & BIT_0)){}
		*data++ = SPI->SPI_DR;
		size--;
	}
	while (SPI->SPI_SR & BIT_7){}
}

static void adxl_read3(uint8_t address, uint8_t* RxData, uint8_t dataSize){
	SerialPeripheralInterface* SPI;
	SPI = (SerialPeripheralInterface*) SPI1_;
	
	address |= 0x80;
	address |= 0x40;
	
	SPI_Transmit3(&address, 1);
	SPI_Receive3(RxData, dataSize);

	SPI->SPI_CR1 &= ~((unsigned int)BIT_6);
}

static uint8_t rxData;
static uint8_t RxData[6];
static volatile int16_t x = 0xFFFF, y = 0xFFFF, z = 0xFFFF;
int main(){
	initGPIOforSPI();
	initSPIRegister3();
	static volatile double xg, yg, zg;

	while (1) {
	//adxl_read3(0x32, &RxData[0], 1);
	//x = RxData[0];
	//adxl_read3(0x33, &RxData[1], 1);
	//x += RxData[1]<<8;
	//xg = 0.038 * x;
		
	//adxl_read3(0x34, &RxData[2], 1);
	//y = RxData[2];
	//adxl_read3(0x35, &RxData[3], 1);
	//y += RxData[3]<<8;
	//yg = 0.038 * y;
		
	//adxl_read3(0x36, &RxData[4], 1);
	//z = RxData[4];
	//adxl_read3(0x37, &RxData[5], 1);
	//z += RxData[5]<<8;
	//zg = 0.038 * z;
		
	adxl_read3(0x32, &RxData[0], 6);
	x = (RxData[0]);
	x += (RxData[1]<<8);
	xg = 0.038 * x;

	y = (RxData[2]);
	y += (RxData[3]<<8);
	yg = 0.038 * y;
	
	z = (RxData[4]);
	z += (RxData[5]<<8);
	zg = 0.038 * z;
	}
	
	return 0;
} 

