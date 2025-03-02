#include <stdint.h>
#include <stdbool.h>
#include "Register.h"
#include "RCC.h"
#include "GPIO.h"
#include "NVIC.h"
#include "EXTI.h"
#include "SYSCFG.h"
#include "Timer.h"

void TIM2_IRQHandler(void){
	static volatile uint32_t period = 0, duty = 0;
	static volatile double period_ms = 0, duty_ms = 0;
	REG32(TIM2_BASE + 0x10) &= ~((unsigned int)(BIT_9 | BIT_1));
	
	period = REG32(TIM2_BASE + 0x34); //read CCR1
	duty = REG32(TIM2_BASE + 0x38); //read CCR2
	
	period_ms = (double)period * 1000 / 16000000;
	duty_ms = (double)duty * 1000 / 16000000;
}


int main(){
	//PWM output on PB6
	GeneralPurposeTimer* timer4;
	timer4 = (GeneralPurposeTimer*)TIM4_BASE;
	REG32(RCC_BASE_ADDRESS + RCC_APB1_OFFSET) |= BIT_2; // enable TIM4 clock
	
	GPIOGeneralRegister* GPIOB;
	GPIOB  = (GPIOGeneralRegister*)GPIOB_BASE_ADDRESS;
	REG32(RCC_BASE_ADDRESS + RCC_AHB1_OFFSET) |= BIT_1; // GPIOB
	GPIOB->MODER |= BIT_13; //alternative function
	GPIOB->OSPEEDR |= BIT_13; //high speed
	GPIOB->AFRL |= BIT_25; //alternative function 2 for pin PB6
	
	timer4->PSC = 3999;
	timer4->ARR = 8000;
	timer4->CCR1 = 4000; // duty cycle 50%
	//CCMR have 2 mode PWM1 PWM2. PWM1 while counter less than CCR value the output is active and inactive for counter more than CCR value. PWM2 is the opposite.
	timer4->CCMR1 |= (BIT_5 | BIT_6); // select PWM1 mode
	timer4->CCER &= ~((unsigned)BIT_1); //active high 
	//if choose up counting the signal will be high until it reaches CCR value and later it will be low
	// else choose down counting the signal will be first low and then become high onces it reach CCR value
	timer4->CCMR1 &= ~((unsigned int)BIT_0); //CC1S bit to choose output channel 1
	
	//These bits are important for setting values to the ARR and CCR regs
	//when these preload bits are enabled and we want to set new values into the ARR and CCR regs
	//the new values will not be used right away, but after the update event
	//if this bit is not set the value will be used right away
	//-> set them both, meaning new values will be updated after each update event
	timer4->CR1 |= BIT_7; //and auto reload-preload enable for ARR
	timer4->CCMR1 |= BIT_3; //Enabling preload reg for CCR1
	
	//create update event to reload the reg
	timer4->EGR |= BIT_0; //update regs
	
	timer4->CCER |= BIT_0; // OC1 signal is output on the corresponding output pin
	timer4->CR1 |= BIT_0; //enable counter for timer
	
	
	//once the main signal is triggered it will trigger the counter to start counting from zero
	//once the falling edge is detected, the counter value will be captured by one of the 2 CCR reg
	//but we won't enable an interrupt to be triggered
	//the counter will continue counting until a rising edge is detected
	//then the value will be captured by the second CCR reg	
	//after which the counter will be set to 0
	//And because we will enable an interrupt to be triggered, the handler will be called and we will read
	//the values stored inside the both regs
	
	//pinPA0
	GPIOGeneralRegister* GPIOA;
	GPIOA = (GPIOGeneralRegister*)GPIOA_BASE_ADDRESS;
	REG32(RCC_BASE_ADDRESS + RCC_AHB1_OFFSET) |= BIT_0;
	GPIOA->MODER |= BIT_1;
	GPIOA->OTYPER &= ~((unsigned int)BIT_0);
	GPIOA->OSPEEDR |= BIT_1;
	GPIOA->PUPDR |= BIT_1;
	GPIOA->AFRL |= BIT_0;
	
	GeneralPurposeTimer* timer2;;
	timer2 = (GeneralPurposeTimer*)TIM2_BASE;
	
	REG32(RCC_BASE_ADDRESS + RCC_APB1_OFFSET) |= BIT_0;
	
	timer2->CR2 &= ~((unsigned int)BIT_7);
	timer2->CCMR1 |= BIT_5;
	timer2->CCMR1 |= BIT_0; //IC1 (CCR1) mapped on TI1
	timer2->CCMR1 |= BIT_9; //IC2 (CCR2) mapped on TI2
	timer2->CCER &= ~((unsigned int)(BIT_1 | BIT_3)); // selecting rising edge
	timer2->CCER |= BIT_5; // selecting falling edge
	timer2->CCMR1 &= ~((unsigned int)(BIT_2 | BIT_3)); // disable prescaler
	timer2->SMCR |= BIT_2; //reset mode
	timer2->SMCR |= (BIT_6 | BIT_4); //trigger is TI1FP1
	timer2->CCER |= (BIT_0 | BIT_4); //enable capture compare
	timer2->DIER |= BIT_1; // capture compare interrupt enable
	timer2->CR1 |= BIT_0; //enable counter
	
	_NVIC_Enable_(_TIM2_IRQn);
	
	
	while(1){
		
	}
	
	return 0;
}