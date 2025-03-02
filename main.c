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
#include "ADC.h"
#include "DAC.h"

void initGPIOForDAC(void){
	//Datasheet from page 48, find addtional functions DAC
	//DAC_OUT1 at pin PA4
	GPIOGeneralRegister* DACPin;
	DACPin = (GPIOGeneralRegister*) GPIOA_BASE_ADDRESS;
	REG32(RCC_BASE_ADDRESS + RCC_AHB1_OFFSET) |= BIT_0;
	
	DACPin->MODER |= (BIT_8 | BIT_9);
	DACPin->OTYPER &= ~((unsigned int)BIT_4);
	DACPin->OSPEEDR &= ~((unsigned int)(BIT_8 | BIT_9));
}

void initDAC(void){
	//enable DAC EN inside RCC_APB1ENR bit 29
	DACStruct* DAC1;
	DAC1 = (DACStruct*)DAC;
	REG32(RCC_BASE_ADDRESS + RCC_APB1_OFFSET) |= BIT_29;
	
	//enable the trigger for channel 1 TEN1 inside DAC_CR
	DAC1->DAC_CR &= ~((unsigned int)BIT_2);
	
	//decide what will be the trigger
	//TSEL bits - DAC channel 1 trigger selection
	//111 use software trigger
	DAC1->DAC_CR |= (BIT_3 | BIT_4 | BIT_5);
	
	//enable the DAC - enabling bit 0
	DAC1->DAC_CR |= BIT_0;
}

void initTimerForDAC(void){
	//increment the voltage value of the DAC every milisecond until it reaches maximum value
	//after that, decrement it until it reaches the minimum value and then again increment it
	GeneralPurposeTimer* DACTimer;
	DACTimer = (GeneralPurposeTimer*)TIM3_BASE;
	REG32(RCC_BASE_ADDRESS + RCC_APB1_OFFSET) |= BIT_1;
	//Ftimer = 16MHz / (PSC + 1) = 4000MHz
	DACTimer->PSC = 3;
	//Finterrupt = Ftimer / ARR + 1 
	// 1/F interrupt = T interrupt. I need 1ms --> F = 1000
	// 4000M/1000 = 4000 --> ARR = 3999
	DACTimer->ARR = 3999;
	
	DACTimer->DIER |= BIT_0;
	DACTimer->CR1 &= ~((unsigned int)BIT_1);
	DACTimer->CR1 |= BIT_0;
	
	_NVIC_Enable_(_TIM3_IRQn);
}

void TIM3_IRQHandler(void){
	//counter values to increment or decrement
	//delta for amount of voltage increment
	static int i = 0, delta = 1;
	
	if (i<=0){
		delta = 1;
	} else if ( i >= 4095){
		delta = -1;
	}
	
	DACStruct* DAC1;
	DAC1 = (DACStruct*)DAC;
	//use 12 bit right aligned data holding reg DHR12R1
	DAC1->DAC_DHR12R1 = i;
	//trigger to start the conversion inside software trigger reg
	DAC1->DAC_SWTRIGR |= BIT_0;
	//this value will increment each time the handler gets called
	i+= delta;
	
	//remove the flag from the status reg so can exit the interrupt
	REG32(TIM3_BASE + 0x10) &= ~((unsigned int)BIT_0);
}

int main(){
	initGPIOForDAC();
	initDAC();
	initTimerForDAC();
	
	
	while (1){
		
	}
	
	return 0;
}