#include <stdint.h>
#include <stdbool.h>
#include "Register.h"
#include "RCC.h"
#include "GPIO.h"
#include "NVIC.h"
#include "EXTI.h"
#include "SYSCFG.h"
#include "Timer.h"
//distance = num_rotations * circumference
//num_rotation_t = number_rotations/time
//diameter = 18 milimeters

void TIM3_IRQHandler(void){
	static volatile double distance_meters = 0, speed_meters_second = 0;
	static uint32_t volatile counter_reading;
	REG32(TIM3_BASE + 0x10) &= ~((unsigned int)(BIT_0));
	counter_reading = REG32(TIM4_BASE + 0x24);
	REG32(TIM4_BASE + 0x24) = 0;
	speed_meters_second = 0.018*3.14*counter_reading/64;
	distance_meters += speed_meters_second;
}

int main(){
	GPIOGeneralRegister* GPIOB;
	GPIOB = (GPIOGeneralRegister*)GPIOB_BASE_ADDRESS;
	REG32(RCC_BASE_ADDRESS + RCC_AHB1_OFFSET) |= BIT_1;
	GPIOB->MODER |= (BIT_13 | BIT_15);
	GPIOB->OTYPER &= ~((unsigned int)(BIT_6 | BIT_7));
	GPIOB->OSPEEDR |= (BIT_13 | BIT_15);
	GPIOB->PUPDR |= (BIT_13 | BIT_15);
	GPIOB->AFRL |= (BIT_25 | BIT_29);
	
	GeneralPurposeTimer* timer4;
	timer4 = (GeneralPurposeTimer*) TIM4_BASE;
	REG32(RCC_BASE_ADDRESS + RCC_APB1_OFFSET) |= BIT_2;
	timer4->CCER &= ~((unsigned int)(BIT_1 | BIT_3));
	timer4->CCER &= ~((unsigned int)(BIT_7 | BIT_6));
	timer4->SMCR |= (BIT_0 | BIT_1);
	timer4->CCMR1 |= (BIT_0 | BIT_8);
	timer4->CR1 |= BIT_0;

	uint16_t cnt_values = REG32(TIM4_BASE + 0x14);
	double distance = 0.018*3.14*cnt_values/64;
	
	GeneralPurposeTimer* timer3;
	timer3 = (GeneralPurposeTimer*) TIM3_BASE;
	REG32(RCC_BASE_ADDRESS + RCC_APB1_OFFSET) |= BIT_1;
	timer3->PSC = 3999;
	timer3->ARR = 4000;
	timer3->CR1 &= ~((unsigned int)BIT_1);
	timer3->CR1 |= BIT_0;
	_NVIC_Enable_(_TIM3_IRQn);
	
	while(1){
		
	}
	
	return 0;
} 