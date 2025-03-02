#include <stdint.h>
#include <stdbool.h>
#include "Register.h"
#include "RCC.h"
#include "GPIO.h"
#include "NVIC.h"
#include "EXTI.h"
#include "SYSCFG.h"
#include "Timer.h"

#define PWR_CONTROLLER 0x40007000
#define FLASH_INTERFACE 0x40023C00


void TIM3_IRQHandler(void){
	static volatile int i = 0;
	REG32(TIM3_BASE + 0x10) &= ~((unsigned int)BIT_0);
	
	if (i%2){
		REG32(GPIOD_BASE_ADDRESS + 0x18) |= BIT_12;
	} else {
		REG32(GPIOD_BASE_ADDRESS + 0x18) |= BIT_28;
	}
	i++;
}


int main(){
	GeneralPurposeTimer* timer3;
	GPIOGeneralRegister* GPIOD;
	GPIOD = (GPIOGeneralRegister*)GPIOD_BASE_ADDRESS;
	timer3 = (GeneralPurposeTimer*)TIM3_BASE;
	ResetandClockControl* RCC;
	RCC = (ResetandClockControl*) RCC_BASE_ADDRESS;
	RCC->AHB1ENR |= BIT_3;
	RCC->APB1ENR |= BIT_1; 
	
	
	
	RCC->CR |= BIT_16; // enable HSE
	while (!(RCC->CR & BIT_17)){} //check if HSE is stable
		
	//RCC->CFGR &= ~((unsigned int)(BIT_0 | BIT_1));
	//RCC->CFGR |= BIT_0;
		
	//while (!(RCC->CFGR & BIT_2)) {} //check if HSE used as system clock source
	
	REG32(PWR_CONTROLLER) |= BIT_14;
	REG32(FLASH_INTERFACE) |= BIT_1;
		
	RCC->PLLCFGR &= ~((unsigned int)0x3F);	
	RCC->PLLCFGR |= 0x4; //M =4
	RCC->PLLCFGR &= ~((unsigned int)0x7FC0);
	RCC->PLLCFGR |= (0x40<<6); //N =64
	RCC->PLLCFGR &= ~((unsigned int)0x30000);
	RCC->PLLCFGR |= (0x0<<16); //P=2
	RCC->PLLCFGR |= BIT_22;// HSE will be the main clock to the PLL

	RCC->CR |= BIT_24;//enable PLL output
	while ((RCC->CR & BIT_25) == 0) {} //Wait until the main PLL is ready
		
	RCC->CFGR &= ((unsigned int)(BIT_1 | BIT_0)); //clear bit ange
	RCC->CFGR |= BIT_1; // select the main PLL as system clock source
	while (!(RCC->CFGR & BIT_3)) {} //Wait until the main PLL is used as system clock source
		
	GPIOD->MODER |= BIT_24;
	GPIOD->OSPEEDR |= BIT_24;
	GPIOD->PUPDR |= BIT_24;
	
	timer3->PSC = 4000;
	timer3->ARR = 4000;
	
	timer3->DIER |= BIT_0;
	timer3->CR1 &= ~((unsigned int)BIT_1);
	timer3->CR1 |= BIT_0;
	
	_NVIC_Enable_(_TIM3_IRQn);
	
	while(1){
		
	}
	
	return 0;
} 