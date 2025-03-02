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

static volatile uint8_t dataRX[12] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
//0x48, 0x65, 0x6C, 0x6C, 0x6F, 0x20, 0x77, 0x6F, 0x72, 0x6C, 0x64, 0x21
//set up for receiving RX 
//UART4_RX -> PA1, AF8
static void enableUart4GpioRX(void){
	GPIOGeneralRegister* GPIOA;
	GPIOA = (GPIOGeneralRegister*) GPIOA_BASE_ADDRESS;
	REG32(RCC_BASE_ADDRESS + RCC_AHB1_OFFSET) |= BIT_0;
	
	GPIOA->MODER |= BIT_3;
	GPIOA->OTYPER &= ~((unsigned int)BIT_1);
	GPIOA->OSPEEDR |= BIT_3;
	GPIOA->PUPDR |= BIT_2;
	GPIOA->AFRL |= BIT_7;
}

static void enableUart4(void){
	Uart* UART4;
	UART4 = (Uart*)UART_4;
	REG32(RCC_BASE_ADDRESS + RCC_APB1_OFFSET) |= BIT_19;
	
	//set the oversampling same to TX 
	UART4->CR1 &= ~((unsigned int)BIT_15);
	UART4->BRR = ((0x68 << 4) | 0x03);
	UART4->CR1 &= ~((unsigned int)BIT_12);
	UART4->CR1 &= ~((unsigned int)BIT_10);
	UART4->CR2 &= ~((unsigned int)(BIT_12 | BIT_13));
	
	UART4->CR1 |= BIT_2;
	
	UART4->CR1 |= BIT_13;
	
	//Enable interrupt for receive data
	// That is done by enabling the RXNEIE bit inside the CR1 bit 5 
	UART4->CR1 |= BIT_5;
	_NVIC_Enable_(_UART4_IRQn);
}

void UART4_IRQHandler(void){
	// the data gets received least significant bit first
	// The character gets stored inside the shift reg, after which it gets copied to the received data reg (RDR)
	// Once it's copied, the RXNE flag gets set up which means data is ready for reading
	// And if the RXNE IE reg is set, setting up the RXNE flag will trigger an interrupt
	// The RXNE flag needs to be cleared before receiving the next character, or else over run error (ORE) will be triggered
	// We can clear the RXNE flag either by reading the data reg or by writing 0 to RXNE bit
	Uart* UART4;
	UART4 = (Uart*)UART_4;
	static int j = 0;
	while(!(UART4->SR & BIT_5)){} //RXNE flag status check if data ready, wait as long as the receive data reg is empty, once it receive data we will exit the loop
	dataRX[j] = (UART4->DR & 0xFF); // read the data
	UART4->SR &= ~((unsigned int)BIT_5);
	j++;
	j = j%12;
	//after reading the data from the data reg, the RXNE flag should be cleared, but just in case we will clear it manually
}

int main(){
	enableUart4GpioRX();
	enableUart4();

	while(1){
		
	}
	
	return 0;
} 

