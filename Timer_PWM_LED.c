#include <stdint.h>
#include <stdbool.h>
#include "Register.h"
#include "RCC.h"
#include "GPIO.h"
#include "NVIC.h"
#include "EXTI.h"
#include "SYSCFG.h"
#include "Timer.h"

#define TIM4_BASE 0x40000800

int main(){
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
	
	
	while(1){
		
	}
	
	return 0;
}