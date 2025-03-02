#include <stdint.h>
#include <stdbool.h>
#include "Register.h"
#include "RCC.h"
#include "GPIO.h"
#include "NVIC.h"
#include "EXTI.h"
#include "SYSCFG.h"
#include "Timer.h"

//capture the button push / leave to the CCR value

#define GPIOA_BASE_ADDRESS 0x40020000
#define TIM2_BASE 0x40000000

void TIM2_IRQHandler(void){
	static volatile uint32_t readCCR = 0;
	REG32(TIM2_BASE + 0x10) &= ~((unsigned int)(BIT_9 | BIT_1));
	readCCR = REG32(TIM2_BASE + 0x34);
	REG32(TIM2_BASE + 0x24) = 0; 
	//REG32(TIM2_BASE + 0x34) = 0; // CNT = 0 right at the time button is pushed. and when leave CCR = CNT and the CNT continue count 
}

int main(){
	GPIOGeneralRegister* GPIOA;
	GeneralPurposeTimer* timer2;
	
	GPIOA = (GPIOGeneralRegister*)GPIOA_BASE_ADDRESS;
	timer2 = (GeneralPurposeTimer*)TIM2_BASE;
	
	REG32(RCC_BASE_ADDRESS + RCC_AHB1_OFFSET) |= BIT_0;
	REG32(RCC_BASE_ADDRESS + RCC_APB1_OFFSET) |= BIT_0;
	
	GPIOA->MODER |= BIT_1;
	GPIOA->OTYPER &= ~((unsigned int)BIT_0); //output push pull
	GPIOA->OSPEEDR |= BIT_1;
	GPIOA->PUPDR |= BIT_1;
	GPIOA->AFRL |= BIT_0; //AF1 for PIN A0
	
	timer2->CR2 &= ~((unsigned int)BIT_7);
	timer2->CCMR1 |= BIT_5;
	timer2->CCER |= (BIT_1 | BIT_3);
	timer2->CCMR1 |= BIT_0;
	timer2->CCMR1 &= ~((unsigned int)(BIT_2 | BIT_3)); // set disable divider
	
	timer2->CCER |= BIT_0;
	timer2->DIER |= BIT_1;
	timer2->CR1 |= BIT_0;
	
	_NVIC_Enable_(_TIM2_IRQn);
	
	
	
	while(1){
		
	}
	
	return 0;
}