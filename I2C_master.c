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
#include "I2C.h"


static void initGPIOI2C(void){
	GPIOGeneralRegister* GPIOB;
	GPIOB = (GPIOGeneralRegister*)GPIOB_BASE_ADDRESS;
	REG32(RCC_BASE_ADDRESS + RCC_AHB1_OFFSET) |= BIT_1;
	
	GPIOB->MODER |= (BIT_13 | BIT_15);
	GPIOB->OTYPER |= (BIT_6 | BIT_7);
	GPIOB->OSPEEDR |= (BIT_13 | BIT_15);
	GPIOB->AFRL |= (BIT_26 | BIT_30);
}

static void initI2CRegsTransmit(void){
	I2CRegister* I2C;
	I2C = (I2CRegister*)I2C_1;
	REG32(RCC_BASE_ADDRESS + RCC_APB1_OFFSET) |= BIT_21;
	
	I2C->I2C_CR2 |= 0x10;
	//0001 0000 --> 16MHz
	
	//choose standard mode
	I2C->I2C_CCR &= ~((unsigned int)BIT_15);
	
	//T high = T low in standard mode
	//T PCLK1 = 1 / FREQ = 1 / 16MHz = 62.5ns
	//CCR - prescaler for I2C
	// F = 100kHz
	// T = 10us
	// --> T high = T low = 1/2 T = 5us = CCR(PSC) * T PCLK1 --> CCR = T / (2 * T PCLK1)
	// --> CCR = 5us / 62.5ns = 80 = 0x50
	I2C->I2C_CCR |= 0x50;
	// 0000 0101 0000
	
	// In Smode, the maximum allowed SCL rise time is 1000ns
	// T max rise = 1000ns , F = 16MHz --> T = 62.5ns
	// TRISE = 1000/62.5 + 1 = 17 --> 0x11 ( 0001 0001)
	I2C->I2C_TRISE |= 0x11;
	
	//disable SMBUS mode bit to use I2C mode
	I2C->I2C_CR1 &= ~((unsigned int)BIT_1);
	
	//enable I2C peripherals - PE
	I2C->I2C_CR1 |= BIT_0;
}

static void triggerStartI2C(void){
	I2CRegister* I2C;
	I2C = (I2CRegister*)I2C_1;
	//Bit 8 in CR1 - START bit
	I2C->I2C_CR1 |= BIT_8;
	//By default I2C interface operates in Slave mode --> after enabling the start bit, need to wait until the device switches to master mode
	// check bit 0 - MSL in status reg2
	while(!(I2C->I2C_SR2 & BIT_0)){}
	//then start bit in SR1 need to be cleared, clear by reading status reg 1
	uint32_t readReg = I2C->I2C_SR1;
}

static void sendSlaveAddress(uint8_t address){
	I2CRegister* I2C;
	I2C = (I2CRegister*)I2C_1;
	I2C->I2C_DR = address;
	//after the address is being sent, the address bit gets set up which is ADDR inside SR1
	while(!(I2C->I2C_SR1 & BIT_1)){} // = 1 after receive ACK
	//ADDR bit is cleared by reading SR1 and then SR2
	uint32_t readReg = I2C->I2C_SR1;
	readReg = I2C->I2C_SR2;
}

static void sendData(uint8_t word){
	I2CRegister* I2C;
	I2C = (I2CRegister*)I2C_1;
	
	I2C->I2C_DR = word;
	//wait until the TXE - transmit buffer empty bit gets set up or the data was moved to the shift reg
	while(!(I2C->I2C_SR1 & BIT_7)){} // 0 - data reg not empty, 1 - data reg empty
}

static void triggerStopI2C(void){
	I2CRegister* I2C;
	I2C = (I2CRegister*)I2C_1;
	//stop condition should be programmed either when TXE or BTF is set
	//BTF is at bit 2 of status reg 1, 0 - data byte transfer not done, 1 - data byte transfer succeeded
	while(!(I2C->I2C_SR1 & BIT_2)){}
	//Stop bit is bit 9 inside CR1, 0 - no stop, 1 - stop
	I2C->I2C_CR1 |= BIT_9;
	
}	

int main(){
	initGPIOI2C();
	//I2C1(0x4000 5400) - APB1 bus  
	// STM32 can communicate with 3 different buses via I2C
	// In the APB bus, the bits for enabling I2C1, I2C2, I2C3 are bit 21, 22, 23
	//The block diagram is given on page 842 of the reference manual
	
	
	//use I2C as master
	initI2CRegsTransmit();
	
	int volatile transmit = 0, i = 0;
	uint8_t word = 0xB0;
	
	while (1){
		if (transmit == 1){
			triggerStartI2C();
			sendSlaveAddress(0xAA);
			while (i<10){
				sendData(word);
				word++;
				i++;
			}
			triggerStopI2C();
			i = 0;
			transmit = 0;
		}
	}
	
	
	return 0;
}