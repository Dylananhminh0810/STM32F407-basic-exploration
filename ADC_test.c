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
static void initADCGPIO(void){
	//ADC information
	//Datasheet from page 47
	//use pin PA1
	GPIOGeneralRegister* GPIO_ADC;
	GPIO_ADC = (GPIOGeneralRegister*) GPIOA_BASE_ADDRESS;
	REG32(RCC_BASE_ADDRESS + RCC_AHB1_OFFSET) |= BIT_0;
		
	GPIO_ADC->MODER |= (BIT_2 | BIT_3);
	GPIO_ADC->OTYPER &= ~((unsigned int)BIT_1);
	GPIO_ADC->OSPEEDR &= ~((unsigned int)(BIT_2 | BIT_3));
}

static void initADCReg(void){
	
	//use ADC1
	//ADC common status reg ADC_CSR = ADC1_base_address + 0x300
	ADCStruct* ADC_1;
	ADCCommonStruct* ADC_COM;
	ADC_1 = (ADCStruct*)ADC1;
	ADC_COM = (ADCCommonStruct*) ADCCOM;
	
	//enable bit 8 of RCC_APB2ENR for ADC1 EN
	REG32(RCC_BASE_ADDRESS + RCC_APB2_OFFSET) |= BIT_8;
	
	//powering the ADC on at ADON bit in ADC_CR2 - 1 to enable, 0 to disable
	ADC_1->ADC_CR2 |= BIT_0;
	
	//set the clock speed inside the ADC CCR
	//provide a prescaler to devide the source clock only by 2 - 00
	ADC_1->ADC_CR2 |= BIT_0;
	ADC_COM->ADC_CCR &= ~((unsigned int)(BIT_16 | BIT_17));
	
	//in SQR1 we choose how many channel to convert
	//use 1 channel conversion
	ADC_1->ADC_SQR1 &= ~((unsigned int)(BIT_20 | BIT_21 | BIT_22 | BIT_23));
	
	//because only use 1 channel conversion --> use first sequence, the input from channel 1 - SQ1
	//Pin PA1 is connected to input channel 1 (in datasheet page 49)
	ADC_1->ADC_SQR3 |= BIT_0;
	
	//Set resolution in CR1 - 12 bits resolution
	ADC_1->ADC_CR1 &= ~((unsigned int)(BIT_24 | BIT_25));
	//By default, the conversion time is 15 ADC clock cycles, one clock cycle per bit and three sampling clock cycles
	//The sampling can be change in SMPR1 and SMPR2
	
	//Using channel 1 --> set up in sample time reg 2 (SMPR2)
	//choose 15 clock cycles for sampling
	ADC_1->ADC_SMPR2 |= BIT_3;
	//15 clock cycles for sampling + 12 bits cycles --> 27 ADC cycle clocks for ADC conversion
	
	//Set data alignment to the right side
	//Data alignment is how we want data to be aligned inside the reg
	//--> alignment to the right --> BIT ALIGN - value 0
	ADC_1->ADC_CR2 &= ~((unsigned int)BIT_11);
	
	//set up an interrupt
	ADC_1->ADC_CR1 |= BIT_5;
	//ADC will be triggered each time there's an external trigger
	//first need to set the discountinuous mode in ADC
	ADC_1->ADC_CR1 |= BIT_11;
	//external event select - choose timer 5 channel 1
	ADC_1->ADC_CR2 |= (BIT_25 | BIT_27);
	//choose trigger detection on both the rising and falling edges
	ADC_1->ADC_CR2 |= (BIT_28 | BIT_29);
	
	//start the conversion - SWSTART
	ADC_1->ADC_CR2 |= BIT_30;
	
	//enable the ADC interrupt inside the Nvic
	_NVIC_Enable_(_ADC_IRQn);
	
}

static void initADCTimer(void){
	GeneralPurposeTimer* timer5;
	timer5 = (GeneralPurposeTimer*)TIM5_BASE;
	REG32(RCC_BASE_ADDRESS + RCC_APB1_OFFSET) |= BIT_3;
	//F = 16MHz, fADC = 8MHz
	//Total conversion time = 27 cycles (15 sampling + 12 bits)
	//conversion period = 27*1/(8) = 3.375 us;
	
	//T prescaler = 3999
	//ARR = 8000
	timer5->PSC = 3999;
	timer5->ARR = 8000;
	timer5->CCR1 = 1000;
	timer5->CCMR1 |= (BIT_5 | BIT_4);
	//enable channel CC1 as output
	timer5->CCMR1 &= ~((unsigned int)BIT_0);
	//enable capture/compare mode
	timer5->CCER |= BIT_0;
	
	//enable counter enable bit inside CR1
	timer5->CR1 |= BIT_0;
}

static volatile uint32_t voltage;
void ADC_IRQHandler(void){
	//helping variable what will read the voltage and call it voltage
	voltage = REG32(ADC1 + 0x4C);
	

}


int main(){
	initADCGPIO();
	initADCTimer();
	initADCReg();
	
	while (1){
		
	}
	
	return 0;
}