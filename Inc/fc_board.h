/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __FC_BOARD_H
#define __FC_BOARD_H

#include "stm32f4xx_ll_rcc.h"
#include "stm32f4xx_ll_bus.h"
#include "stm32f4xx_ll_system.h"
#include "stm32f4xx_ll_exti.h"
#include "stm32f4xx_ll_cortex.h"
#include "stm32f4xx_ll_utils.h"
#include "stm32f4xx_ll_pwr.h"
#include "stm32f4xx_ll_dma.h"
#include "stm32f4xx_ll_tim.h"
#include "stm32f4xx.h"
#include "stm32f4xx_ll_gpio.h"

#include "fc_usart.h"
#include "fc_load_sw.h"
#include "fc_gyroacc.h"
#include "fc_barometer.h"

#define SYSTICK_FREQUENCY       8000U

#define STATE_ERROR             0
#define STATE_UNINITIALIZED     1
#define STATE_RUNNING           2


#define OUT1    GPIOA, LL_GPIO_PIN_12


#ifndef NVIC_PRIORITYGROUP_0
#define NVIC_PRIORITYGROUP_0         ((uint32_t)0x00000007) /*!< 0 bit  for pre-emption priority,
                                                                 4 bits for subpriority */
#define NVIC_PRIORITYGROUP_1         ((uint32_t)0x00000006) /*!< 1 bit  for pre-emption priority,
                                                                 3 bits for subpriority */
#define NVIC_PRIORITYGROUP_2         ((uint32_t)0x00000005) /*!< 2 bits for pre-emption priority,
                                                                 2 bits for subpriority */
#define NVIC_PRIORITYGROUP_3         ((uint32_t)0x00000004) /*!< 3 bits for pre-emption priority,
                                                                 1 bit  for subpriority */
#define NVIC_PRIORITYGROUP_4         ((uint32_t)0x00000003) /*!< 4 bits for pre-emption priority,
                                                                 0 bit  for subpriority */
#endif


extern unsigned long GlobalTime;
extern unsigned long SoftTimer;
extern unsigned long SoftTimer1;
extern unsigned long SoftTimer2;
extern unsigned long SoftTimer3;
extern unsigned long SoftTimer4;


#ifdef __cplusplus
extern "C" {
#endif
 
extern void InitBoard(void);
  
extern void InitInterrupts(void);
  
extern void InitSystemClocks(void);

extern void InitPeripheralClocks(void);
  
extern void InitGpios(void);

void SysTick_Handler(void);
void _Error_Handler(char *, int);

#define Error_Handler() _Error_Handler(__FILE__, __LINE__)

#ifdef __cplusplus
}
#endif

#endif /* __FC_BOARD_H */