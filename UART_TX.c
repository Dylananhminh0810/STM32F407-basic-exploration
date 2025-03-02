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
//the data reg is 9 bits inside
static volatile uint8_t dataTX[12] = {0x48, 0x65, 0x6C, 0x6C, 0x6F, 0x20, 0x77, 0x6F, 0x72, 0x6C, 0x64, 0x21};
//So we need to send byte by byte each element of the array

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

static void Uart4Send(void){
	UART4 = (Uart*) UART_4;

	//Enable UART - bit 13 
	UART4->CR1 |= BIT_13;
	//Create a data array with secret values that we will try to decode
	// So for that we will create a loop and we will iterate through the array
	static volatile int i;
	
	for (i=0; i<12; i++){
		UART4->DR = dataTX[i];
		//First let use the TXE flag
		//This flag is set when the transmit data reg is empty
		//So when data was successfully moved to the shift reg
		while (!(UART4->SR & BIT_7)){
		//we have used negation here because we want to wait as long as the data reg is not empty
		// once it becomes empty, we can move to the next line
		}
		// here we will check the complete flag
		// it gets set up when the transmission is done
		// this mean that the data bits are sent from the shift reg to output pin, including the stop bit
		while (!(UART4->SR & BIT_6)){
		// use negation as condition of the while loop to wait until the transmission finishes
		}
	}
	//In the end we can disable the uart and clear TC flag
	UART4->CR1 &= ~((unsigned int)BIT_13);
	UART4->SR &= ~((unsigned int)BIT_6);
	
	//the speed of data transmission is 9600 bits per second, one bit of data is sent in a time period of 1/9600
	//And the time period to go over this loop is shorter
	//This means that after the first iteration, data that is not sent will be overwritten and corrupted in the data reg
	// in the very next iteration
	// what we want is to write the data to the data reg
	//wait until the data is sent and then write new data to the empty data reg
	//-> we need to have some kind of a way to know when data is sent and the data reg is empty
	// we will not set up [send break character reg] and parity selection we will not consider -> there will be no parity bit
	// the data reg consists of the transmit data reg and a shift reg
	// USART_BRR, CR1, Transmit Control -> Transmit shift reg
	// First it gets written to the transmit data reg, after which it is copied to the shift reg
	// By using transmit control, it gets sent to the output pin TX
	// Not only does the transmit control provide the functionality of sending data, but in regards of the steps of the transfer process
	// it can also set up flags inside the [status reg]
	// Because we are considering basic transmission, we will consider the TC and TXE flags
	// The transmit data reg empty
	// TXE flag gets set up automatically once the data from the datareg gets transferred to the shift reg
	// Later from the shift reg, it gets transferred to the pins - least significant bit first
	// By enabling the TXEIE bit of the CR1 each time the TXE flag gets set up, we can trigger an interrupt
	
	//The TXE gets cleared when the transmit data reg is not empty, that is by writing to it
	//This flag is useful if we want to make sure the data, after writing to the transmit data reg was transferred to the shift reg
	// The transmission complete bit gets set up once the TXE bit is set up meaning data was copied to the shift reg
	// And when the transmission of a frame containing data is complete
	// Also here we can enable the TCIE bit so that each time this bit gets set up we can trigger an interrupt
	
	// After this bit is set up and the last byte is send UART should be disabled
	//The TCIE is cleared by reading the status reg or just disbling it manually in the status reg [SR]
}

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
	//*UART & USART has the same information of set up in the datasheet
	enableUart4GpioTx();
	//Because setting up uart and sending data are 2 different functionalities so we create functions and call it them the main
	enableUart4();
	Uart4Send();
	
	while(1){
		
	}
	
	return 0;
} 

