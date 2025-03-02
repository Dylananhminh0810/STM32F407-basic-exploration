#include <stdint.h>
#include <stdbool.h>
#include "Register.h"
#include "RCC.h"
#include "GPIO.h"
#include "NVIC.h"
#include "EXTI.h"
#include "SYSCFG.h"
#include "Timer.h"

//The ultrasonic sensor HC SR04
//Connect the VCC and GND
//Trigger pin will connect to PB6. 
//For trigger we will create a PWM signal with load duty cycle and an output on pin PB6
//The echo pin connect to pin PA0. This will be an input capture.
//It will measure the time between the changing of values for the echo pin.
//We will create a pulse that lasts 10 microseconds, which will trigger a burst of 40KHz
//That burst can pass a maximum range of four meters and then it will comeback
//So the distance it passes in that case will be eight meters with a speed of sound of roughly 340m/s
//So it takes roughly 23.53 miliseconds in worst case scenario for the burst to return to the sensor
//During this time until the sound comes back, there mustn't be another pulse created
//Because of that, we'll pick a time interval of 60 miliseconds when each pulse will be generated
//60 miliseconds is a long time compared to 10 microseconds
//meaning the duty cycle of the PWM will very small compared to the period
//To achieve that we will have a big value entered inside the ARR register compared to the Prescaler
//The frequency that we need in the output is 1/60 ms, which is 16.67Hz
//Output frequency = 16.67 = (sysclk)16000000/((prescaler+1) * (ARR+1))
// CCR/ 48000 = duty cycle (thoi gian on)
//choose abitrary value (gia tri tuy y) for prescaler and ARR that fulfill this requirement
//prescaler = 19, ARR = 48000, CCR = 160 
// 16MHZ / (20 x 48001) = 16.67
// 160/48000 = 0.33 -> xung PWM rat ngan so voi toan bo chu ky

void TIM2_IRQHandler(void) //input capture mode for ultrasonic
{
	static volatile double distance[5];
	static volatile uint32_t time = 0, i =0; // in meters
	
	REG32(TIM2_BASE + 0x10) &= ~((unsigned int)0x1);
	
	time = REG32(TIM2_BASE + 0x34);
	
	if (!(REG32(GPIOA_BASE_ADDRESS + 0x10) & 0x1)){
		distance[i%5] = time * 0.5 * 340/16000000;
		i++;
	}
	REG32(TIM2_BASE + 0x24) = 0;
}


void EnablePWMForUltrasonicSensor(void){
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
	
	timer4->PSC = 19;
	timer4->ARR = 48000;
	timer4->CCR1 = 160; 
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
}

void EnableReadingUltrasonicSensor(void){
	//pinPA0 read the echo
	GPIOGeneralRegister* GPIOA;
	GPIOA = (GPIOGeneralRegister*)GPIOA_BASE_ADDRESS;
	REG32(RCC_BASE_ADDRESS + RCC_AHB1_OFFSET) |= BIT_0;
	GPIOA->MODER |= BIT_1;
	GPIOA->OTYPER &= ~((unsigned int)BIT_0);
	GPIOA->OSPEEDR |= BIT_1;
	GPIOA->AFRL |= BIT_0;
	GPIOA->PUPDR |= BIT_1;
	
	GeneralPurposeTimer* timer2;;
	timer2 = (GeneralPurposeTimer*)TIM2_BASE;
	
	REG32(RCC_BASE_ADDRESS + RCC_APB1_OFFSET) |= BIT_0;
	
	timer2->CR2 &= ~((unsigned int)BIT_7);
	timer2->CCMR1 |= BIT_5;
	timer2->CCER |= (BIT_1 | BIT_3); 
	timer2->CCMR1 |= BIT_0; 
	timer2->CCMR1 &= ~((unsigned int)(BIT_2 | BIT_3));
	timer2->CCER |= BIT_0;
	timer2->DIER |= BIT_1; // capture compare interrupt enable
	timer2->CR1 |= BIT_0; //enable counter
	
	_NVIC_Enable_(_TIM2_IRQn);
}

int main(){
	EnablePWMForUltrasonicSensor();
	EnableReadingUltrasonicSensor();
	
	
	while(1){
		
	}
	
	return 0;
}