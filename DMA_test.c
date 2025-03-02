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

//create a pwm output signal with some period and duty cycle and measure the length of the period
static void InitPWMPB6() {
	GPIOGeneralRegister* GPIOB;
	GeneralPurposeTimer* timer4;
	GPIOB = (GPIOGeneralRegister*) GPIOB_BASE_ADDRESS;
	timer4 = (GeneralPurposeTimer*) TIM4_BASE;
	
	GPIOB->MODER |= BIT_13;
	GPIOB->OSPEEDR |= BIT_13;
	GPIOB->AFRL |= BIT_25;
	
	timer4->PSC = 3999;
	timer4->ARR = 4000;
	timer4->CCR1 = 2000;
	
	timer4->CCMR1 |= (BIT_5 | BIT_6);
	timer4->CCER &= ~((unsigned int)BIT_1);
	timer4->CCMR1 &= ~((unsigned int)BIT_0);
	timer4->EGR |= BIT_0;
	timer4->CCER |= BIT_0;
	timer4->CR1 |= BIT_0;
}
static void InitPWMPA0(){
	GPIOGeneralRegister* GPIOA;
	GPIOA = (GPIOGeneralRegister*)GPIOA_BASE_ADDRESS;
	
	GPIOA->MODER |= BIT_1;
	//GPIOA->OTYPER &= ~((unsigned int)BIT_0);
	GPIOA->OSPEEDR |= BIT_1;
	GPIOA->PUPDR |= BIT_1;
	GPIOA->AFRL |= BIT_0;
	
	
	GeneralPurposeTimer* timer2;
	timer2 = (GeneralPurposeTimer*)TIM2_BASE;
	
	timer2->CCMR1 &= ~((unsigned int)(BIT_2 | BIT_3));
	timer2->CCMR1 |= BIT_5;
	timer2->CCMR1 |= BIT_0; //IC1 (CCR1) mapped on TI1

	timer2->CCER &= ~((unsigned int)(BIT_1 | BIT_3)); // selecting rising edge
	

	timer2->CCMR1 &= ~((unsigned int)(BIT_2 | BIT_3)); // disable prescaler
	timer2->SMCR |= BIT_2; //reset mode
	timer2->SMCR |= (BIT_6 | BIT_4); //trigger is TI1FP1
	
	timer2->CCER |= BIT_0 ; //enable capture compare

	timer2->CR1 |= BIT_0; //enable counter
}

static volatile uint32_t readDMA[20];

void DMA1_Stream5_IRQHandler(void){
	REG32(DMA1_BASE_ADDRESS + 0x0C) |= (BIT_10 | BIT_11);
}

int main(){
	ResetandClockControl* RCC;
	RCC = (ResetandClockControl*) RCC_BASE_ADDRESS;
	
	RCC->APB1ENR |= BIT_2;
	RCC->AHB1ENR |= BIT_1;
		
	RCC->APB1ENR |= BIT_0;
	RCC->AHB1ENR |= BIT_0;
	
	InitPWMPB6();
	InitPWMPA0();
	
	RCC->AHB1ENR |= BIT_21;
	DirectMemoryAccess* DMA1;
	DMA1 = (DirectMemoryAccess*) DMA1_BASE_ADDRESS;
	
	DMA1->S5CR &= ~((unsigned int) BIT_0);
	DMA1->S5PAR =((uint32_t)(TIM2_BASE + 0x34));
	DMA1->S5M0AR = (uint32_t)(&readDMA[0]);
	DMA1->S5NDTR = 0x8;
	DMA1->S5CR |= (BIT_25 | BIT_26);
	DMA1->S5CR &= ~((unsigned int)BIT_5);
	DMA1->S5CR |= BIT_16;
	DMA1->S5FCR &= ~((unsigned int)BIT_2);
	DMA1->S5CR &= ~((unsigned int)(BIT_6 | BIT_7));
	DMA1->S5CR |= BIT_10;
	DMA1->S5CR |= (BIT_12 | BIT_14);
	DMA1->S5CR |= (BIT_3 | BIT_4);
	DMA1->S5CR |= BIT_0;
	
	REG32(TIM2_BASE + 0x0C) |= BIT_9;
	
	_NVIC_Enable_(_DMA1_Stream5_IRQn);
	
	while(1){
		
	}
	
	return 0;
} 

