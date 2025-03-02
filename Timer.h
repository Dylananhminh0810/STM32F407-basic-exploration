#ifndef TIMER_H
#define TIMER_H
#include <stdint.h>
#define TIM3_BASE 0x40000400
#define TIM2_BASE 0x40000000
#define TIM4_BASE 0x40000800
#define TIM5_BASE 0x40000C00
typedef struct{
	uint32_t CR1;//0x00
	uint32_t CR2;//0x04
	uint32_t SMCR;//0x08
	uint32_t DIER;//0x0C
	uint32_t SR;//0x10
	uint32_t EGR;//0x14
	uint32_t CCMR1;//0x18
	uint32_t CCMR2;//0x1C
	uint32_t CCER;//0x20
	uint32_t CNT;//0x24
	uint32_t PSC;//0x28
	uint32_t ARR;//0x2C
	uint32_t reserved1;//0x30
	uint32_t CCR1;//0x34
	uint32_t CCR2;//0x38
	uint32_t CCR3;//0x3C
	uint32_t CCR4;//0x40
	uint32_t reserved2;//0x44
	uint32_t DCR;//0x48
	uint32_t DMAR;//0x4C
	uint32_t OR;//0x50
} GeneralPurposeTimer;//the reg offset is 4 bytes so simply create 32 bit members of the object, we need to
//be careful when setting up bits because not all 32 bits are used

//void TIM3_IRQHandler(void){
//	static volatile uint8_t i = 0;
	
//	REG32(TIM3_BASE + 0x10) &= ~((unsigned int)BIT_0);//BIT UIF in PIN TIMx SR to update after interrupt
	
//	if (i%2){
//		REG32(GPIOD_BASE_ADDRESS + GPIOD_BSRR_OFFSET) |= BIT_28; // LED low
//	}
//	else 
//	{
//		REG32(GPIOD_BASE_ADDRESS + GPIOD_BSRR_OFFSET) |= BIT_12; // LED high
//	}
//	i++;
//}



#endif