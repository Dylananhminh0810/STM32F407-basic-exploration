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

uint32_t dataBytes[8] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

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

static void initI2CRegsReceive(void){
	// I2C as slave mode
	I2CRegister* I2C;
	I2C = (I2CRegister*)I2C_1;
	REG32(RCC_BASE_ADDRESS + RCC_APB1_OFFSET) |= BIT_21;
	
	//enable I2C peripheral
	I2C->I2C_CR1 |= BIT_0;
	
	//set up an address for the slave device in own address reg I2C_OAR1
	//bit 15 to set 7 or 10 bit address
	I2C->I2C_OAR1 &= ~((unsigned int)BIT_15);
	I2C->I2C_OAR1 |= (0x55 << 1); //0xAA (bit 0 is used for 10 bit add mode, but I use 7 bit add mode)
	
	//ACK enable bit can find in CR1, bit 10, 0 - no ack returned, 1 - ack returned after a byte is received
	//This bit is set and cleared by software and cleared by hardware when PE = 0
	I2C->I2C_CR1 |= BIT_10;
	
	//use interrupt to write the received data into a buffer
	//need to enable interrupts inside the CR2
	//By enabling bit 9, we will enable interrupts for all events - page 863 reference manual
	//TXE and RXNE are triggered when ITBUFEN = 1 which is bit 10 in CR2
	I2C->I2C_CR2 |= (BIT_9 | BIT_10);
}
/*
static int i = 0;
void I2C1_EV_IRQHandler(void){
	I2CRegister* I2C;
	I2C = (I2CRegister*)I2C_1;
	
	if ((I2C->I2C_SR1 & BIT_1)){ // check the address bit if it was raised, or the address was received and matched, clear by reading SR1 then SR2
		uint32_t readReg = I2C->I2C_SR1;
		readReg = I2C->I2C_SR2;
	} else if (I2C->I2C_SR1 & BIT_6){ // check data reg is not empty then read, RXNE
		dataBytes[i] = I2C->I2C_DR;
		i++;
	} else if (I2C->I2C_SR1 & BIT_4){
		//STOPF, 1 - stop detected, 0 - no stop detected, cleared by reading the SR1 then write in CR1
		//CR1 - BIT9 - Stop generation bit 
		uint32_t readReg = I2C->I2C_SR1;
		I2C->I2C_CR1 |= BIT_0;
		i = 0;
	}
}

static void enableUart4GpioTx(void){
//set up the pin for transmitting
	GPIOGeneralRegister* GPIOA;
	GPIOA = (GPIOGeneralRegister*)GPIOA_BASE_ADDRESS;
	REG32(RCC_BASE_ADDRESS + RCC_AHB1_OFFSET) |= BIT_0;
	GPIOA->MODER |= BIT_1;
	GPIOA->OTYPER &= ~((unsigned int)BIT_0);
	GPIOA->OSPEEDR |= BIT_1;
	GPIOA->PUPDR |= BIT_0;
	GPIOA->AFRL |= BIT_3;
	
}

static Uart* UART4;

static void enableUart4(){

	UART4 = (Uart*) UART_4;

	REG32(RCC_BASE_ADDRESS + RCC_APB1_OFFSET) |= BIT_19;
	//use baudrate 9600 bits/s
	//First start bit, its time length is 1/9600
	//This bit and every other bit are divided into sampling points
	//There can be 8 or 16 sampling points
	//How many clock counts are going to be inside one baudrate cycle
	UART4->CR1 &= ~((unsigned int)BIT_15); // set bit OVER8 - oversampling mode to 0 -> oversampling by 16
	//caculate the prescaler by using the formula on page 978
	// frequency fck = 16MHz and the sampling is 16
	// prescaler needs to be entered as hexadecimal representation inside the baudrate reg - BRR
	// from fomula 16Mhz / [8*(2-OVER8) * USARTDIV] = 9600
	// OVER8 = bit 0
	// -> USARTDIV = 104.16666
	// 104 = Mantissa in baudrate reg = 0x68 (hexadecimal)
	// 0x1666 = Fraction in baudrate reg * 16(sampling) = 2.666~3 = 0x03 (hexadecimal)	(round it up the closest number)
	//page 979 has some example for caculation
	UART4->BRR = ((0x68 << 4) | 0x03);
	// data frame set up
	// we need 8 bits or 1 byte
	// bit 12 CR1
	// Can choose 8 or 9 bits.
	UART4->CR1 &= ~((unsigned int)BIT_12);
	//We will have no parity bit so disable it by setting 0 to bit 10 
	UART4->CR1 &= ~((unsigned int)BIT_10);
	//We will have 1 stop bit at the end - choose in control reg 2 bit 12 and 13
	//00 - 1 stop bit
	//Because we are using uart, there is no need to focus on any clocks
	//So we will disregard the other bits
	UART4->CR2 &= ~((unsigned int)(BIT_12 | BIT_13));
	//Set device as a transmitter
	// enabling the transmit enable bit inside the control reg 1 - bit 3
	//There's 1 bit delay before the data transmission start
	UART4->CR1 |= BIT_3;
	
}

int main(){
	initGPIOI2C();
	//I2C1(0x4000 5400) - APB1 bus  
	// STM32 can communicate with 3 different buses via I2C
	// In the APB bus, the bits for enabling I2C1, I2C2, I2C3 are bit 21, 22, 23
	//The block diagram is given on page 842 of the reference manual
	
	initI2CRegsReceive();
	_NVIC_Enable_(_I2C1_EV_IRQn);
	
	enableUart4GpioTx();
	enableUart4();
	
	Uart* UART4;
	UART4 = (Uart*)UART_4;
	
	while(1)
	{
		if (i != 0){
			int j = 0;
			while (j<i){
				UART4->DR = dataBytes[j];
				while(!(UART4->SR & BIT_7)){}
				while(!(UART4->SR & BIT_6)){}
				dataBytes[j] = 0;
				j++;
			}
			i = 0;
		}
	}
	*/
	
int main(){
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