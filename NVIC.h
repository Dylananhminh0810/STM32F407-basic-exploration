#ifndef NVIC_H
#define NVIC_H

#include <stdint.h>
#include "Register.h"
#define NVIC_ENABLE_BASE 0xE000E100
#define NVIC_PRIORITY_BASE 0xE000E400
#define NVIC_ISER0 (NVIC_ENABLE_BASE)
#define NVIC_ISER1 (NVIC_ENABLE_BASE + 0x04)
#define NVIC_ISER2 (NVIC_ISER1 + 0x04)

typedef enum
{
	_WWDG_IRQn = 0,
	_PVD_IRQn = 1,
	_TAMP_STAMP_IRQn = 2,
	_RTC_WKUP_IRQn = 3,
	_FLASH_IRQn = 4,
	_RCC_IRQn = 5,
	_EXTI0_IRQn = 6,
	_EXTI1_IRQn = 7,
	_EXTI2_IRQn = 8,
	_EXTI3_IRQn = 9,
	_EXTI4_IRQn = 10,
	_DMA1_Stream0_IRQn = 11,
	_DMA1_Stream1_IRQn = 12,
	_DMA1_Stream2_IRQn = 13,
	_DMA1_Stream3_IRQn = 14,
	_DMA1_Stream4_IRQn = 15,
	_DMA1_Stream5_IRQn = 16,
	_DMA1_Stream6_IRQn = 17,
	_ADC_IRQn = 18,
	_CAN1_TX_IRQn = 19,
	_CAN1_RX0_IRQn = 20,
	_CAN1_RX1_IRQn = 21,
	_CAN1_SCE_IRQn = 22,
	_EXTI9_5_IRQn = 23,
	_TIM1_BRK_TIM9_IRQn = 24,
	_TIM1_UP_TIM10_IRQn = 25,
	_TIM1_TRG_COM_TIM11_IRQn = 26,
	_TIM1_CC_IRQn = 27,
	_TIM2_IRQn = 28,
	_TIM3_IRQn = 29,
	_TIM4_IRQn = 30,
	_I2C1_EV_IRQn = 31,
	_I2C1_ER_IRQn = 32,
	_I2C2_EV_IRQn = 33,
	_I2C2_ER_IRQn = 34,
	_SPI1_IRQn = 35,
	_SPI2_IRQn = 36,
	_USART1_IRQn = 37,
	_USART2_IRQn = 38,
	_USART3_IRQn = 39,
	_EXTI15_10_IRQn = 40,
	_RTC_ALARM_IRQn = 41,
	_OTG_FS_WKUP_IRQn = 42,
	_TIM8_BRK_TIM12_IRQn = 43,
	_TIM8_UP_TIM13_IRQn = 44,
	_TIM8_TRG_COM_TIM14_IRQn = 45,
	_TIM8_CC_IRQn = 46,
	_DMA1_Stream7_IRQn = 47,
	_FSMC_IRQn = 48,
	_SDIO_IRQn = 49,
	_TIM5_IRQn = 50,
	_SPI3_IRQn = 51,
	_UART4_IRQn = 52,
	_UART5_IRQn = 53,
	_TIM6_DAC_IRQn = 54,
	_TIM7_IRQn = 55,
	_DMA2_Stream0_IRQn = 56,
	_DMA2_Stream1_IRQn = 57,
	_DMA2_Stream2_IRQn = 58,
	_DMA2_Stream3_IRQn = 59,
	_DMA2_Stream4_IRQn = 60,
	_ETH_IRQn = 61,
	_ETH_WKUP_IRQn = 62,
	_CAN2_TX_IRQn = 63,
	_CAN2_RX0_IRQn = 64,
	_CAN2_RX1_IRQn = 65,
	_CAN2_SCE_IRQn = 66,
	_OTG_FS_IRQn = 67,
	_DMA2_Stream5_IRQn = 68,
	_DMA2_Stream6_IRQn = 69,
	_DMA2_Stream7_IRQn = 70,
	_USART6_IRQn = 71,
	_I2C3_EV_IRQn = 72,
	_I2C3_ER_IRQn = 73,
	_OTG_HS_EP1_OUT_IRQn = 74,
	_OTG_HS_EP1_IN_IRQn = 75,
	_OTG_HS_WKUP_IRQn = 76,
	_OTG_HS_IRQn = 77,
	_DCMI_IRQn = 78,
	_CRYP_IRQn = 79,
	_HASH_RNG_IRQn = 80,
	_FPU_IRQn = 81,
}IRQ_t;
//All stm32 interrupts will be contained in the registers ISER0, ISER1, ISER2 -> 1 bit in ISER = 1 interrupt enable
	void _NVIC_Enable_(IRQ_t IRQn){
		uint8_t bit = IRQn % 0x20;
		switch((uint32_t)IRQn/0x20)
		{
			case 0:
			{
				REG32(NVIC_ISER0) |= (1 << bit);
				break;
			}
			case 1:
			{
				REG32(NVIC_ISER1) |= (1 << bit);
				break;
			}
			case 2:
			{
				REG32(NVIC_ISER2) |= (1 << bit);
				break;
			}
		}
	}

	void _NVIC_SetPriority_(IRQ_t IRQn, uint32_t priority){
		uint8_t byte = (uint32_t)IRQn; // 1 byte is 1 interrupt priority
	
		uint8_t* ptrToNVICPriority;
		ptrToNVICPriority = (uint8_t*)NVIC_PRIORITY_BASE;
	
		ptrToNVICPriority[byte] = priority;
	}

#endif